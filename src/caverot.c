/* > caverot.c
 * Reads in SURVEX .3d image files & allows quick rotation and examination
 * Copyright (C) 1990,1993-2001 Olly Betts
 * Portions Copyright (C) 1993 Wookey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_SIGNAL
# ifdef HAVE_SETJMP
#  include <setjmp.h>
static jmp_buf jmpbufSignal;
#  include <signal.h>
# else
#  undef HAVE_SIGNAL
# endif
#endif

#include "cmdline.h"
#include "filename.h"
#include "message.h"
#include "filelist.h"
#include "useful.h"

#include "caverot.h"
#include "cvrotimg.h"
#include "cvrotgfx.h"
#include "rotplot.h"
#include "labels.h"

/* For funcs which want to be immune from messing around with different
 * calling conventions */
#ifndef CDECL
# define CDECL
#endif

/*#define ANIMATE*/
#define ANIMATE_FPS 1

static unsigned char acolDraw[] = {
   255, 23, 119, 139, 67, 210, 207, 226, 31, 39, 55, 83, 186, 195, 75
};

/***************************************************************************/

/* prototypes */

void setup(void);              /* initialize as required */
static void parse_command(int argc, char **argv);
static void show_help(void);   /* displays help screen */
static void wait_for_key_or_mouse(void);
void set_defaults(void);       /* reset default values */
bool process_key(void);        /* read & process keyboard and mouse input */
void swap_screen(bool);        /* swap displayed screen and drawing screen */

/* translate all points */
static void translate_data(coord Xchange, coord Ychange, coord Zchange);

static double lap_timer(bool want_time);

/***************************************************************************/

/* global variables */

/* lots of these could be in structures - eg a view struct
   then it could be passed to input_values, process_key etc */

/*
Hungarian style types:
 deg  - angle in degrees
 sc   - scaling factor
 f    - flag
 c    - count
 pth  - filing system path
 sz   - pointer to zero-terminated string
*/

static float degView;          /* current direction of view (degrees) */
static float degViewStep;      /* current size of change in view direction */
static float degStereoSep;     /* half the stereo separation of 3d view */
double scDefault;        /* scale to show whole survey on screen */
static double sc;               /* current scale */
static float ZoomFactor;       /* zoom in/out factor */
static bool fRotating;        /* flag indicating auto-rotation */
static bool fNames = fFalse;  /* Draw station names? */
bool fAllNames = fFalse;  /* Draw all station names? */
static bool fLegs = fTrue;   /* Draw legs? */
static bool fSLegs = fFalse;   /* Draw surface legs? */
static bool fStns = fFalse;  /* Draw crosses at stns? */
static bool fAll = fFalse;  /* Draw all during movement? */
static bool fRevSense = fFalse; /* Movements relate to cave/observer */
static bool fRevRot = fFalse; /* auto-rotate backwards */
static float elev;             /* angle of elevation of viewpoint */
static float nStepsize;        /* stepsize for movements */
static bool f3D;              /* flag indicating red/green 3d view mode */
static bool fOverlap; /* are overlapping labels pruned? may be disabled when memory is tight */
int xcMac, ycMac;            /* screen size in plot units (==pixels usually) */
#if 1
#ifdef Y_UP
float y_stretch = 1.0f;   /* factor to multiply y by to correct aspect ratio */
#else /* !defined(Y_UP) */
float y_stretch = -1.0f;  /* factor to multiply y by to correct aspect ratio */
#endif
#else
float y_stretch = -0.3f;  /* test that aspect ratio works */
#endif

#if 0
static int Stored_views;     /* number of defined stores */
static view *pView_store;     /* pointer to array of view stores */
#endif

static int locked; /* no data along this axis (1=>x, 2=>y, 3=>z, 0=>none) */

#ifdef ANIMATE
int cAnimate = 0;
int incAnimate = 1;
#endif

static point Huge **ppLegs = NULL;
static point Huge **ppSLegs = NULL;
static point Huge **ppStns = NULL;

static void
draw_legs(void)
{
#ifndef ANIMATE
   int c;
   for (c = 0; ppLegs[c]; c++) {
      set_gcolour(acolDraw[c]);
      draw_view_legs(ppLegs[c]);
   }
#else
   set_gcolour(acolDraw[0]);
   draw_view_legs(ppLegs[cAnimate]);
#endif
}

