//
//  mainfrm.cc
//
//  Main frame handling for Aven.
//
//  Copyright (C) 2000-2002 Mark R. Shinwell
//  Copyright (C) 2001-2002 Olly Betts
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "mainfrm.h"
#include "aven.h"
#include "aboutdlg.h"

#include "message.h"
#include "img.h"
#include "namecmp.h"

#include <wx/confbase.h>
#include <float.h>
#include <stack>

#ifdef HAVE_REGEX_H
# include <regex.h>
#else
extern "C" {
# ifdef HAVE_RXPOSIX_H
#  include <rxposix.h>
# elif defined(HAVE_RX_RXPOSIX_H)
#  include <rx/rxposix.h>
# endif
}
#endif

const int NUM_DEPTH_COLOURS = 13; // up to 13

#define TOOLBAR_BITMAP(file) wxBitmap(wxString(msg_cfgpth()) + \
    wxCONFIG_PATH_SEPARATOR + wxString("icons") + wxCONFIG_PATH_SEPARATOR + \
    wxString(file), wxBITMAP_TYPE_PNG)

#include "aven.pal"

class AvenSplitterWindow : public wxSplitterWindow {
    MainFrm *parent;

    public:
	AvenSplitterWindow(MainFrm *parent_)
	    : parent(parent_),
	      wxSplitterWindow(parent_, -1, wxDefaultPosition, wxDefaultSize,
			       wxSP_3D | wxSP_LIVE_UPDATE) {
	}

	void OnSplitterDClick(wxSplitterEvent &e) {
	    parent->ToggleSidePanel();
	}
	
    private:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(AvenSplitterWindow, wxSplitterWindow)
    // The wx docs say "EVT_SPLITTER_DOUBLECLICKED" but the wx headers say
    // "EVT_SPLITTER_DCLICK" (wx docs corrected to agree with headers in 2.3)
#ifdef EVT_SPLITTER_DOUBLECLICKED
    EVT_SPLITTER_DOUBLECLICKED(-1, AvenSplitterWindow::OnSplitterDClick)
#else
    EVT_SPLITTER_DCLICK(-1, AvenSplitterWindow::OnSplitterDClick)
#endif
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MainFrm, wxFrame)
    EVT_BUTTON(button_FIND, MainFrm::OnFind)
    EVT_BUTTON(button_HIDE, MainFrm::OnHide)

    EVT_MENU(menu_FILE_OPEN, MainFrm::OnOpen)
#ifdef AVENPRES
    EVT_MENU(menu_FILE_OPEN_PRES, MainFrm::OnOpenPres)
#endif
    EVT_MENU(menu_FILE_QUIT, MainFrm::OnQuit)
    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MainFrm::OnMRUFile)

#ifdef AVENPRES
    EVT_MENU(menu_PRES_CREATE, MainFrm::OnPresCreate)
    EVT_MENU(menu_PRES_GO, MainFrm::OnPresGo)
    EVT_MENU(menu_PRES_GO_BACK, MainFrm::OnPresGoBack)
    EVT_MENU(menu_PRES_RESTART, MainFrm::OnPresRestart)
    EVT_MENU(menu_PRES_RECORD, MainFrm::OnPresRecord)
    EVT_MENU(menu_PRES_FINISH, MainFrm::OnPresFinish)
    EVT_MENU(menu_PRES_ERASE, MainFrm::OnPresErase)
    EVT_MENU(menu_PRES_ERASE_ALL, MainFrm::OnPresEraseAll)

    EVT_UPDATE_UI(menu_FILE_OPEN_PRES, MainFrm::OnOpenPresUpdate)

    EVT_UPDATE_UI(menu_PRES_CREATE, MainFrm::OnPresCreateUpdate)
    EVT_UPDATE_UI(menu_PRES_GO, MainFrm::OnPresGoUpdate)
    EVT_UPDATE_UI(menu_PRES_GO_BACK, MainFrm::OnPresGoBackUpdate)
    EVT_UPDATE_UI(menu_PRES_RESTART, MainFrm::OnPresRestartUpdate)
    EVT_UPDATE_UI(menu_PRES_RECORD, MainFrm::OnPresRecordUpdate)
    EVT_UPDATE_UI(menu_PRES_FINISH, MainFrm::OnPresFinishUpdate)
    EVT_UPDATE_UI(menu_PRES_ERASE, MainFrm::OnPresEraseUpdate)
    EVT_UPDATE_UI(menu_PRES_ERASE_ALL, MainFrm::OnPresEraseAllUpdate)
#endif

    EVT_CLOSE(MainFrm::OnClose)
    EVT_SET_FOCUS(MainFrm::OnSetFocus)

    EVT_MENU(menu_ROTATION_START, MainFrm::OnStartRotation)
    EVT_MENU(menu_ROTATION_TOGGLE, MainFrm::OnToggleRotation)
    EVT_MENU(menu_ROTATION_STOP, MainFrm::OnStopRotation)
    EVT_MENU(menu_ROTATION_SPEED_UP, MainFrm::OnSpeedUp)
    EVT_MENU(menu_ROTATION_SLOW_DOWN, MainFrm::OnSlowDown)
    EVT_MENU(menu_ROTATION_REVERSE, MainFrm::OnReverseDirectionOfRotation)
    EVT_MENU(menu_ROTATION_STEP_CCW, MainFrm::OnStepOnceAnticlockwise)
    EVT_MENU(menu_ROTATION_STEP_CW, MainFrm::OnStepOnceClockwise)
    EVT_MENU(menu_ORIENT_MOVE_NORTH, MainFrm::OnMoveNorth)
    EVT_MENU(menu_ORIENT_MOVE_EAST, MainFrm::OnMoveEast)
    EVT_MENU(menu_ORIENT_MOVE_SOUTH, MainFrm::OnMoveSouth)
    EVT_MENU(menu_ORIENT_MOVE_WEST, MainFrm::OnMoveWest)
    EVT_MENU(menu_ORIENT_SHIFT_LEFT, MainFrm::OnShiftDisplayLeft)
    EVT_MENU(menu_ORIENT_SHIFT_RIGHT, MainFrm::OnShiftDisplayRight)
    EVT_MENU(menu_ORIENT_SHIFT_UP, MainFrm::OnShiftDisplayUp)
    EVT_MENU(menu_ORIENT_SHIFT_DOWN, MainFrm::OnShiftDisplayDown)
    EVT_MENU(menu_ORIENT_PLAN, MainFrm::OnPlan)
    EVT_MENU(menu_ORIENT_ELEVATION, MainFrm::OnElevation)
    EVT_MENU(menu_ORIENT_HIGHER_VP, MainFrm::OnHigherViewpoint)
    EVT_MENU(menu_ORIENT_LOWER_VP, MainFrm::OnLowerViewpoint)
    EVT_MENU(menu_ORIENT_ZOOM_IN, MainFrm::OnZoomIn)
    EVT_MENU(menu_ORIENT_ZOOM_OUT, MainFrm::OnZoomOut)
    EVT_MENU(menu_ORIENT_DEFAULTS, MainFrm::OnDefaults)
    EVT_MENU(menu_VIEW_SHOW_LEGS, MainFrm::OnShowSurveyLegs)
    EVT_MENU(menu_VIEW_SHOW_CROSSES, MainFrm::OnShowCrosses)
    EVT_MENU(menu_VIEW_SHOW_ENTRANCES, MainFrm::OnShowEntrances)
    EVT_MENU(menu_VIEW_SHOW_FIXED_PTS, MainFrm::OnShowFixedPts)
    EVT_MENU(menu_VIEW_SHOW_EXPORTED_PTS, MainFrm::OnShowExportedPts)
    EVT_MENU(menu_VIEW_SHOW_NAMES, MainFrm::OnShowStationNames)
    EVT_MENU(menu_VIEW_SHOW_OVERLAPPING_NAMES, MainFrm::OnDisplayOverlappingNames)
    EVT_MENU(menu_VIEW_SHOW_SURFACE, MainFrm::OnShowSurface)
    EVT_MENU(menu_VIEW_SURFACE_DEPTH, MainFrm::OnShowSurfaceDepth)
    EVT_MENU(menu_VIEW_SURFACE_DASHED, MainFrm::OnShowSurfaceDashed)
    EVT_MENU(menu_VIEW_COMPASS, MainFrm::OnViewCompass)
    EVT_MENU(menu_VIEW_CLINO, MainFrm::OnViewClino)
    EVT_MENU(menu_VIEW_GRID, MainFrm::OnViewGrid)
    EVT_MENU(menu_VIEW_DEPTH_BAR, MainFrm::OnToggleDepthbar)
    EVT_MENU(menu_VIEW_SCALE_BAR, MainFrm::OnToggleScalebar)
    EVT_MENU(menu_VIEW_SIDE_PANEL, MainFrm::OnViewSidePanel)
    EVT_MENU(menu_VIEW_METRIC, MainFrm::OnToggleMetric)
    EVT_MENU(menu_VIEW_DEGREES, MainFrm::OnToggleDegrees)
