//
//  guicontrol.h
//
//  Handlers for events relating to the display of a survey.
//
//  Copyright (C) 2000-2002 Mark R. Shinwell
//  Copyright (C) 2001-2003 Olly Betts
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

#ifndef guicontrol_h
#define guicontrol_h

#include "wx.h"
#include "survey.h"

class GfxCore;

class GUIControl {
    GfxCore* m_View;
    
    bool m_DraggingLeft;
    bool m_DraggingMiddle;
    bool m_DraggingRight;

    wxPoint m_DragStart;
    wxPoint m_DragRealStart;
    wxPoint m_DragLast;

    enum { drag_NONE, drag_MAIN, drag_COMPASS, drag_ELEV, drag_SCALE } m_LastDrag;

    bool m_ReverseControls;

    void HandleScaleRotate(wxPoint);
    void HandleTilt(wxPoint);
    void HandleTranslate(wxPoint);
    void HandleScale(wxPoint);
    void HandleTiltRotate(wxPoint);
    
    void HandCursor();
    void RestoreCursor();

public:
    GUIControl();
    ~GUIControl();

    void SetView(GfxCore* view);

    bool MouseDown();
    
    void OnDefaults();
    void OnPlan();
    void OnElevation();
    void OnDisplayOverlappingNames();
    void OnShowCrosses();
    void OnShowStationNames();
    void OnShowSurveyLegs();
    void OnShowSurface();
    void OnMoveEast();
    void OnMoveNorth();
    void OnMoveSouth();
    void OnMoveWest();
    void OnStartRotation();
    void OnToggleRotation();
    void OnStopRotation();
    void OnReverseControls();
    void OnSlowDown(bool accel = false);
    void OnSpeedUp(bool accel = false);
    void OnStepOnceAnticlockwise(bool accel = false);
    void OnStepOnceClockwise(bool accel = false);
    void OnHigherViewpoint(bool accel = false);
    void OnLowerViewpoint(bool accel = false);
    void OnShiftDisplayDown(bool accel = false);
    void OnShiftDisplayLeft(bool accel = false);
    void OnShiftDisplayRight(bool accel = false);
    void OnShiftDisplayUp(bool accel = false);
    void OnZoomIn(bool accel = false);
    void OnZoomOut(bool accel = false);
    void OnToggleScalebar();
    void OnToggleDepthbar();
    void OnViewCompass();
    void OnViewClino();
    void OnViewGrid();
    void OnReverseDirectionOfRotation();
    void OnShowEntrances();
    void OnShowFixedPts();
    void OnShowExportedPts();
    void OnCancelDistLine();
    void OnMouseMove(wxMouseEvent& event);
    void OnLButtonDown(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
    void OnMButtonDown(wxMouseEvent& event);
    void OnMButtonUp(wxMouseEvent& event);
    void OnRButtonDown(wxMouseEvent& event);
    void OnRButtonUp(wxMouseEvent& event);

    void OnKeyPress(wxKeyEvent &e);

    void OnDisplayOverlappingNamesUpdate(wxUpdateUIEvent&);
    void OnShowCrossesUpdate(wxUpdateUIEvent&);
    void OnShowStationNamesUpdate(wxUpdateUIEvent&);
    void OnShowSurveyLegsUpdate(wxUpdateUIEvent&);
    void OnShowSurfaceUpdate(wxUpdateUIEvent&);
    void OnMoveEastUpdate(wxUpdateUIEvent&);
    void OnMoveNorthUpdate(wxUpdateUIEvent&);
    void OnMoveSouthUpdate(wxUpdateUIEvent&);
    void OnMoveWestUpdate(wxUpdateUIEvent&);
    void OnStartRotationUpdate(wxUpdateUIEvent&);
    void OnToggleRotationUpdate(wxUpdateUIEvent&);
    void OnStopRotationUpdate(wxUpdateUIEvent&);
    void OnReverseControlsUpdate(wxUpdateUIEvent&);
    void OnReverseDirectionOfRotationUpdate(wxUpdateUIEvent&);
    void OnSlowDownUpdate(wxUpdateUIEvent&);
    void OnSpeedUpUpdate(wxUpdateUIEvent&);
    void OnStepOnceAnticlockwiseUpdate(wxUpdateUIEvent&);
    void OnStepOnceClockwiseUpdate(wxUpdateUIEvent&);
    void OnDefaultsUpdate(wxUpdateUIEvent&);
    void OnElevationUpdate(wxUpdateUIEvent&);
    void OnHigherViewpointUpdate(wxUpdateUIEvent&);
    void OnLowerViewpointUpdate(wxUpdateUIEvent&);
    void OnPlanUpdate(wxUpdateUIEvent&);
    void OnShiftDisplayDownUpdate(wxUpdateUIEvent&);
    void OnShiftDisplayLeftUpdate(wxUpdateUIEvent&);
    void OnShiftDisplayRightUpdate(wxUpdateUIEvent&);
    void OnShiftDisplayUpUpdate(wxUpdateUIEvent&);
    void OnZoomInUpdate(wxUpdateUIEvent&);
    void OnZoomOutUpdate(wxUpdateUIEvent&);
    void OnToggleScalebarUpdate(wxUpdateUIEvent&);
    void OnToggleDepthbarUpdate(wxUpdateUIEvent&);
    void OnViewCompassUpdate(wxUpdateUIEvent&);
    void OnViewClinoUpdate(wxUpdateUIEvent&);
    void OnViewGridUpdate(wxUpdateUIEvent&);
    void OnShowEntrancesUpdate(wxUpdateUIEvent&);
    void OnShowExportedPtsUpdate(wxUpdateUIEvent&);
    void OnShowFixedPtsUpdate(wxUpdateUIEvent&);

    void OnIndicatorsUpdate(wxUpdateUIEvent&);
    void OnCancelDistLineUpdate(wxUpdateUIEvent&);

    void OnViewPerspective();
    void OnViewPerspectiveUpdate(wxUpdateUIEvent& cmd);

    void OnToggleMetric();
    void OnToggleMetricUpdate(wxUpdateUIEvent& cmd);

    void OnToggleDegrees();
    void OnToggleDegreesUpdate(wxUpdateUIEvent& cmd);
   
    void OnToggleTubes();
    void OnToggleTubesUpdate(wxUpdateUIEvent& cmd);
   
    void OnViewFullScreenUpdate(wxUpdateUIEvent&);
    void OnViewFullScreen();
};

#endif