static void
draw_slegs(void)
{
   int c;
   for (c = 0; ppSLegs[c]; c++) {
      set_gcolour(acolDraw[c]);
      draw_view_legs(ppSLegs[c]);
   }
}

static void
draw_stns(void)
{
#ifndef ANIMATE
   int c;
   for (c = 0; ppStns[c]; c++) {
      set_gcolour(acolDraw[c]);
      draw_view_stns(ppStns[c]);
   }
#else
   set_gcolour(acolDraw[c]);
   draw_view_stns(ppStns[cAnimate]);
#endif
}

static void
draw_names(void)
{
#ifndef ANIMATE
   int c;
   set_tcolour(colText);
   for (c = 0; ppStns[c]; c++) {
      draw_view_labs(ppStns[c]);
   }
#else
   set_tcolour(colText);
   draw_view_labs(ppStns[cAnimate]);
#endif
}

/*************************************************************************/
/*************************************************************************/

#ifdef HAVE_SIGNAL

/* for systems not using autoconf, assume the signal handler returns void
 * unless specified elsewhere */
#ifndef RETSIGTYPE
# define RETSIGTYPE void
#endif

static CDECL RETSIGTYPE FAR
handle_sig(int sig)
{
   sig = sig;
   longjmp(jmpbufSignal, 1);
}

#endif

int
main(int argc, char **argv)
{
   enum { LEGS, STNS, LABS, DONE } item = LEGS;
#if (OS==WIN32)
   /* workaround for bug in win32 allegro - should be fixed in WIP 3.9.34 */
   argv[argc] = NULL;
#endif

   set_codes(MOVE, DRAW, STOP);

   msg_init(argv[0]);

   puts("Survex cave rotator v"VERSION"\n  "COPYRIGHT_MSG);

   parse_command(argc, argv);

   /* these aren't in set_defaults() 'cos we don't want DELETE to reset them */
   degView = 0;
   elev = 90.0f;       /* display plan by default */
   fRotating = fFalse; /* stationary to start with */
   f3D = fFalse;       /* red/green 3d view off by default */
   degStereoSep = 2.5f;

   /* setup colour values for 3dview (RED_3D,GREEN_3D,BLUE_3D) or ??? */

#if 0
   /* setup view store */
   Stored_views = 9;
   pView_store = (viewstore*) osmalloc(Stored_views * sizeof(view));
#endif

#ifdef HAVE_SIGNAL
   /* set up signal handler */
   if (setjmp(jmpbufSignal)) {
      signal(SIGINT, SIG_DFL);
      cvrotgfx_final();
      exit(EXIT_FAILURE);
   }
#endif

   /* initialise graphics h/ware & s/ware */
   cvrotgfx_init();

#ifdef HAVE_SIGNAL
   /* set up signal handler to close down graphics on SIGINT */
   signal(SIGINT, handle_sig);
#endif
   
   fOverlap = init_map(xcMac, ycMac);

   /* display help screen to give user something to read while we do scaling */
   show_help();

   {
      int c;
      point lower, upper;

      reset_limits(&lower, &upper);

      for (c = 0; ppLegs[c]; c++) {
	 update_limits(&lower, &upper, ppLegs[c], ppStns[c]);
	 update_limits(&lower, &upper, ppSLegs[c], NULL);
      }

      /* can't do this until after we've initialised the graphics */
      scDefault = scale_to_screen(&lower, &upper, xcMac, ycMac, y_stretch);
   }

   /* Check if we've got a flat plot aligned perpendicular to an axis */
   locked = 0;
   if (Xrad == 0) locked |= 1;
   if (Yrad == 0) locked |= 2;
   if (Zrad == 0) locked |= 4;
   switch (locked) {
    case 1:
      degView = 90;
      elev = 0.0f; /* elevation looking along X axis */
      break;
    case 2:
      degView = 0;
      elev = 0.0f; /* elevation looking along Y axis */
      break;
    case 4:
      locked = 3;
      degView = 0;
      elev = 90.0f; /* plan */
      break;
    default:
      /* don't bother locking it if it's linear or a single point */
      locked = 0;
      break;
   }

   /* set base step size according to screen size */
   nStepsize = min(xcMac, ycMac) / 25.0f;
   set_defaults();

   /* still displaying help page so wait here */
   wait_for_key_or_mouse();

     {
	const char *szPlan;
	const char *szElev;

	fRedraw = fTrue;

	szPlan = msgPerm(/*Plan, %05.1f up screen*/101);
	szElev = msgPerm(/*View towards %05.1f*/102);

	lap_timer(fFalse);
	
	while (fTrue) {
	   char sz[80];

	   if (fRedraw) {
	      fRedraw = fFalse;
	      while (degView >= 360) degView -= 360;
	      while (degView < 0) degView += 360;
	      clear_map();
	      cvrotgfx_pre_main_draw();
	      if (f3D) {
#if 0
		 int c;
		 set_gcolour(col3dL);
		 set_view(sc, degView - degStereoSep, elev);
		 for (c = 0; ppLegs[c]; c++) draw_view_legs(ppLegs[c]);
		 set_gcolour(col3dR);
		 set_view(sc, degView + degStereoSep, elev);
		 for (c = 0; ppLegs[c]; c++) draw_view_legs(ppLegs[c]);
#endif
	      } else {
		 set_view(sc, degView, elev);
		 if (fAll) {
		    if (fLegs) draw_legs();
		    if (fSLegs) draw_slegs();
		    if (fStns) draw_stns();
		    if (fNames) draw_names();
		    item = DONE;
		 } else if (fLegs || fSLegs) {
		    if (fLegs) draw_legs();
		    if (fSLegs) draw_slegs();
		    item = STNS;
		 } else if (fStns) {
		    draw_stns();
		    item = LABS;
		 } else {
		    if (fNames) draw_names();
		    item = DONE;
		 }
	      }
	      set_tcolour(colText);
	      sprintf(sz, (elev == 90.0f) ? szPlan : szElev, degView);
	      text_xy(0, 0, sz);
	      draw_scale_bar();
	      cvrotgfx_post_main_draw();
	   } else {
	      switch (item) {
	       case STNS:
		 if (fStns) {
		    cvrotgfx_pre_supp_draw();
		    draw_stns();
		    cvrotgfx_post_supp_draw();
		 }
		 item++;
		 break;
	       case LABS:
		 if (fNames) {
		    cvrotgfx_pre_supp_draw();
		    draw_names();
		    cvrotgfx_post_supp_draw();
		 }
		 item++;
		 break;
	       default:
		 break; /* suppress warning */
	      }
	   }

	   /* if (fDemo)
	    * if (demo_step()) fRedraw = fTrue;
	    * else */
	   if (process_key()) fRedraw = fTrue;
	}
     }
}