#ifdef AVENGL
    EVT_MENU(menu_VIEW_SHOW_TUBES, MainFrm::OnToggleTubes)
#endif
    EVT_MENU(menu_CTL_REVERSE, MainFrm::OnReverseControls)
    EVT_MENU(menu_CTL_CANCEL_DIST_LINE, MainFrm::OnCancelDistLine)
    EVT_MENU(menu_HELP_ABOUT, MainFrm::OnAbout)

    EVT_UPDATE_UI(menu_ROTATION_START, MainFrm::OnStartRotationUpdate)
    EVT_UPDATE_UI(menu_ROTATION_TOGGLE, MainFrm::OnToggleRotationUpdate)
    EVT_UPDATE_UI(menu_ROTATION_STOP, MainFrm::OnStopRotationUpdate)
    EVT_UPDATE_UI(menu_ROTATION_SPEED_UP, MainFrm::OnSpeedUpUpdate)
    EVT_UPDATE_UI(menu_ROTATION_SLOW_DOWN, MainFrm::OnSlowDownUpdate)
    EVT_UPDATE_UI(menu_ROTATION_REVERSE, MainFrm::OnReverseDirectionOfRotationUpdate)
    EVT_UPDATE_UI(menu_ROTATION_STEP_CCW, MainFrm::OnStepOnceAnticlockwiseUpdate)
    EVT_UPDATE_UI(menu_ROTATION_STEP_CW, MainFrm::OnStepOnceClockwiseUpdate)
    EVT_UPDATE_UI(menu_ORIENT_MOVE_NORTH, MainFrm::OnMoveNorthUpdate)
    EVT_UPDATE_UI(menu_ORIENT_MOVE_EAST, MainFrm::OnMoveEastUpdate)
    EVT_UPDATE_UI(menu_ORIENT_MOVE_SOUTH, MainFrm::OnMoveSouthUpdate)
    EVT_UPDATE_UI(menu_ORIENT_MOVE_WEST, MainFrm::OnMoveWestUpdate)
    EVT_UPDATE_UI(menu_ORIENT_SHIFT_LEFT, MainFrm::OnShiftDisplayLeftUpdate)
    EVT_UPDATE_UI(menu_ORIENT_SHIFT_RIGHT, MainFrm::OnShiftDisplayRightUpdate)
    EVT_UPDATE_UI(menu_ORIENT_SHIFT_UP, MainFrm::OnShiftDisplayUpUpdate)
    EVT_UPDATE_UI(menu_ORIENT_SHIFT_DOWN, MainFrm::OnShiftDisplayDownUpdate)
    EVT_UPDATE_UI(menu_ORIENT_PLAN, MainFrm::OnPlanUpdate)
    EVT_UPDATE_UI(menu_ORIENT_ELEVATION, MainFrm::OnElevationUpdate)
    EVT_UPDATE_UI(menu_ORIENT_HIGHER_VP, MainFrm::OnHigherViewpointUpdate)
    EVT_UPDATE_UI(menu_ORIENT_LOWER_VP, MainFrm::OnLowerViewpointUpdate)
    EVT_UPDATE_UI(menu_ORIENT_ZOOM_IN, MainFrm::OnZoomInUpdate)
    EVT_UPDATE_UI(menu_ORIENT_ZOOM_OUT, MainFrm::OnZoomOutUpdate)
    EVT_UPDATE_UI(menu_ORIENT_DEFAULTS, MainFrm::OnDefaultsUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_LEGS, MainFrm::OnShowSurveyLegsUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_CROSSES, MainFrm::OnShowCrossesUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_ENTRANCES, MainFrm::OnShowEntrancesUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_FIXED_PTS, MainFrm::OnShowFixedPtsUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_EXPORTED_PTS, MainFrm::OnShowExportedPtsUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_NAMES, MainFrm::OnShowStationNamesUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_SURFACE, MainFrm::OnShowSurfaceUpdate)
    EVT_UPDATE_UI(menu_VIEW_SURFACE_DEPTH, MainFrm::OnShowSurfaceDepthUpdate)
    EVT_UPDATE_UI(menu_VIEW_SURFACE_DASHED, MainFrm::OnShowSurfaceDashedUpdate)
    EVT_UPDATE_UI(menu_VIEW_SHOW_OVERLAPPING_NAMES, MainFrm::OnDisplayOverlappingNamesUpdate)
    EVT_UPDATE_UI(menu_VIEW_COMPASS, MainFrm::OnViewCompassUpdate)
    EVT_UPDATE_UI(menu_VIEW_CLINO, MainFrm::OnViewClinoUpdate)
    EVT_UPDATE_UI(menu_VIEW_DEPTH_BAR, MainFrm::OnToggleDepthbarUpdate)
    EVT_UPDATE_UI(menu_VIEW_SCALE_BAR, MainFrm::OnToggleScalebarUpdate)
    EVT_UPDATE_UI(menu_VIEW_GRID, MainFrm::OnViewGridUpdate)
    EVT_UPDATE_UI(menu_VIEW_INDICATORS, MainFrm::OnIndicatorsUpdate)
    EVT_UPDATE_UI(menu_VIEW_SIDE_PANEL, MainFrm::OnViewSidePanelUpdate)
    EVT_UPDATE_UI(menu_CTL_REVERSE, MainFrm::OnReverseControlsUpdate)
    EVT_UPDATE_UI(menu_CTL_CANCEL_DIST_LINE, MainFrm::OnCancelDistLineUpdate)
    EVT_UPDATE_UI(menu_VIEW_METRIC, MainFrm::OnToggleMetricUpdate)
    EVT_UPDATE_UI(menu_VIEW_DEGREES, MainFrm::OnToggleDegreesUpdate)
#ifdef AVENGL
    EVT_UPDATE_UI(menu_VIEW_SHOW_TUBES, MainFrm::OnToggleTubesUpdate)
#endif
END_EVENT_TABLE()

class LabelCmp {
    int separator;
public:
    LabelCmp(int separator_) : separator(separator_) {}
    bool operator()(const LabelInfo* pt1, const LabelInfo* pt2) {
	return name_cmp(pt1->GetText(), pt2->GetText(), separator) < 0;
    }
};

class LabelPlotCmp {
    int separator;
public:
    LabelPlotCmp(int separator_) : separator(separator_) {}
    bool operator()(const LabelInfo* pt1, const LabelInfo* pt2) {
	int n = pt1->flags - pt2->flags;
	if (n) return n > 0;
	wxString l1 = pt1->GetText().AfterLast(separator);
	wxString l2 = pt2->GetText().AfterLast(separator);
	return name_cmp(l1, l2, separator) < 0;
    }
};

#if wxUSE_DRAG_AND_DROP
class DnDFile : public wxFileDropTarget {
    public:
	DnDFile(MainFrm *parent) : m_Parent(parent) { }
	virtual bool OnDropFiles(wxCoord, wxCoord,
			const wxArrayString &filenames);

    private:
	MainFrm * m_Parent;
};

bool
DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString &filenames)
{
    // Load a survey file by drag-and-drop.
    assert(filenames.GetCount() > 0);

    if (filenames.GetCount() != 1) {
	wxGetApp().ReportError(msg(/*You may only view one 3d file at a time.*/336));
	return FALSE;
    }

    m_Parent->OpenFile(filenames[0]);
    return TRUE;
}
#endif

