/*
 *  selecttool.h - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __selecttoolrectangular_h__
#define __selecttoolrectangular_h__

#include <qpoint.h>
#include "kis_view.h"
#include "kis_tool.h"

class KisDoc;
class KisCanvas;
class KisView;

class RectangularSelectTool : public KisTool
{
public:
    RectangularSelectTool( KisDoc* _doc, KisView* _view, KisCanvas* _canvas );
    ~RectangularSelectTool();

    virtual QString toolName() { return QString( "SelectTool" ); }

    virtual void mousePress( QMouseEvent *_event );
    virtual void mouseMove( QMouseEvent *_event );
    virtual void mouseRelease( QMouseEvent *_event );

    virtual void clearOld();

protected:
    void drawRect( const QPoint&, const QPoint& ); 

protected:
    QPoint     m_dragStart;
    QPoint     m_dragEnd;
    bool       m_dragging;
    bool       m_drawn;   
    bool       m_init;

    KisView   *m_view;  
    KisCanvas *m_canvas;

    QRect      m_selectRect;
};

#endif //__selecttoolrectangular_h__
