/* cvrotgfx.h */

/* Copyright (C) Olly Betts 1997
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

#ifndef SVX_CVROTGFX_H
#define SVX_CVROTGFX_H 1

/* FIXME: sort out _cvrotgfx_textcol/_cvrotgfx_drawcol ugliness */

#include "whichos.h"

extern int mouse_buttons;

extern int colText, colHelp;

#ifdef __DJGPP__
# define ALLEGRO 1
#endif

extern int cvrotgfx_parse_cmdline(int *pargc, char **argv);
extern int cvrotgfx_init(void);
extern int cvrotgfx_pre_main_draw(void);
extern int cvrotgfx_post_main_draw(void);
extern int cvrotgfx_pre_supp_draw(void);
extern int cvrotgfx_post_supp_draw(void);
extern int cvrotgfx_final(void);
extern int cvrotgfx_get_key(void);
extern void cvrotgfx_read_mouse(int *pdx, int *pdy, int *pbut);
extern void (cvrotgfx_beep)(void); /* make a beep */
extern void cvrotgfx_moveto(int X, int Y);
extern void cvrotgfx_lineto(int X, int Y);

#if (OS==UNIX)

/* UNIX + Allegro */
# include "allegro.h"
# define ALLEGRO 1
extern BITMAP *BitMap, *BitMapDraw;

# define CVROTGFX_LBUT 1 /* mask for left mouse button */
# define CVROTGFX_RBUT 2 /* mask for right mouse button (if present) */
# define CVROTGFX_MBUT 4 /* mask for middle mouse button (if present) */

# define cvrotgfx_beep() NOP /* !HACK! */

# ifdef NO_MOUSE_SUPPORT
/* emulate a dead mouse */
#  define cvrotgfx_read_mouse(PDX, PDY, PBUT) NOP
# endif

# ifdef NO_TEXT
#  define outtextxy(X, Y, S) NOP
# else
#  define outtextxy(X, Y, S) BLK(\
 extern int _cvrotgfx_textcol;\
 textout(BitMapDraw, font, (char *)(S), (X), (Y), _cvrotgfx_textcol);)
# endif
# define set_tcolour(X) BLK(extern int _cvrotgfx_textcol; _cvrotgfx_textcol = (X);)
# define set_gcolour(X) BLK(extern int _cvrotgfx_drawcol; _cvrotgfx_drawcol = (X);)
# define text_xy(X, Y, S) outtextxy(12 + (X) * 12, 12 + (Y) * 12, (char *)(S))

/* # define shift_pressed() (key[KEY_LSHIFT] || key[KEY_RSHIFT]) alternative... */
# define shift_pressed() (key_shifts & KB_SHIFT_FLAG)

#elif (OS==MSDOS)

/* DOS-specific I/O functions */
# include <bios.h>
# include <conio.h>
# include <dos.h>

# ifdef MSC
/* Microsoft C */
#  include <graph.h>

# elif defined(JLIB)
/* DJGPP + JLIB */
#  include "jlib.h"
extern buffer_rec *BitMap;

# elif defined(ALLEGRO)
/* DJGPP + Allegro */
#  include "allegro.h"
extern BITMAP *BitMap, *BitMapDraw;

# elif defined(__DJGPP__)
/* DJGPP + GRX */
#  include "grx20.h"
extern GrContext *BitMap;

# else
/* Borland C */
#  include <graphics.h>

# endif

# ifdef NO_MOUSE_SUPPORT
/* emulate a dead mouse */
#  define cvrotgfx_read_mouse(PDX, PDY, PBUT) NOP
# endif

# ifdef JLIB
#  define CVROTGFX_LBUT MOUSE_B_LEFT
#  define CVROTGFX_MBUT MOUSE_B_MIDDLE
#  define CVROTGFX_RBUT MOUSE_B_RIGHT
# elif !defined(ALLEGRO) && defined(__DJGPP__)
#  define CVROTGFX_LBUT GR_M_LEFT
#  define CVROTGFX_MBUT GR_M_MIDDLE
#  define CVROTGFX_RBUT GR_M_RIGHT
# else
#  define CVROTGFX_LBUT 1 /* mask for left mouse button */
#  define CVROTGFX_RBUT 2 /* mask for right mouse button (if present) */
#  define CVROTGFX_MBUT 4 /* mask for middle mouse button (if present) */
# endif