MainFrm::MainFrm(const wxString& title, const wxPoint& pos, const wxSize& size) :
    wxFrame(NULL, 101, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE),
    m_Gfx(NULL), m_NumEntrances(0), m_NumFixedPts(0), m_NumExportedPts(0)
{
#ifdef _WIN32
    // The peculiar name is so that the icon is the first in the file
    // (required by Microsoft Windows for this type of icon)
    SetIcon(wxIcon("aaaaaAven"));
#endif

    InitialisePensAndBrushes();
    CreateMenuBar();
    CreateToolBar();
    CreateSidePanel();

#ifdef __X__
    int x;
    int y;
    GetSize(&x, &y);
    // X seems to require a forced resize.
    SetSize(-1, -1, x, y);
#endif

#ifdef AVENPRES
    m_PresLoaded = false;
    m_Recording = false;
#endif

#if wxUSE_DRAG_AND_DROP
    SetDropTarget(new DnDFile(this));
#endif
}

MainFrm::~MainFrm()
{
    ClearPointLists();
    delete[] m_Points;
    delete[] m_Pens;
    delete[] m_Brushes;
}

void MainFrm::InitialisePensAndBrushes()
{
    m_Points = new list<PointInfo*>[NUM_DEPTH_COLOURS + 1];
    m_Pens = new GLAPen[NUM_DEPTH_COLOURS + 1];
    m_Brushes = new wxBrush[NUM_DEPTH_COLOURS + 1];
    for (int pen = 0; pen < NUM_DEPTH_COLOURS + 1; ++pen) {
	m_Pens[pen].SetColour(REDS[pen] / 255.0, GREENS[pen] / 255.0, BLUES[pen] / 255.0);
        m_Pens[pen].SetAlpha(1.0);
	m_Brushes[pen].SetColour(REDS[pen], GREENS[pen], BLUES[pen]);
    }
}

void MainFrm::CreateMenuBar()
{
    // Create the menus and the menu bar.

    wxMenu* filemenu = new wxMenu;
    filemenu->Append(menu_FILE_OPEN, GetTabMsg(/*@Open...##Ctrl+O*/220));
    filemenu->AppendSeparator();
    filemenu->Append(menu_FILE_QUIT, GetTabMsg(/*E@xit*/221));

    m_history.UseMenu(filemenu);
    m_history.Load(*wxConfigBase::Get());

    wxMenu* rotmenu = new wxMenu;
    rotmenu->Append(menu_ROTATION_START, GetTabMsg(/*@Start Rotation##Return*/230));
    rotmenu->Append(menu_ROTATION_STOP, GetTabMsg(/*S@top Rotation##Space*/231));
    rotmenu->AppendSeparator();
    rotmenu->Append(menu_ROTATION_SPEED_UP, GetTabMsg(/*Speed @Up*/232));
    rotmenu->Append(menu_ROTATION_SLOW_DOWN, GetTabMsg(/*Slow @Down*/233));
    rotmenu->AppendSeparator();
    rotmenu->Append(menu_ROTATION_REVERSE, GetTabMsg(/*@Reverse Direction*/234));
    rotmenu->AppendSeparator();
    rotmenu->Append(menu_ROTATION_STEP_CCW, GetTabMsg(/*Step Once @Anticlockwise*/235));
    rotmenu->Append(menu_ROTATION_STEP_CW, GetTabMsg(/*Step Once @Clockwise*/236));

    wxMenu* orientmenu = new wxMenu;
    orientmenu->Append(menu_ORIENT_MOVE_NORTH, GetTabMsg(/*View @North*/240));
    orientmenu->Append(menu_ORIENT_MOVE_EAST, GetTabMsg(/*View @East*/241));
    orientmenu->Append(menu_ORIENT_MOVE_SOUTH, GetTabMsg(/*View @South*/242));
    orientmenu->Append(menu_ORIENT_MOVE_WEST, GetTabMsg(/*View @West*/243));
    orientmenu->AppendSeparator();
    orientmenu->Append(menu_ORIENT_SHIFT_LEFT, GetTabMsg(/*Shift Survey @Left*/244));
    orientmenu->Append(menu_ORIENT_SHIFT_RIGHT, GetTabMsg(/*Shift Survey @Right*/245));
    orientmenu->Append(menu_ORIENT_SHIFT_UP, GetTabMsg(/*Shift Survey @Up*/246));
    orientmenu->Append(menu_ORIENT_SHIFT_DOWN, GetTabMsg(/*Shift Survey @Down*/247));
    orientmenu->AppendSeparator();
    orientmenu->Append(menu_ORIENT_PLAN, GetTabMsg(/*@Plan View*/248));
    orientmenu->Append(menu_ORIENT_ELEVATION, GetTabMsg(/*Ele@vation*/249));
    orientmenu->AppendSeparator();
    orientmenu->Append(menu_ORIENT_HIGHER_VP, GetTabMsg(/*@Higher Viewpoint*/250));
    orientmenu->Append(menu_ORIENT_LOWER_VP, GetTabMsg(/*L@ower Viewpoint*/251));
    orientmenu->AppendSeparator();
    orientmenu->Append(menu_ORIENT_ZOOM_IN, GetTabMsg(/*@Zoom In##]*/252));
    orientmenu->Append(menu_ORIENT_ZOOM_OUT, GetTabMsg(/*Zoo@m Out##[*/253));
    orientmenu->AppendSeparator();
    orientmenu->Append(menu_ORIENT_DEFAULTS, GetTabMsg(/*Restore De@fault Settings*/254));

    wxMenu* viewmenu = new wxMenu;
    viewmenu->Append(menu_VIEW_SHOW_NAMES, GetTabMsg(/*Station @Names##Ctrl+N*/270), "", true);
#ifdef AVENGL
    viewmenu->Append(menu_VIEW_SHOW_TUBES, GetTabMsg(/*Passage @Tubes*/346), "", true);
#endif
    viewmenu->Append(menu_VIEW_SHOW_CROSSES, GetTabMsg(/*@Crosses##Ctrl+X*/271), "", true);
    viewmenu->Append(menu_VIEW_GRID, GetTabMsg(/*@Grid##Ctrl+G*/297), "", true);
    viewmenu->AppendSeparator();
    viewmenu->Append(menu_VIEW_SHOW_LEGS, GetTabMsg(/*@Underground Survey Legs##Ctrl+L*/272), "", true);
    viewmenu->Append(menu_VIEW_SHOW_SURFACE, GetTabMsg(/*@Surface Survey Legs##Ctrl+F*/291), "", true);
    viewmenu->AppendSeparator();
    viewmenu->Append(menu_VIEW_SURFACE_DEPTH, GetTabMsg(/*@Altitude Colouring on Surface Surveys*/292), "", true);
    viewmenu->Append(menu_VIEW_SURFACE_DASHED, GetTabMsg(/*@Dashed Surface Surveys*/293), "", true);
    viewmenu->AppendSeparator();
    viewmenu->Append(menu_VIEW_SHOW_OVERLAPPING_NAMES, GetTabMsg(/*@Overlapping Names*/273), "", true);
    viewmenu->AppendSeparator();
    viewmenu->Append(menu_VIEW_SHOW_ENTRANCES, GetTabMsg(/*Highlight @Entrances*/294), "", true);
    viewmenu->Append(menu_VIEW_SHOW_FIXED_PTS, GetTabMsg(/*Highlight @Fixed Points*/295), "", true);
    viewmenu->Append(menu_VIEW_SHOW_EXPORTED_PTS, GetTabMsg(/*Highlight E@xported Points*/296), "", true);
            
    wxMenu* ctlmenu = new wxMenu;
    ctlmenu->Append(menu_CTL_REVERSE, GetTabMsg(/*@Reverse Sense##Ctrl+R*/280), "", true);
    ctlmenu->AppendSeparator();
    ctlmenu->Append(menu_CTL_CANCEL_DIST_LINE, GetTabMsg(/*@Cancel Measuring Line##Escape*/281));
    ctlmenu->AppendSeparator();
    wxMenu* indmenu = new wxMenu;
    indmenu->Append(menu_VIEW_COMPASS, GetTabMsg(/*@Compass*/274), "", true);
    indmenu->Append(menu_VIEW_CLINO, GetTabMsg(/*C@linometer*/275), "", true);
    indmenu->Append(menu_VIEW_DEPTH_BAR, GetTabMsg(/*@Depth Bar*/276), "", true);
    indmenu->Append(menu_VIEW_SCALE_BAR, GetTabMsg(/*@Scale Bar*/277), "", true);
    ctlmenu->Append(menu_VIEW_INDICATORS, GetTabMsg(/*@Indicators*/299), indmenu);
    ctlmenu->Append(menu_VIEW_SIDE_PANEL, GetTabMsg(/*@Side Panel*/337), "", true);
    ctlmenu->AppendSeparator();
    ctlmenu->Append(menu_VIEW_METRIC, GetTabMsg(/*@Metric*/342), "", true);
    ctlmenu->Append(menu_VIEW_DEGREES, GetTabMsg(/*@Degrees*/343), "", true);

    wxMenu* helpmenu = new wxMenu;
    helpmenu->Append(menu_HELP_ABOUT, GetTabMsg(/*@About...*/290));

    wxMenuBar* menubar = new wxMenuBar(wxMB_DOCKABLE);
    menubar->Append(filemenu, GetTabMsg(/*@File*/210));
    menubar->Append(rotmenu, GetTabMsg(/*@Rotation*/211));
    menubar->Append(orientmenu, GetTabMsg(/*@Orientation*/212));
    menubar->Append(viewmenu, GetTabMsg(/*@View*/213));
#ifdef AVENPRES
    menubar->Append(presmenu, GetTabMsg(/*@Presentation*/317));
#endif
    menubar->Append(ctlmenu, GetTabMsg(/*@Controls*/214));
    menubar->Append(helpmenu, GetTabMsg(/*@Help*/215));
    SetMenuBar(menubar);
}