/* magic macro needed so Allegro can automagically parse the command-line
 * into argc and argv on MS Windows */
#ifdef ALLEGRO
END_OF_MAIN()
#endif

/***************************************************************************/

/* things in here get reset by DELETE key */
void
set_defaults(void)
{
   degViewStep = 3;
   sc = scDefault;
   translate_data(-Xorg, -Yorg, -Zorg); /* rotate about 'centre' of cave */
}

/***************************************************************************/

/*
bool
demo_step(void)
{
   static n = 0;
   struct {
   };
} */

static double
lap_timer(bool want_time)
{
   double lap_time = 0.0;
   static clock_t new_time = 0;
   clock_t old_time = new_time;
   /* no point using anything better than clock() on DOS at least */
   new_time = clock();
   if (want_time) {
      int dt = (int)(new_time - old_time);
      /* better to be paranoid */
      if (dt < 1) dt = 1;
      lap_time = (double)dt / CLOCKS_PER_SEC;
      /* put a ceiling on the time so motion is at least controllable.
       * 1.0 sec ceiling means cave will rotate in <=30 degree steps
       * at default speed.
       */
      if (lap_time > 1.0) lap_time = 1.0;
   }
   return lap_time;
}

bool
process_key(void) /* and mouse! */
{
   float nStep, Accel;
   int iKeycode;
   static bool fChanged = fTrue; /* Want to time initial draw */
   double s = SIND(degView), c = COSD(degView);
   static int autotilt = 0;
   static float autotilttarget;
   static double tsc = 1.0; /* sane starting value */
#ifdef ANIMATE
   static clock_t last_animate = 0;
#endif

   if (fChanged) {
      fChanged = fFalse;
      tsc = lap_timer(fTrue) * 10.0;
   } else {
      lap_timer(fFalse);
   }

#ifdef ANIMATE
   if (new_time - last_animate > CLOCKS_PER_SEC / ANIMATE_FPS) {
      cAnimate += incAnimate;
      if (cAnimate == 0 || ppLegs[cAnimate + 1] == NULL)
         incAnimate = -incAnimate;

      if (last_animate == 0)
         last_animate = new_time;
      else
         last_animate += CLOCKS_PER_SEC / ANIMATE_FPS;

      fChanged = fTrue;
   }
#endif

   iKeycode = cvrotgfx_get_key();
   if (shift_pressed()) {
      nStep = 5.0f * nStepsize / sc;
      ZoomFactor = BIG_MAGNIFY_FACTOR;
      Accel = 5.0f;
   } else {
      nStep = nStepsize / sc;
      ZoomFactor = LITTLE_MAGNIFY_FACTOR;
      Accel = 1.0f;
   }
   nStep *= tsc;
   ZoomFactor = (float)pow(ZoomFactor, tsc);
   Accel *= tsc;

   if (fRevSense) nStep = -nStep;

   if (iKeycode > -1) {
      autotilt = 0;
      if (iKeycode >= 'a' && iKeycode <= 'z')
	 iKeycode -= 32; /* force lower case letters to upper case */
      if (locked == 0 || locked == 3) {
	 /* no restriction, or plan only */
	 switch (iKeycode) {
/*        case '3': case SHIFT_3:    f3D = !f3D; break; */
	  case 'Z':
	    degViewStep *= 1.2f;
	    if (degViewStep >= 45) degViewStep = 45;
	    break;
	  case 'X':
	    degViewStep /= 1.2f;
	    if (degViewStep <= 0.1) degViewStep = 0.1;
	    break;
	  case 'C': degView += degViewStep * Accel; fChanged = fTrue; break;
	  case 'V': degView -= degViewStep * Accel; fChanged = fTrue; break;
	  case 'R': fRevRot = !fRevRot; break;
	  case ' ':            fRotating = fFalse; break;
	  case RETURN_KEY:     fRotating = fTrue; break;
	 }
      }
      if (locked == 0) {
	 switch (iKeycode) {
	  case '\'': case '@': case '"':
	    /* shift-' varies with keyboard layout
	     * so check both '@' (UK) and '"' (US) */
	    if (fRevSense) goto tiltup;
	    tiltdown:
	    if (elev > -90.0f) {
	       elev -= degViewStep * Accel;
	       if (elev < -90.0f) elev = -90.0f;
	       fChanged = fTrue;
	    }
	    break;
	  case '/': case '?':
	    if (fRevSense) goto tiltdown;
	    tiltup:
	    if (elev < 90.0f) {
	       elev += degViewStep * Accel;
	       if (elev > 90.0f) elev = 90.0f;
	       fChanged = fTrue;
	    }
	    break;
	  case 'P':
	    if (elev != 90.0f) {
/*fprintf(stderr, "P:tsc = %f\n", tsc);fflush(stderr);*/
	       autotilt = (int)ceil(90.0 / ceil(9.0 / tsc));
/*fprintf(stderr,"P:autotilt = %d\n",autotilt);fflush(stderr);*/
	       if (autotilt > 16) {
		  autotilt = 0;
		  elev = 90.0f;
	       }
	       autotilttarget = 90.0f;
	       fChanged = fTrue;
	    }
	    break;
	  case 'L':
	    if (elev!=0.0f) {
/*fprintf(stderr,"L:tsc = %f\n",tsc);fflush(stderr);*/
	       autotilt = (int)ceil(90.0 / ceil(9.0 / tsc));
/*fprintf(stderr,"L:autotilt = %d\n",autotilt);fflush(stderr);*/
	       if (autotilt > 16) {
		  elev = 0.0f;
		  autotilt = 0;
	       } else if (elev > 0.0f) {
		  autotilt = -autotilt;
	       }
	       autotilttarget = 0.0f;
	       fChanged = fTrue;
	    }
	    break;
	 }
      }
      switch (iKeycode) {
       case ']': case '}':
         sc = fRevSense ? sc / ZoomFactor : sc * ZoomFactor;
         fChanged = fTrue; break;
       case '[': case '{':
         sc = fRevSense ? sc * ZoomFactor : sc / ZoomFactor;
         fChanged = fTrue; break;
       case 'U': elev = 90; fChanged = fTrue; break;
       case 'D': elev = -90; fChanged = fTrue; break;
       case 'N': degView = 0; fChanged = fTrue; break;
       case 'S': degView = 180; fChanged = fTrue; break;
       case 'E': degView = 90; fChanged = fTrue; break;
       case 'W': degView = 270; fChanged = fTrue; break;
       case DELETE_KEY:  set_defaults(); fChanged = fTrue; break;
       case 'O':   fAllNames = !fAllNames; fChanged = fNames; break;
/*       case 'I':   get_view_details(); fChanged=fTrue; break; */
       case CURSOR_UP:
         if (elev == 90.0f)
            translate_data((coord)(nStep * s), (coord)(nStep * c), 0);
         else
            translate_data(0, 0, (coord)nStep);
         fChanged = fTrue; break;
       case CURSOR_DOWN:
         if (elev == 90.0f)
            translate_data(-(coord)(nStep * s), -(coord)(nStep * c), 0);
         else
            translate_data(0, 0, (coord)-nStep);
         fChanged = fTrue; break;
       case CURSOR_LEFT:
         translate_data(-(coord)(nStep * c), (coord)(nStep * s), 0);
         fChanged = fTrue; break;
       case CURSOR_RIGHT:
         translate_data((coord)(nStep * c), -(coord)(nStep * s), 0);
         fChanged = fTrue; break;
       case 'H': {
	  show_help();
	  wait_for_key_or_mouse();	  
	  fChanged = fTrue;
	  lap_timer(fFalse);
	  break;
       }
       case ('A' - '@'):
         fAll = !fAll; break; /* no need to redraw */
       case ('N' - '@'):
         fChanged = fTrue; fNames = !fNames; break;
       case ('X' - '@'):
         fChanged = fTrue; fStns = !fStns; break;
       case ('L' - '@'):
         fChanged = fTrue; fLegs = !fLegs; break;
       case ('S' - '@'): /* FIXME: add to help screen */
       case ('F' - '@'): /* surFace - ^S is iffy on DOS iirc - FIXME: check */
         fChanged = fTrue; fSLegs = !fSLegs; break;
       case ('R' - '@'):
         fRevSense = !fRevSense; break;
       case ESCAPE_KEY:
	 cvrotgfx_final();
         exit(EXIT_SUCCESS); break; /* That's all folks */
      }
   }

   if (mouse_buttons >= 0) {
      int buttons = 0; /* zero in case read_mouse() doesn't set */
      int dx = 0, dy = 0;
      cvrotgfx_read_mouse(&dx, &dy, &buttons);
      if (buttons & (CVROTGFX_LBUT | CVROTGFX_RBUT | CVROTGFX_MBUT))
	 autotilt = 0;
      if (buttons & CVROTGFX_LBUT) {
         if (fRevSense) {
            sc *= (float)pow(LITTLE_MAGNIFY_FACTOR, 0.08 * dy);
            if (locked == 0 || locked == 3) degView += dx * 0.16f;
         } else {
            sc *= (float)pow(LITTLE_MAGNIFY_FACTOR, -0.08 * dy);
            if (locked == 0 || locked == 3) degView -= dx * 0.16f;
         }
         fChanged = fTrue;
      }
      if (buttons & CVROTGFX_RBUT) {
         nStep *= 0.025f;
         if (elev == 90.0f)
            translate_data((coord)(nStep * (dx * c + dy * s)),
                           -(coord)(nStep * (dx * s - dy * c)), 0);
         else
            translate_data((coord)(nStep * (double)dx * c),
                           -(coord)(nStep * (double)dx * s),
			   (coord)(nStep * dy));
         fChanged = fTrue;
      }
      if (locked == 0 && (buttons & CVROTGFX_MBUT)) {
         if (fRevSense) {
	    elev += dy * 0.16f;
	 } else {
	    elev -= dy * 0.16f;
	 }
	 if (elev > 90.0f) elev = 90.0f;
	 if (elev < -90.0f) elev = -90.0f;
         fChanged = fTrue;
      }
   }

   if (autotilt) {
      elev += (float)autotilt;
      fChanged = fTrue;
      if (autotilt > 0) {
         if (elev >= autotilttarget) {
            elev = autotilttarget;
            autotilt = 0;
         }
      } else {
         if (elev <= autotilttarget) {
            elev = autotilttarget;
            autotilt = 0;
         }
      }
   }

   /* rotate by current stepsize */
   if (fRotating) {
      if (fRevRot) {
	 degView -= degViewStep * tsc;
      } else {
	 degView += degViewStep * tsc;
      }
      fChanged = fTrue; /* and force a redraw */
   }

   return fChanged;
}

