/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qapplication.h>
#include <qpainter.h>
#include "vpainterfactory.h"
#include "vpainter.h"
#include <qpixmap.h>
#include <kdebug.h>

#include "karbon_view.h"
#include "karbon_part.h"
#include "vcanvas.h"

VCanvas::VCanvas( KarbonView* view, KarbonPart* part )
	: QScrollView( view, "canvas", WNorthWestGravity | WResizeNoErase /* |
	  WRepaintNoErase */ ), m_part( part ), m_view( view ), m_zoomFactor( 1.0 )
{
	viewport()->setFocusPolicy( QWidget::StrongFocus );

	viewport()->setMouseTracking( true );
	setMouseTracking( true );

	viewport()->setBackgroundColor( Qt::white );
	viewport()->setBackgroundMode( QWidget::NoBackground );

// TODO: remove this line
resizeContents( 800, 600 );

	setFocus();
}

void
VCanvas::drawContents( QPainter* painter, int clipx, int clipy,
	int clipw, int cliph  )
{
	drawDocument( painter, QRect( clipx, clipy, clipw, cliph ) );
}

void
VCanvas::drawDocument( QPainter* painter, const QRect& rect )
{
	VPainter *p = VPainterFactory::painter();
	p->begin();
	//VPainter *p = VPainterFactory::painter( this, visibleWidth(), visibleHeight() );
	//erase( rect );

	QPtrListIterator<VLayer> i = m_part->layers();
	for ( ; i.current(); ++i )
		if ( i.current()->visible() )
			i.current()->draw( p, rect, m_zoomFactor );

	p->end();
}

void
VCanvas::repaintAll( bool erase )
{
	viewport()->repaint( erase );
}

void
VCanvas::resizeEvent( QResizeEvent* event )
{
	QScrollView::resizeEvent( event );
}

#include <vcanvas.moc>