void MainFrm::CreateToolBar()
{
    // Create the toolbar.

    wxToolBar* toolbar = wxFrame::CreateToolBar();

#ifndef _WIN32
    toolbar->SetMargins(5, 5);
#endif

    toolbar->AddTool(menu_FILE_OPEN, TOOLBAR_BITMAP("open.png"), "Open a 3D file for viewing");
#ifdef AVENPRES
    toolbar->AddTool(menu_FILE_OPEN_PRES, TOOLBAR_BITMAP("open-pres.png"), "Open a presentation");
#endif
    toolbar->AddSeparator();
    toolbar->AddTool(menu_ROTATION_TOGGLE, TOOLBAR_BITMAP("rotation.png"),
		     wxNullBitmap, true, -1, -1, NULL, "Toggle rotation");
    toolbar->AddSeparator();
    toolbar->AddTool(menu_ORIENT_PLAN, TOOLBAR_BITMAP("plan.png"), "Switch to plan view");
    toolbar->AddTool(menu_ORIENT_ELEVATION, TOOLBAR_BITMAP("elevation.png"), "Switch to elevation view");
    toolbar->AddSeparator();
    toolbar->AddTool(menu_ORIENT_DEFAULTS, TOOLBAR_BITMAP("defaults.png"), "Restore default view");
    toolbar->AddSeparator();
    toolbar->AddTool(menu_VIEW_SHOW_NAMES, TOOLBAR_BITMAP("names.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Show station names");
    toolbar->AddTool(menu_VIEW_SHOW_CROSSES, TOOLBAR_BITMAP("crosses.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Show crosses on stations");
    toolbar->AddTool(menu_VIEW_SHOW_ENTRANCES, TOOLBAR_BITMAP("entrances.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Highlight entrances");
    toolbar->AddTool(menu_VIEW_SHOW_FIXED_PTS, TOOLBAR_BITMAP("fixed-pts.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Highlight fixed points");
    toolbar->AddTool(menu_VIEW_SHOW_EXPORTED_PTS, TOOLBAR_BITMAP("exported-pts.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Highlight exported stations");
    toolbar->AddSeparator();
    toolbar->AddTool(menu_VIEW_SHOW_LEGS, TOOLBAR_BITMAP("ug-legs.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Show underground surveys");
    toolbar->AddTool(menu_VIEW_SHOW_SURFACE, TOOLBAR_BITMAP("surface-legs.png"), wxNullBitmap, true,
		     -1, -1, NULL, "Show surface surveys");
#ifdef AVENGL
    toolbar->AddSeparator();
    toolbar->AddTool(menu_VIEW_SHOW_TUBES, TOOLBAR_BITMAP("tubes.png"), wxNullBitmap, true,
                     -1, -1, NULL, "Show passage tubes");
#endif

#if 0 // FIXME: maybe...
    m_FindBox = new wxTextCtrl(toolbar, -1, "");
    toolbar->AddControl(m_FindBox);
#endif

    toolbar->Realize();
}

void MainFrm::CreateSidePanel()
{
    m_Splitter = new AvenSplitterWindow(this);

    m_Panel = new wxPanel(m_Splitter);
    m_Tree = new AvenTreeCtrl(this, m_Panel);
    wxPanel *find_panel = new wxPanel(m_Panel);
    m_Panel->Show(false);

    m_FindBox = new wxTextCtrl(find_panel, -1, "");
    wxButton *find_button, *hide_button;
    find_button = new wxButton(find_panel, button_FIND, msg(/*Find*/332));
    find_button->SetDefault();
    find_panel->SetDefaultItem(find_button);
    hide_button = new wxButton(find_panel, button_HIDE, msg(/*Hide*/333));
    m_RegexpCheckBox = new wxCheckBox(find_panel, -1,
				      msg(/*Regular expression*/334));
    m_Coords = new wxStaticText(find_panel, -1, "");
    m_StnCoords = new wxStaticText(find_panel, -1, "");
    //  m_MousePtr = new wxStaticText(find_panel, -1, "Mouse coordinates");
    m_StnName = new wxStaticText(find_panel, -1, "");
    m_StnAlt = new wxStaticText(find_panel, -1, "");
    m_Dist1 = new wxStaticText(find_panel, -1, "");
    m_Dist2 = new wxStaticText(find_panel, -1, "");
    m_Dist3 = new wxStaticText(find_panel, -1, "");
    m_Found = new wxStaticText(find_panel, -1, "");

    wxBoxSizer *find_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    find_button_sizer->Add(m_FindBox, 1, wxALL, 2);
#ifdef _WIN32
    find_button_sizer->Add(find_button, 0, wxALL, 2);
#else
    // GTK+ (and probably Motif) default buttons have a thick external
    // border we need to allow for
    find_button_sizer->Add(find_button, 0, wxALL, 4);
#endif

    wxBoxSizer *hide_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    hide_button_sizer->Add(m_Found, 1, wxALL, 2);
#ifdef _WIN32
    hide_button_sizer->Add(hide_button, 0, wxALL, 2);
#else
    hide_button_sizer->Add(hide_button, 0, wxALL, 4);
#endif

    wxBoxSizer *find_sizer = new wxBoxSizer(wxVERTICAL);
    find_sizer->Add(find_button_sizer, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(hide_button_sizer, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_RegexpCheckBox, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(10, 5, 0, wxALL | wxEXPAND, 2);
    //   find_sizer->Add(m_MousePtr, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_Coords, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(10, 5, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_StnName, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_StnCoords, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_StnAlt, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(10, 5, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_Dist1, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_Dist2, 0, wxALL | wxEXPAND, 2);
    find_sizer->Add(m_Dist3, 0, wxALL | wxEXPAND, 2);

    find_panel->SetAutoLayout(true);
    find_panel->SetSizer(find_sizer);
    find_sizer->Fit(find_panel);
    find_sizer->SetSizeHints(find_panel);

    wxBoxSizer *panel_sizer = new wxBoxSizer(wxVERTICAL);
    panel_sizer->Add(m_Tree, 1, wxALL | wxEXPAND, 2);
    panel_sizer->Add(find_panel, 0, wxALL | wxEXPAND, 2);
    m_Panel->SetAutoLayout(true);
    m_Panel->SetSizer(panel_sizer);
//    panel_sizer->Fit(m_Panel);
//    panel_sizer->SetSizeHints(m_Panel);

    m_Control = new GUIControl();
    m_Gfx = new GfxCore(this, m_Splitter, m_Control);
    m_Control->SetView(m_Gfx);

    m_Splitter->Initialize(m_Gfx);
}

void MainFrm::ClearPointLists()
{
    // Free memory occupied by the contents of the point and label lists.

    for (int band = 0; band < NUM_DEPTH_COLOURS + 1; band++) {
	list<PointInfo*>::iterator pos = m_Points[band].begin();
	list<PointInfo*>::iterator end = m_Points[band].end();
	while (pos != end) {
	    PointInfo* point = *pos++;
	    delete point;
	}
	m_Points[band].clear();
    }

    list<LabelInfo*>::iterator pos = m_Labels.begin();
    while (pos != m_Labels.end()) {
	LabelInfo* label = *pos++;
	delete label;
    }
    m_Labels.clear();
}

bool MainFrm::LoadData(const wxString& file, wxString prefix)
{
    // Load survey data from file, centre the dataset around the origin,
    // chop legs such that no legs cross depth colour boundaries and prepare
    // the data for drawing.

    // Load the survey data.
#if 0
    wxStopWatch timer;
    timer.Start();
#endif

    img* survey = img_open_survey(file, prefix.c_str());
    if (!survey) {
	wxString m = wxString::Format(msg(img_error()), file.c_str());
	wxGetApp().ReportError(m);
	return false;
    }

    m_File = file;

    m_Tree->DeleteAllItems();

    m_TreeRoot = m_Tree->AddRoot(wxFileNameFromPath(file));
    m_Tree->SetEnabled();

    // Create a list of all the leg vertices, counting them and finding the
    // extent of the survey at the same time.

    m_NumLegs = 0;
    m_NumPoints = 0;
    m_NumExtraLegs = 0;
    m_NumCrosses = 0;
    m_NumFixedPts = 0;
    m_NumExportedPts = 0;
    m_NumEntrances = 0;

#ifdef AVENPRES
    m_PresLoaded = false;
    m_Recording = false;
    //--Pres: FIXME: discard existing one, ask user about saving
#endif

    // Delete any existing list entries.
    ClearPointLists();

    Double xmin = DBL_MAX;
    Double xmax = -DBL_MAX;
    Double ymin = DBL_MAX;
    Double ymax = -DBL_MAX;
    m_ZMin = DBL_MAX;
    Double zmax = -DBL_MAX;

    list<PointInfo*> points;

    int result;
    do {
	img_point pt;
	result = img_read_item(survey, &pt);
	switch (result) {
	    case img_MOVE:
	    case img_LINE:
	    {
		m_NumPoints++;

		// Update survey extents.
		if (pt.x < xmin) xmin = pt.x;
		if (pt.x > xmax) xmax = pt.x;
		if (pt.y < ymin) ymin = pt.y;
		if (pt.y > ymax) ymax = pt.y;
		if (pt.z < m_ZMin) m_ZMin = pt.z;
		if (pt.z > zmax) zmax = pt.z;

		PointInfo* info = new PointInfo;
		info->x = pt.x;
		info->y = pt.y;
		info->z = pt.z;

		if (result == img_LINE) {
		    // Set flags to say this is a line rather than a move
		    m_NumLegs++;
		    info->isLine = true;
		    info->isSurface = (survey->flags & img_FLAG_SURFACE);
		} else {
		    info->isLine = false;
		}

		// Store this point in the list.
		points.push_back(info);

		break;
	    }

	    case img_LABEL:
	    {
		LabelInfo* label = new LabelInfo;
		label->text = survey->label;
		label->x = pt.x;
		label->y = pt.y;
		label->z = pt.z;
		if (survey->flags & img_SFLAG_ENTRANCE) {
		    survey->flags ^= (img_SFLAG_ENTRANCE | LFLAG_ENTRANCE);
		}
		label->flags = survey->flags;
		if (label->IsEntrance()) {
		    m_NumEntrances++;
		}
		if (label->IsFixedPt()) {
		    m_NumFixedPts++;
		}
		if (label->IsExportedPt()) {
		    m_NumExportedPts++;
		}
		m_Labels.push_back(label);
		m_NumCrosses++;

		break;
	    }

	    case img_BAD:
	    {
		m_Labels.clear();

		// FIXME: Do we need to reset all these? - Olly
		m_NumLegs = 0;
		m_NumPoints = 0;
		m_NumExtraLegs = 0;
		m_NumCrosses = 0;
		m_NumFixedPts = 0;
		m_NumExportedPts = 0;
		m_NumEntrances = 0;

		m_ZMin = DBL_MAX;

		img_close(survey);

		wxString m = wxString::Format(msg(img_error()), file.c_str());
		wxGetApp().ReportError(m);

		return false;
	    }

	    default:
		break;
	}
    } while (result != img_STOP);

    separator = survey->separator;
    img_close(survey);

    // Check we've actually loaded some legs or stations!
    if (m_NumLegs == 0 && m_Labels.empty()) {
	wxString m = wxString::Format(msg(/*No survey data in 3d file `%s'*/202), file.c_str());
	wxGetApp().ReportError(m);
	return false;
    }

    if (points.empty()) {
	// No legs, so get survey extents from stations
	list<LabelInfo*>::const_iterator i;
	for (i = m_Labels.begin(); i != m_Labels.end(); ++i) {
	    if ((*i)->x < xmin) xmin = (*i)->x;
	    if ((*i)->x > xmax) xmax = (*i)->x;
	    if ((*i)->y < ymin) ymin = (*i)->y;
	    if ((*i)->y > ymax) ymax = (*i)->y;
	    if ((*i)->z < m_ZMin) m_ZMin = (*i)->z;
	    if ((*i)->z > zmax) zmax = (*i)->z;
	}
    } else {
	// Delete any trailing move.
	PointInfo* pt = points.back();
	if (!pt->isLine) {
	    m_NumPoints--;
	    points.pop_back();
	    delete pt;
	}
    }

    m_XExt = xmax - xmin;
    m_YExt = ymax - ymin;
    m_ZExt = zmax - m_ZMin;

    // FIXME -- temporary bodge
    m_XMin = xmin;
    m_YMin = ymin;

    // Sort the labels.
    m_Labels.sort(LabelCmp(separator));

    // Fill the tree of stations and prefixes.
    FillTree();
    m_Tree->Expand(m_TreeRoot);

    // Sort labels so that entrances are displayed in preference,
    // then fixed points, then exported points, then other points.
    //
    // Also sort by leaf name so that we'll tend to choose labels
    // from different surveys, rather than labels from surveys which
    // are earlier in the list.
    m_Labels.sort(LabelPlotCmp(separator));

    // Sort out depth colouring boundaries (before centering dataset!)
    SortIntoDepthBands(points);

    // Centre the dataset around the origin.
    CentreDataset(xmin, ymin, m_ZMin);

#if 0
    printf("time to load = %.3f\n", (double)timer.Time());
#endif

    // Update window title.
    SetTitle(wxString("Aven - [") + file + wxString("]"));
    m_File = file;

    return true;
}

void MainFrm::FillTree()
{
    // Fill the tree of stations and prefixes.

    list<LabelInfo*>::iterator pos = m_Labels.begin();
    stack<wxTreeItemId> previous_ids;
    wxString current_prefix = "";
    wxTreeItemId current_id = m_TreeRoot;

    while (pos != m_Labels.end()) {
	LabelInfo* label = *pos++;

	// Determine the current prefix.
	wxString prefix = label->GetText().BeforeLast(separator);

	// Determine if we're still on the same prefix.
	if (prefix == current_prefix) {
	    // no need to fiddle with branches...
	}
	// If not, then see if we've descended to a new prefix.
	else if (prefix.Length() > current_prefix.Length() &&
		 prefix.StartsWith(current_prefix) &&
		 (prefix[current_prefix.Length()] == separator ||
		  current_prefix == "")) {
	    // We have, so start as many new branches as required.
	    int current_prefix_length = current_prefix.Length();
	    current_prefix = prefix;
	    if (current_prefix_length != 0) {
		prefix = prefix.Mid(current_prefix_length + 1);
	    }
	    int next_dot;
	    do {
		// Extract the next bit of prefix.
		next_dot = prefix.Find(separator);

		wxString bit = next_dot == -1 ? prefix : prefix.Left(next_dot);
		assert(bit != "");

		// Add the current tree ID to the stack.
		previous_ids.push(current_id);

		// Append the new item to the tree and set this as the current branch.
		current_id = m_Tree->AppendItem(current_id, bit);
		m_Tree->SetItemData(current_id, new TreeData(NULL));
		prefix = prefix.Mid(next_dot + 1);
	    } while (next_dot != -1);
	}
	// Otherwise, we must have moved up, and possibly then down again.
	else {
	    size_t count = 0;
	    bool ascent_only = (prefix.Length() < current_prefix.Length() &&
				current_prefix.StartsWith(prefix) &&
				(current_prefix[prefix.Length()] == separator ||
				 prefix == ""));
	    if (!ascent_only) {
		// Find out how much of the current prefix and the new prefix
		// are the same.
		// Note that we require a match of a whole number of parts
		// between dots!
		size_t pos = 0;
		while (prefix[pos] == current_prefix[pos]) {
		    if (prefix[pos] == separator) count = pos + 1;
		    pos++;
		}
	    }
	    else {
		count = prefix.Length() + 1;
	    }

	    // Extract the part of the current prefix after the bit (if any)
	    // which has matched.
	    // This gives the prefixes to ascend over.
	    wxString prefixes_ascended = current_prefix.Mid(count);

	    // Count the number of prefixes to ascend over.
	    int num_prefixes = prefixes_ascended.Freq(separator);

	    // Reverse up over these prefixes.
	    for (int i = 1; i <= num_prefixes; i++) {
		previous_ids.pop();
	    }
	    current_id = previous_ids.top();
	    previous_ids.pop();

	    if (!ascent_only) {
		// Now extract the bit of new prefix.
		wxString new_prefix = prefix.Mid(count);

		// Add branches for this new part.
		while (1) {
		    // Extract the next bit of prefix.
		    int next_dot = new_prefix.Find(separator);

		    wxString bit;
		    if (next_dot == -1) {
			bit = new_prefix;
		    } else {
			bit = new_prefix.Left(next_dot);
		    }

		    // Add the current tree ID to the stack.
		    previous_ids.push(current_id);

		    // Append the new item to the tree and set this as the
		    // current branch.
		    current_id = m_Tree->AppendItem(current_id, bit);
		    m_Tree->SetItemData(current_id, new TreeData(NULL));

		    if (next_dot == -1) break;

		    new_prefix = new_prefix.Mid(next_dot + 1);
		}
	    }

	    current_prefix = prefix;
	}

	// Now add the leaf.
	wxString bit = label->GetText().AfterLast(separator);
	assert(bit != "");
	wxTreeItemId id = m_Tree->AppendItem(current_id, bit);
	m_Tree->SetItemData(id, new TreeData(label));
	label->tree_id = id; // before calling SetTreeItemColour()...
	SetTreeItemColour(label);
    }
}

void MainFrm::SelectTreeItem(LabelInfo* label)
{
    m_Tree->SelectItem(label->tree_id);
}

void MainFrm::SetTreeItemColour(LabelInfo* label)
{
    // Set the colour for an item in the survey tree.

    if (label->IsSurface()) {
	m_Tree->SetItemTextColour(label->tree_id, wxColour(49, 158, 79));
    }

    if (label->IsEntrance()) {
	// FIXME: making this red here doesn't match with entrance blobs
	// being green...
	m_Tree->SetItemTextColour(label->tree_id, wxColour(255, 0, 0));
    }
}

void MainFrm::CentreDataset(Double xmin, Double ymin, Double zmin)
{
    // Centre the dataset around the origin.

    Double xoff = m_Offsets.x = xmin + (m_XExt / 2.0);
    Double yoff = m_Offsets.y = ymin + (m_YExt / 2.0);
    Double zoff = m_Offsets.z = zmin + (m_ZExt / 2.0);

    for (int band = 0; band < NUM_DEPTH_COLOURS + 1; band++) {
	list<PointInfo*>::iterator pos = m_Points[band].begin();
	list<PointInfo*>::iterator end = m_Points[band].end();
	while (pos != end) {
	    PointInfo* point = *pos++;
	    point->x -= xoff;
	    point->y -= yoff;
	    point->z -= zoff;
	}
    }

    list<LabelInfo*>::iterator pos = m_Labels.begin();
    while (pos != m_Labels.end()) {
	LabelInfo* label = *pos++;
	label->x -= xoff;
	label->y -= yoff;
	label->z -= zoff;
    }
}

int MainFrm::GetDepthColour(Double z)
{
    // Return the (0-based) depth colour band index for a z-coordinate.
    return int(((z - m_ZMin) / (m_ZExt == 0.0 ? 1.0 : m_ZExt)) * (NUM_DEPTH_COLOURS - 1));
}

Double MainFrm::GetDepthBoundaryBetweenBands(int a, int b)
{
    // Return the z-coordinate of the depth colour boundary between
    // two adjacent depth colour bands (specified by 0-based indices).

    assert((a == b - 1) || (a == b + 1));

    int band = (a > b) ? a : b; // boundary N lies on the bottom of band N.
    return m_ZMin + (m_ZExt * band / (NUM_DEPTH_COLOURS == 1 ? 1 : NUM_DEPTH_COLOURS - 1));
}

void MainFrm::IntersectLineWithPlane(Double x0, Double y0, Double z0,
				     Double x1, Double y1, Double z1,
				     Double z, Double& x, Double& y)
{
    // Find the intersection point of the line (x0, y0, z0) -> (x1, y1, z1)
    // with the plane parallel to the xy-plane with z-axis intersection z.
    assert(z1 - z0 != 0.0);

    Double t = (z - z0) / (z1 - z0);
    x = x0 + t*(x1 - x0);
    y = y0 + t*(y1 - y0);
}

void MainFrm::SortIntoDepthBands(list<PointInfo*>& points)
{
    // Split legs which cross depth colouring boundaries and classify all
    // points into the correct depth bands.

    list<PointInfo*>::iterator pos = points.begin();
    PointInfo* prev_point = NULL;
    while (pos != points.end()) {
	PointInfo* point = *pos++;
	assert(point);

	// If this is a leg, then check if it intersects a depth
	// colour boundary.
	if (point->isLine) {
	    assert(prev_point);
	    int col1 = GetDepthColour(prev_point->z);
	    int col2 = GetDepthColour(point->z);
	    if (col1 != col2) {
		// The leg does cross at least one boundary, so split it as
		// many times as required...
		int inc = (col1 > col2) ? -1 : 1;
		for (int band = col1; band != col2; band += inc) {
		    int next_band = band + inc;

		    // Determine the z-coordinate of the boundary being
		    // intersected.
		    Double split_at_z = GetDepthBoundaryBetweenBands(band, next_band);
		    Double split_at_x, split_at_y;

		    // Find the coordinates of the intersection point.
		    IntersectLineWithPlane(prev_point->x, prev_point->y, prev_point->z,
					   point->x, point->y, point->z,
					   split_at_z, split_at_x, split_at_y);

		    // Create a new leg only as far as this point.
		    PointInfo* info = new PointInfo;
		    info->x = split_at_x;
		    info->y = split_at_y;
		    info->z = split_at_z;
		    info->isLine = true;
		    info->isSurface = point->isSurface;
		    m_Points[band].push_back(info);

		    // Create a move to this point in the next band.
		    info = new PointInfo;
		    info->x = split_at_x;
		    info->y = split_at_y;
		    info->z = split_at_z;
		    info->isLine = false;
		    info->isSurface = point->isSurface;
		    m_Points[next_band].push_back(info);

		    m_NumExtraLegs++;
		    m_NumPoints += 2;
		}
	    }

	    // Add the last point of the (possibly split) leg.
	    m_Points[col2].push_back(point);
	}
	else {
	    // The first point, a surface point, or another move: put it in the
	    // correct list according to depth.
	    int band = GetDepthColour(point->z);
	    m_Points[band].push_back(point);
	}

	prev_point = point;
    }
}

void MainFrm::OnMRUFile(wxCommandEvent& event)
{
    wxString f(m_history.GetHistoryFile(event.GetId() - wxID_FILE1));
    if (!f.empty()) OpenFile(f);
}

void MainFrm::OpenFile(const wxString& file, wxString survey, bool delay)
{
    wxBusyCursor hourglass;
    if (LoadData(file, survey)) {
	if (wxIsAbsolutePath(file)) {
	    m_history.AddFileToHistory(file);
	} else {
	    wxString abs = wxGetCwd() + wxString(FNM_SEP_LEV) + file;
	    m_history.AddFileToHistory(abs);
	}
	wxConfigBase *b = wxConfigBase::Get();
	m_history.Save(*b);
	b->Flush();

	if (delay) {
	    m_Gfx->InitialiseOnNextResize();
	}
	else {
	    m_Gfx->Initialise();
	}

	m_Panel->Show(true);
	int x;
	int y;
	GetSize(&x, &y);
	if (x < 600)
	    x /= 3;
	else if (x < 1000)
	    x = 200;
	else
	    x /= 5;

	m_Splitter->SplitVertically(m_Panel, m_Gfx, x);

	m_SashPosition = m_Splitter->GetSashPosition(); // save width of panel

	m_Gfx->SetFocus();
    }
}

//
//  UI event handlers
//

#undef FILEDIALOG_MULTIGLOBS
// MS Windows supports "*.abc;*.def" natively; wxGtk supports them as of 2.3
#if defined(_WIN32) || (wxMAJOR_VERSION > 2) || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION >= 3)
# define FILEDIALOG_MULTIGLOBS
#endif

void MainFrm::OnOpen(wxCommandEvent&)
{
#ifdef __WXMOTIF__
    wxFileDialog dlg (this, wxString(msg(/*Select a 3d file to view*/206)), "", "",
		      "*.3d", wxOPEN);
#else
    wxFileDialog dlg (this, wxString(msg(/*Select a 3d file to view*/206)), "", "",
		      wxString::Format("%s|*.3d"
#ifdef FILEDIALOG_MULTIGLOBS
				       ";*.3D"
#endif
#ifdef FILEDIALOG_MULTIGLOBS 
				       "|%s|*.plt;*.plf"
#ifndef _WIN32
				       ";*.PLT;*.PLF"
#endif
#else
				       "|%s|*.pl?" // not ideal...
#endif
			      	       "|%s|*.xyz"
#ifdef FILEDIALOG_MULTIGLOBS
#ifndef _WIN32
				       ";*.XYZ"
#endif
#endif
				       "|%s|*.*",
				       msg(/*Survex 3d files*/207),
				       /* FIXME TRANSLATE */
				       "Compass PLT files",
				       "CMAP XYZ files",
				       msg(/*All files*/208)), wxOPEN);
#endif
    if (dlg.ShowModal() == wxID_OK) {
	OpenFile(dlg.GetPath());
    }
}


void MainFrm::OnQuit(wxCommandEvent&)
{
    exit(0);
}

void MainFrm::OnClose(wxCloseEvent&)
{
    exit(0);
}

void MainFrm::OnAbout(wxCommandEvent&)
{
    wxDialog* dlg = new AboutDlg(this);
    dlg->Centre();
    dlg->ShowModal();
}

void MainFrm::GetColour(int band, Double& r, Double& g, Double& b) const
{
    assert(band >= 0 && band < NUM_DEPTH_COLOURS);
    r = Double(REDS[band]) / 255.0;
    g = Double(GREENS[band]) / 255.0;
    b = Double(BLUES[band]) / 255.0;
}

void MainFrm::ClearTreeSelection()
{
    m_Tree->UnselectAll();
    m_Gfx->SetThere();
}

void MainFrm::ClearCoords()
{
    m_Coords->SetLabel("");
}

void MainFrm::SetCoords(Double x, Double y)
{
    wxString str;
    if (m_Gfx->GetMetric()) {
	str.Printf(msg(/*  %d E, %d N*/338), int(x), int(y));
    } else {
	str.Printf(msg(/*  %d E, %d N*/338),
		   int(x / METRES_PER_FOOT), int(y / METRES_PER_FOOT));
    }
    m_Coords->SetLabel(str);
}

void MainFrm::SetAltitude(Double z)
{
    wxString str;
    if (m_Gfx->GetMetric()) {
	str.Printf("  %s %dm", msg(/*Altitude*/335),                                               int(z));
    } else {
	str.Printf("  %s %dft", msg(/*Altitude*/335),
		   int(z / METRES_PER_FOOT));
    }
    m_Coords->SetLabel(str);
}

void MainFrm::ShowInfo(const LabelInfo *label)
{
    assert(m_Gfx);
	
    wxString str;
    if (m_Gfx->GetMetric()) {
	str.Printf(msg(/*  %d E, %d N*/338),
		   int(label->x + m_Offsets.x),
		   int(label->y + m_Offsets.y));
    } else {
	str.Printf(msg(/*  %d E, %d N*/338),
		   int((label->x + m_Offsets.x) / METRES_PER_FOOT),
		   int((label->y + m_Offsets.y) / METRES_PER_FOOT));
    }
    m_StnCoords->SetLabel(str);
    m_StnName->SetLabel(label->text);

    if (m_Gfx->GetMetric()) {
	str.Printf("  %s %dm", msg(/*Altitude*/335),
		   int(label->z + m_Offsets.z));
    } else {
	str.Printf("  %s %dft", msg(/*Altitude*/335),
		   int((label->z + m_Offsets.z) / METRES_PER_FOOT));
    }
    m_StnAlt->SetLabel(str);
    m_Gfx->SetHere(label->x, label->y, label->z);

    wxTreeItemData* sel_wx;
    bool sel = m_Tree->GetSelectionData(&sel_wx);
    if (sel) {
	TreeData *data = (TreeData*) sel_wx;

	if (data->IsStation()) {
	    const LabelInfo* label2 = data->GetLabel();
	    assert(label2);

	    Double x0 = label2->x;
	    Double x1 = label->x;
	    Double dx = x1 - x0;
	    Double y0 = label2->y;
	    Double y1 = label->y;
	    Double dy = y1 - y0;
	    Double z0 = label2->z;
	    Double z1 = label->z;
	    Double dz = z1 - z0;

	    Double d_horiz = sqrt(dx*dx + dy*dy);
	    Double dr = sqrt(dx*dx + dy*dy + dz*dz);

	    Double brg = atan2(dx, dy) * 180.0 / M_PI;
	    if (brg < 0) brg += 360;

	    str.Printf(msg(/*From %s*/339), label2->text.c_str());
	    m_Dist1->SetLabel(str);
	    if (m_Gfx->GetMetric()) {
		str.Printf(msg(/*  H %d%s, V %d%s*/340),
			   int(d_horiz), "m",
			   int(dz), "m");
	    } else {
		str.Printf(msg(/*  H %d%s, V %d%s*/340),
			   int(d_horiz / METRES_PER_FOOT), "ft",
			   int(dz / METRES_PER_FOOT), "ft");
	    }
	    m_Dist2->SetLabel(str);
	    wxString brg_unit;
	    if (m_Gfx->GetDegrees()) {
		brg_unit = msg(/*&deg;*/344);
	    } else {
		brg *= 400.0 / 360.0;
		brg_unit = msg(/*grad*/345);
	    }
	    if (m_Gfx->GetMetric()) {
		str.Printf(msg(/*  Dist %d%s, Brg %03d%s*/341),
			   int(dr), "m", int(brg), brg_unit.c_str());
	    } else {
		str.Printf(msg(/*  Dist %d%s, Brg %03d%s*/341),
			   int(dr / METRES_PER_FOOT), "ft", int(brg),
			   brg_unit.c_str());
	    }
	    m_Dist3->SetLabel(str);
	    m_Gfx->SetThere(x0, y0, z0);
	} else {
	    m_Gfx->SetThere(); // FIXME: not in SetMouseOverStation version?
	}
    }
}

void MainFrm::DisplayTreeInfo(const wxTreeItemData* item)
{
    const TreeData* data = static_cast<const TreeData*>(item);
    if (data) {
	if (data->IsStation()) {
	    ShowInfo(data->GetLabel());
	} else {
	    m_StnName->SetLabel("");
	    m_StnCoords->SetLabel("");
	    m_StnAlt->SetLabel("");
	    m_Gfx->SetHere();
	    m_Dist1->SetLabel("");
	    m_Dist2->SetLabel("");
	    m_Dist3->SetLabel("");
	}
    }
}

void MainFrm::TreeItemSelected(wxTreeItemData* item)
{
    TreeData* data = (TreeData*) item;

    if (data && data->IsStation()) {
	const LabelInfo* label = data->GetLabel();
	m_Gfx->CentreOn(label->x, label->y, label->z);
    }

    m_Dist1->SetLabel("");
    m_Dist2->SetLabel("");
    m_Dist3->SetLabel("");
}

#ifdef AVENPRES
void MainFrm::OnPresCreate(wxCommandEvent& event)
{
#ifdef __WXMOTIF__
    wxFileDialog dlg (this, wxString(msg(/*Select an output filename*/319)), "", "",
		      "*.avp", wxSAVE);
#else
    wxFileDialog dlg (this, wxString(msg(/*Select an output filename*/319)), "", "",
		      wxString::Format("%s|*.avp|%s|*.*",
				       msg(/*Aven presentations*/320),
				       msg(/*All files*/208)), wxSAVE);
#endif
    if (dlg.ShowModal() == wxID_OK) {
	m_PresFP = fopen(dlg.GetPath().c_str(), "w");
	assert(m_PresFP); //--Pres: FIXME

	// Update window title.
	SetTitle(wxString("Aven - [") + m_File + wxString("] - ") +
		 wxString(msg(/*Recording Presentation*/323)));

	//--Pres: FIXME: discard existing one
	m_PresLoaded = true;
	m_Recording = true;
    }
}

void MainFrm::OnPresGo(wxCommandEvent& event)
{
    assert(m_PresLoaded && !m_Recording); //--Pres: FIXME

    m_Gfx->PresGo();
}

void MainFrm::OnPresGoBack(wxCommandEvent& event)
{
    assert(m_PresLoaded && !m_Recording); //--Pres: FIXME

    m_Gfx->PresGoBack();
}

void MainFrm::OnPresRecord(wxCommandEvent& event)
{
    assert(m_PresLoaded && m_Recording); //--Pres: FIXME

    m_Gfx->RecordPres(m_PresFP);
}

void MainFrm::OnPresFinish(wxCommandEvent& event)
{
    assert(m_PresFP); //--Pres: FIXME
    fclose(m_PresFP);

    // Update window title.
    SetTitle(wxString("Aven - [") + m_File + wxString("]"));
}

void MainFrm::OnPresRestart(wxCommandEvent& event)
{
    assert(m_PresLoaded && !m_Recording); //--Pres: FIXME

    m_Gfx->RestartPres();
}

void MainFrm::OnPresErase(wxCommandEvent& event)
{
    assert(m_PresLoaded && m_Recording); //--Pres: FIXME


}

void MainFrm::OnPresEraseAll(wxCommandEvent& event)
{
    assert(m_PresLoaded && m_Recording); //--Pres: FIXME


}

void MainFrm::OnOpenPres(wxCommandEvent& event)
{
#ifdef __WXMOTIF__
    wxFileDialog dlg (this, wxString(msg(/*Select a presentation to open*/322)), "", "",
		      "*.avp", wxSAVE);
#else
    wxFileDialog dlg (this, wxString(msg(/*Select a presentation to open*/322)), "", "",
		      wxString::Format("%s|*.avp|%s|*.*",
				       msg(/*Aven presentations*/320),
				       msg(/*All files*/208)), wxOPEN);
#endif
    if (dlg.ShowModal() == wxID_OK) {
	m_PresFP = fopen(dlg.GetPath(), "rb");
	assert(m_PresFP); //--Pres: FIXME

	m_Gfx->LoadPres(m_PresFP);

	fclose(m_PresFP);

	m_PresLoaded = true;
	m_Recording = false;
    }
}
#endif

void MainFrm::OnFileOpenTerrainUpdate(wxUpdateUIEvent& event)
{
    event.Enable(!m_File.empty());
}

#ifdef AVENPRES
void MainFrm::OnOpenPresUpdate(wxUpdateUIEvent& event)
{
    event.Enable(!m_File.empty());
}

void MainFrm::OnPresCreateUpdate(wxUpdateUIEvent& event)
{
    event.Enable(!m_PresLoaded && !m_File.empty());
}

void MainFrm::OnPresGoUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && !m_Recording && !m_Gfx->AtEndOfPres());
}

void MainFrm::OnPresGoBackUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && !m_Recording && !m_Gfx->AtStartOfPres());
}

void MainFrm::OnPresFinishUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && m_Recording);
}

void MainFrm::OnPresRestartUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && !m_Recording && !m_Gfx->AtStartOfPres());
}