/***************************************************************************/

#define FLAG_ALWAYS 1
#define FLAG_MOUSE 2
#define FLAG_MOUSE2 4|FLAG_MOUSE
#define FLAG_MOUSE3 8|FLAG_MOUSE
#define FLAG_NOMOUSE 16
#define FLAG_OVERLAP 32

typedef struct {
   int mesg_no;
   const char *mesg;
   int flags;
} flagged_msg;

static void
show_help(void)
{
   /* help text stored as static array of strings, and printed as graphics
    *  to the screen */
   /* TRANSLATE */
   static flagged_msg help_msgs[] = {
	{0, "                  Z,X : Faster/Slower rotation", FLAG_ALWAYS},
	{0, "                    R : [R]everse direction of rotation", FLAG_ALWAYS},
	{0, "          Enter,Space : Start/Stop auto-rotation", FLAG_ALWAYS},
	{0, "                  C,V : Rotate cave one step (anti)clockwise", FLAG_ALWAYS},
	{0, "                  ',/ : Higher/Lower viewpoint", FLAG_ALWAYS},
	{0, "                  ],[ : Zoom In/Out", FLAG_ALWAYS},
	{0, "                  U,D : View [U]p/[D]own", FLAG_ALWAYS},
	{0, "              N,S,E,W : View [N]orth/[S]outh/[E]ast/[W]est", FLAG_ALWAYS},
	{0, "               Delete : Default scale, rotation rate, etc", FLAG_ALWAYS},
	{0, "                  P,L : [P]lan/e[L]evation", FLAG_ALWAYS},
      /*  "                    I : [I]nput values", */
      /*  "                  3,# : Toggle [3]D goggles view", */
	{0, "    Cursor Left,Right : Cave [Left]/[Right] (on screen)", FLAG_ALWAYS},
	{0, "       Cursor Up,Down : Cave [Up]/[Down] (on screen)", FLAG_ALWAYS},
      /*  "    Sft Copy + Letter : Store view in store <Letter>", */
      /*  "        Copy + Letter : Recall view in store <Letter>", */
	{0, "               Ctrl-N : Toggle display of station [N]ames", FLAG_ALWAYS},
	{0, "               Ctrl-X : Toggle display of crosses ([X]s) at stations", FLAG_ALWAYS},
	{0, "               Ctrl-L : Toggle display of survey [L]egs", FLAG_ALWAYS},
	{0, "               Ctrl-A : Toggle display of [A]ll/skeleton when moving", FLAG_ALWAYS},
	{0, "                    O : Toggle display of non-overlapping/all names", FLAG_OVERLAP},
	{0, "               Ctrl-R : Reverse sense of controls", FLAG_ALWAYS},
	{0, "               ESCAPE : Quit program", FLAG_ALWAYS},
	{0, "             Shift accelerates all movement keys", FLAG_ALWAYS},
	{0, NULL, FLAG_ALWAYS},
	{0, "                    H : [H]elp screen (this!)", FLAG_ALWAYS},
	{0, NULL, FLAG_ALWAYS},
	{0, "Mouse controls:  LEFT : left/right rotates, up/down zooms", FLAG_MOUSE},
	{0, "               MIDDLE : up/down tilts", FLAG_MOUSE3},
	{0, "                RIGHT : mouse moves cave", FLAG_MOUSE2},
	{/*"No mouse detected"*/100, NULL, FLAG_NOMOUSE},
	{0, NULL, FLAG_ALWAYS},
	{0, "              PRESS  ANY  KEY  TO  CONTINUE", FLAG_ALWAYS},
	{0, NULL, 0}
   };
   int i, flags, y;

   flags = FLAG_ALWAYS;
   if (mouse_buttons < 0) flags |= FLAG_NOMOUSE;
   if (mouse_buttons >= 1) flags |= FLAG_MOUSE;
   if (mouse_buttons >= 2) flags |= FLAG_MOUSE2;
   if (mouse_buttons >= 3) flags |= FLAG_MOUSE3;
   if (fOverlap) flags |= FLAG_OVERLAP;

   /* display help text over currently displayed image */
   cvrotgfx_pre_supp_draw();
   set_tcolour(colHelp);

   y = 0;
   for (i = 0; help_msgs[i].flags; i++) {
      if (help_msgs[i].flags & flags) {
	 const char *p;
	 if (help_msgs[i].mesg_no)
	    p = msg(help_msgs[i].mesg_no);
	 else
	    p = help_msgs[i].mesg;
	 if (p) text_xy(4, 2 + y, p);
	 y++;
      }
   }

   cvrotgfx_post_supp_draw();

   /* clear keyboard buffer */
   (void)cvrotgfx_get_key();
}

