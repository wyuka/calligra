
/*
 *  kis_tool_stamp.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf
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

#ifndef __stamptool_h__
#define __stamptool_h__

#include <qpoint.h>

#include "kis_pattern.h"
#include "kis_canvas.h"
#include "kis_tool.h"

class KisPattern;
class KisDoc;

class StampTool : public KisTool {
public:
	StampTool(KisDoc *doc, KisCanvas *canvas, KisPattern *pattern);
	virtual ~StampTool();
 
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);
 
	virtual bool shouldRepaint();
	virtual void setPattern(KisPattern *pattern);
	virtual void optionsDialog();

	void setOpacity(int opacity);
	bool stampMonochrome(QPoint pos);
	bool stampColor(QPoint pos);
	bool stampToCanvas(QPoint pos);

public slots:
	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

protected:

	KisCanvas   *m_pCanvas;

	QPoint      oldp;
	QPoint      mHotSpot;
	int         mHotSpotX;
	int         mHotSpotY;
	QSize       mPatternSize;
	int         patternWidth;
	int         patternHeight;

	QPoint      m_dragStart;
	bool        m_dragging;
	float       m_dragdist;
	int         spacing;
};

#endif //__stamptool_h__