void MainFrm::OnPresRecordUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && m_Recording);
}

void MainFrm::OnPresEraseUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && m_Recording); //--Pres: FIXME
}

void MainFrm::OnPresEraseAllUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_PresLoaded && m_Recording); //--Pres: FIXME
}
#endif

void MainFrm::OnFind(wxCommandEvent& event)
{
    wxBusyCursor hourglass;
    // Find stations specified by a string or regular expression.

    wxString str = m_FindBox->GetValue();
    int cflags = REG_NOSUB;

    if (true /* case insensitive */) {
	cflags |= REG_ICASE;
    }

    if (m_RegexpCheckBox->GetValue()) {
	cflags |= REG_EXTENDED;
    } else if (false /* simple glob-style */) {
	wxString pat;
	for (size_t i = 0; i < str.size(); i++) {
	   char ch = str[i];
	   // ^ only special at start; $ at end.  But this is simpler...
	   switch (ch) {
	    case '^': case '$': case '.': case '[': case '\\':
	      pat += '\\';
	      pat += ch;
	      break;
	    case '*':
	      pat += ".*";
	      break;
	    case '?':
	      pat += '.';
	      break;
	    default:
	      pat += ch;
	   }
	}
	str = pat;
    } else {
	wxString pat;
	for (size_t i = 0; i < str.size(); i++) {
	   char ch = str[i];
	   // ^ only special at start; $ at end.  But this is simpler...
	   switch (ch) {
	    case '^': case '$': case '*': case '.': case '[': case '\\':
	      pat += '\\';
	   }
	   pat += ch;
	}
	str = pat;
    }

    if (!true /* !substring */) {
	// FIXME "0u" required to avoid compilation error with g++-3.0
	if (str.empty() || str[0u] != '^') str = '^' + str;
	if (str[str.size() - 1] != '$') str = str + '$';
    }

    regex_t buffer;
    int errcode = regcomp(&buffer, str.c_str(), cflags);
    if (errcode) {
	size_t len = regerror(errcode, &buffer, NULL, 0);
	char *msg = new char[len];
	regerror(errcode, &buffer, msg, len);
	wxGetApp().ReportError(msg);
	delete[] msg;
	return;
    }

    list<LabelInfo*>::iterator pos = m_Labels.begin();

    int found = 0;
    while (pos != m_Labels.end()) {
	LabelInfo* label = *pos++;

	if (regexec(&buffer, label->text.c_str(), 0, NULL, 0) == 0) {
	    label->flags |= LFLAG_HIGHLIGHTED;
	    found++;
	} else {
	    label->flags &= ~LFLAG_HIGHLIGHTED;
	}
    }

    regfree(&buffer);

    m_Found->SetLabel(wxString::Format(msg(/*%d found*/331), found));
#ifdef _WIN32
    m_Found->Refresh(); // FIXME
#endif
    // Re-sort so highlighted points get names in preference
    if (found) m_Labels.sort(LabelPlotCmp(separator));
    m_Gfx->ForceRefresh();

#if 0
    if (!found) {
	wxGetApp().ReportError(msg(/*No matches were found.*/328));
    }
#endif

    m_Gfx->SetFocus();
}