static void
wait_for_key_or_mouse(void)
{
   int dummy, buttons = 0;

   /* wait until key pressed or mouse clicked */
   do {
      if (mouse_buttons > 0) cvrotgfx_read_mouse(&dummy, &dummy, &buttons);
   } while (cvrotgfx_get_key() < 0 && !buttons);
}

/***************************************************************************/

static void
translate_data(coord Xchange, coord Ychange, coord Zchange)
{
   int c;

   Xorg += Xchange;
   Yorg += Ychange;
   Zorg += Zchange;

   for (c = 0; ppLegs[c]; c++)
      do_translate(ppLegs[c], Xchange, Ychange, Zchange);
   for (c = 0; ppSLegs[c]; c++)
      do_translate(ppSLegs[c], Xchange, Ychange, Zchange);
   for (c = 0; ppStns[c]; c++)
      do_translate_stns(ppStns[c], Xchange, Ychange, Zchange);
}

/**************************************************************************/

static const struct option long_opts[] = {
   /* const char *name; int has_arg (0 no_argument, 1 required_*, 2 optional_*); int *flag; int val; */
   CVROTGFX_LONGOPTS
   {"help", no_argument, 0, HLP_HELP},
   {"version", no_argument, 0, HLP_VERSION},
   {0, 0, 0, 0}
};

