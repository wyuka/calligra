/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __KARBON_PART_H__
#define __KARBON_PART_H__

#include <qlist.h>
#include <koDocument.h>

#include "vlayer.h"

class KarbonPart : public KoDocument
{
	Q_OBJECT
public:
	KarbonPart( QWidget* parentWidget = 0, const char* widgetName = 0,
		QObject* parent = 0, const char* name = 0, bool singleViewMode = false );
	virtual ~KarbonPart();

	virtual void paintContent( QPainter& painter, const QRect& rect,
	bool transparent = false, double zoomX = 1.0, double zoomY = 1.0 );

	virtual bool initDoc();

	virtual bool loadXML( QIODevice*, const QDomDocument& );
	virtual QDomDocument saveXML();

	const QList<VLayer>& layers() { return m_layers; }
		
protected:
	virtual KoView* createViewInstance( QWidget* parent, const char* name );

private:
	QList<VLayer> m_layers;
};

#endif