void MainFrm::OnHide(wxCommandEvent& event)
{
    // Hide any search result highlights.
    m_Found->SetLabel("");
    list<LabelInfo*>::iterator pos = m_Labels.begin();
    while (pos != m_Labels.end()) {
	LabelInfo* label = *pos++;
	label->flags &= ~LFLAG_HIGHLIGHTED;
    }
    m_Gfx->ForceRefresh();
}

void MainFrm::SetMouseOverStation(LabelInfo* label)
{
    if (label) {
	ShowInfo(label);
    } else {
	m_StnName->SetLabel("");
	m_StnCoords->SetLabel("");
	m_StnAlt->SetLabel("");
	m_Gfx->SetHere();
	m_Dist1->SetLabel("");
	m_Dist2->SetLabel("");
	m_Dist3->SetLabel("");
    }
}

void MainFrm::OnViewSidePanel(wxCommandEvent&)
{
    ToggleSidePanel();
}

void MainFrm::ToggleSidePanel()
{
    // Toggle display of the side panel.

    assert(m_Gfx);

    if (m_Splitter->IsSplit()) {
	m_SashPosition = m_Splitter->GetSashPosition(); // save width of panel
	m_Splitter->Unsplit(m_Panel);
    }
    else {
	m_Panel->Show(true);
	m_Gfx->Show(true);
	m_Splitter->SplitVertically(m_Panel, m_Gfx, m_SashPosition);
    }
}

void MainFrm::OnViewSidePanelUpdate(wxUpdateUIEvent& ui)
{
    ui.Enable(!m_File.empty());
    ui.Check(m_Splitter->IsSplit());
}
