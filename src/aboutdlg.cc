//
//  aboutdlg.cc
//
//  About box handling for Aven.
//
//  Copyright (C) 2001-2003 Mark R. Shinwell.
//  Copyright (C) 2001,2002,2003,2004 Olly Betts
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aboutdlg.h"
#include "aven.h"
#include "message.h"

BEGIN_EVENT_TABLE(AboutDlg, wxDialog)
END_EVENT_TABLE()

AboutDlg::AboutDlg(wxWindow* parent) :
    wxDialog(parent, 500, wxString::Format(msg(/*About %s*/205), "Aven"))
{
    wxBoxSizer* horiz = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* vert = new wxBoxSizer(wxVERTICAL);

    wxBitmap& bm = wxGetApp().GetAboutBitmap();
    if (bm.Ok()) {
	wxStaticBitmap* bitmap = new wxStaticBitmap(this, 501, bm);
	horiz->Add(bitmap, 0 /* horizontally unstretchable */, wxALL,
		   2 /* border width */);
    }
    horiz->Add(vert, 0, wxALL, 2);

    wxString id = wxString("Aven "VERSION);
    id += '\n';
    id += msg(/*Survey visualisation tool*/209);
    wxStaticText* title = new wxStaticText(this, 502, id);
    wxStaticText* copyright = new wxStaticText(this, 503,
	    wxString::Format(AVEN_COPYRIGHT_MSG"\n"COPYRIGHT_MSG,
			     msg(/*&copy;*/0), msg(/*&copy;*/0)));

    wxString licence_str;
    wxString l(msg(/*This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public Licence as published by the Free Software Foundation; either version 2 of the Licence, or (at your option) any later version.*/219));
    wxClientDC dc(this);
    dc.SetFont(this->GetFont());
    do {
	unsigned int a = 72;
	if (a >= l.length()) {
	    a = l.length();
	} else {
	    while (a > 1 && l[a] != ' ') --a;
	}

	while (a > 1) {
	    wxCoord w, h;
	    dc.GetTextExtent(l.substr(0, a), &w, &h);
	    if (w <= 380) break;
	    do { --a; } while (a > 1 && l[a] != ' ');
	}

	if (!licence_str.empty()) licence_str += '\n';
	licence_str += l.substr(0, a);
	if (a < l.length() && l[a] == ' ') ++a;
	l = l.substr(a);
    } while (!l.empty());

    wxStaticText* licence = new wxStaticText(this, 504, licence_str);
    wxButton* close = new wxButton(this, wxID_OK, wxString(msg(/*Close*/204)));
    close->SetDefault();

    vert->Add(10, 5, 0, wxTOP, 5);
    vert->Add(title, 0, wxLEFT | wxRIGHT, 20);
    vert->Add(10, 5, 0, wxTOP, 5);

    vert->Add(copyright, 0, wxLEFT | wxRIGHT, 20);
    vert->Add(10, 5, 0, wxTOP, 5);

    vert->Add(new wxStaticText(this, 505, msg(/*System Information:*/390)),
	      0, wxLEFT | wxRIGHT, 20);

    wxString info(wxGetOsDescription());
#ifdef AVENGL // FIXME: make this a function in gla-gl.cc
    info += "\nOpenGL ";
    info += (const char*)glGetString(GL_VERSION);
    info += '\n';
    info += (const char*)glGetString(GL_VENDOR);
    info += '\n';
    info += (const char*)glGetString(GL_RENDERER);
    info += '\n';

    const GLubyte* gl_extensions = glGetString(GL_EXTENSIONS);
    if (*gl_extensions) {
	info += gl_extensions;
	info += '\n';
    }  
    GLint red, green, blue;
    glGetIntegerv(GL_RED_BITS, &red);
    glGetIntegerv(GL_GREEN_BITS, &green);
    glGetIntegerv(GL_BLUE_BITS, &blue);
    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    GLint max_viewport[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, max_viewport);
    GLdouble point_size_range[2];
    glGetDoublev(GL_POINT_SIZE_RANGE, point_size_range);
    GLdouble point_size_granularity;
    glGetDoublev(GL_POINT_SIZE_GRANULARITY, &point_size_granularity);
    wxString s;
    s.Printf("R%dG%dB%d\nMax Texture size: %dx%d\nMax Viewport size: %dx%d\nPoint Size %.3f-%.3f (granularity %.3f)",
	     (int)red, (int)green, (int)blue,
	     (int)max_texture_size, (int)max_texture_size,
	     (int)max_viewport[0], (int)max_viewport[1],
	     point_size_range[0], point_size_range[1],
	     point_size_granularity);
    info += s;
#endif
    vert->Add(new wxTextCtrl(this, 506, info, wxDefaultPosition,
			     wxSize(360, 64),
			     wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP),
	      0, wxLEFT | wxRIGHT, 20);

    vert->Add(10, 5, 0, wxTOP, 15);
    
    vert->Add(licence, 0, wxLEFT | wxRIGHT, 20);
    vert->Add(10, 5, 1, wxALIGN_BOTTOM | wxTOP, 5);

    wxBoxSizer* bottom = new wxBoxSizer(wxHORIZONTAL);
    bottom->Add(5, 5, 1);
    bottom->Add(close, 0, wxRIGHT | wxBOTTOM, 15);
    vert->Add(bottom, 0, wxEXPAND | wxLEFT | wxRIGHT, 0);
    vert->SetMinSize(0, bm.GetHeight());

    SetAutoLayout(true);
    SetSizer(horiz);

    horiz->Fit(this);
    horiz->SetSizeHints(this);
}