# define cvrotgfx_beep() sound(256) /* 256 is frequency */

/* !HACK! fix this stuff */
# ifdef MSC
#  define shift_pressed() (_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0x03)
# elif defined(ALLEGRO)
#  define shift_pressed() (key_shifts & KB_SHIFT_FLAG)
# else
#  define R_SHIFT  0x01
#  define L_SHIFT  0x02
#  ifndef _KEYBRD_SHIFTSTATUS
#   define _KEYBRD_SHIFTSTATUS 2 /* for DJGPP */
#  endif
/* use function 2 to determine if shift keys are depressed */
#  define shift_pressed() (bioskey(_KEYBRD_SHIFTSTATUS) & (R_SHIFT | L_SHIFT))
# endif

# ifdef MSC
#  define cvrotgfx_moveto _moveto
#  define cvrotgfx_lineto _lineto
# elif !defined(__DJGPP__)
#  define cvrotgfx_moveto moveto
#  define cvrotgfx_lineto lineto
# else
extern void cvrotgfx_moveto(int X, int Y);
extern void cvrotgfx_lineto(int X, int Y);
# endif

# ifdef MSC
#  define outtextxy(X, Y, S) BLK(cvrotgfx_moveto(X, Y); _outgtext(S);)
#  define set_tcolour(X) _setcolor(X)
#  define set_gcolour(X) _setcolor(X)
#  define text_xy(X, Y, S) outtextxy((X) * 12, (Y) * 12, S)
# elif defined(JLIB)
#  define outtextxy(X, Y, S) buff_draw_string(BitMap, (S), (X), (Y), 15); /*15 is colour !HACK!*/
#  define set_tcolour(X) /* !!HACK!! */
#  define set_gcolour(X) BLK(extern int _cvrotgfx_drawcol; _cvrotgfx_drawcol = (X);)
#  define text_xy(X, Y, S) outtextxy(12 + (X) * 12, 12 + (Y) * 12, (S))
#  define far
# elif defined(ALLEGRO)
#  ifdef NO_TEXT
#   define outtextxy(X, Y, S) NOP
#  else
#   define outtextxy(X, Y, S) BLK(\
 extern int _cvrotgfx_textcol;\
 textout(BitMapDraw, font, (char *)(S), (X), (Y), _cvrotgfx_textcol);)
#  endif
#  define set_tcolour(X) BLK(extern int _cvrotgfx_textcol; _cvrotgfx_textcol = (X);)
#  define set_gcolour(X) BLK(extern int _cvrotgfx_drawcol; _cvrotgfx_drawcol = (X);)
#  define text_xy(X, Y, S) outtextxy(12 + (X) * 12, 12 + (Y) * 12, S)
# elif defined(__DJGPP__)
#  define outtextxy(X, Y, S) GrTextXY((X), (Y), (S), 15, 0) /* !HACK! 15 and 0 are colours */
#  define set_tcolour(X) /* !!HACK!! */
#  define set_gcolour(X) /* !!HACK!! */
#  define text_xy(X, Y, S) outtextxy(12 + (X) * 12, 12 + (Y) * 12, S)
# else
#  define set_tcolour(X) setcolor(X)
#  define set_gcolour(X) setcolor(X)
#  define text_xy(X, Y, S) outtextxy((X) * 12, (Y) * 12, S)
# endif
# undef  Y_UP /* PC has increasing Y down screen */

#elif (OS==RISCOS)

# undef bool /* bloody namespace polluting libraries */
# include "oslib/os.h"
# include "oslib/osbyte.h"
# include "oslib/osword.h"
# define bool BOOL

extern void outtextxy(int x, int y, const char *str);
extern void text_xy(int x, int y, const char *str);

# define set_gcolour(X) (ol_setcol((X)))
# define set_tcolour(X) (xos_set_gcol(), xos_writec(0), xos_writec((X)))
# define shift_pressed() (osbyte_read(osbyte_VAR_KEYBOARD_STATE) & 0x08)

# define cvrotgfx_beep() xos_bell()

# define CVROTGFX_LBUT 4
# define CVROTGFX_MBUT 2
# define CVROTGFX_RBUT 1

#else
# error Operating System not known
#endif

#endif