#define short_opts "c:"CVROTGFX_SHORTOPTS

/* TRANSLATE: FIXME extract help messages to message file */
static struct help_msg help[] = {
/*				<-- */
   CVROTGFX_HELP(0)
   {'c',                        "set display colours"},
   {0, 0}
};

static void
parse_command(int argc, char **argv)
{
   int c;
   size_t col_idx = 0;

   cmdline_set_syntax_message("3D_FILE...", NULL); /* TRANSLATE */
   cmdline_init(argc, argv, short_opts, long_opts, NULL, help, 1, -1);
   while (1) {
      int opt = cmdline_getopt();
      if (opt == EOF) break;
      switch (opt) {
       case 'c': {
	  char *p = optarg;
	  while (*p && col_idx < (sizeof(acolDraw) / sizeof(acolDraw[0]))) {
	     acolDraw[col_idx++] = (unsigned char)strtol(p, &p, 0);
	     if (*p != ',') break;
	     p++;
	  }
	  break;
       }
      }
   }

   putnl();
   puts(msg(/*Reading in data - please wait...*/105));

   argc -= optind;
   argv += optind;

   /* process the tail end of the command line which lists file(s) to load */
   ppLegs = osmalloc((argc + 1) * sizeof(point Huge *));
   ppSLegs = osmalloc((argc + 1) * sizeof(point Huge *));
   ppStns = osmalloc((argc + 1) * sizeof(point Huge *));

   /* load data into memory */
   for (c = 0; c < argc; c++) {
      if (!load_data(argv[c], ppLegs + c, ppSLegs + c, ppStns + c)) {
	 printf(msg(/*Bad 3d image file `%s'*/106), argv[c]);
	 putnl();
	 exit(1);
      }
   }

   ppLegs[argc] = ppSLegs[argc] = ppStns[argc] = NULL;
}

