/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998		  */
/* Version: 0.1.0						  */
/* Author: Reginald Stadlbauer					  */
/* E-Mail: reggie@kde.org					  */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs			  */
/* needs c++ library Qt (http://www.troll.no)			  */
/* written for KDE (http://www.kde.org)				  */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)	  */
/* needs OpenParts and Kom (weis@kde.org)			  */
/* License: GNU GPL						  */
/******************************************************************/
/* Module: pixmap object					  */
/******************************************************************/

#include "kppixmapobject.h"

#include <qpainter.h>
#include <qwmatrix.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qcstring.h>

/******************************************************************/
/* Class: KPPixmapObject					  */
/******************************************************************/

/*================ default constructor ===========================*/
KPPixmapObject::KPPixmapObject( KPPixmapCollection *_pixmapCollection )
    : KPObject()
{
    pixmapCollection = _pixmapCollection;
    pixmap = 0L;
    brush = Qt::NoBrush;
    gradient = 0L;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    gColor1 = Qt::red;
    gColor2 = Qt::green;
    unbalanced = FALSE;
    xfactor = 100;
    yfactor = 100;
}

/*================== overloaded constructor ======================*/
KPPixmapObject::KPPixmapObject( KPPixmapCollection *_pixmapCollection, const QString &_filename, QDateTime _lastModified )
    : KPObject()
{
    pixmapCollection = _pixmapCollection;

    if ( !_lastModified.isValid() )
    {
	QFileInfo inf( _filename );
	_lastModified = inf.lastModified();
    }

    ext = orig_size;
    brush = Qt::NoBrush;
    gradient = 0L;
    pixmap = 0L;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    gColor1 = Qt::red;
    gColor2 = Qt::green;
    unbalanced = FALSE;
    xfactor = 100;
    yfactor = 100;

    setPixmap( _filename, _lastModified );
}

/*================================================================*/
KPPixmapObject &KPPixmapObject::operator=( const KPPixmapObject & )
{
    return *this;
}

/*======================= set size ===============================*/
void KPPixmapObject::setSize( int _width, int _height )
{
    KPObject::setSize( _width, _height );
    if ( move ) return;

    if ( pixmap )
	pixmapCollection->removeRef( key );

    key = KPPixmapCollection::Key( KPPixmapDataCollection::Key( key.dataKey.filename, key.dataKey.lastModified ), ext );
    pixmap = pixmapCollection->findPixmap( key );

    if ( ext == orig_size && pixmap )
	ext = pixmap->size();

    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );
}

/*======================= set size ===============================*/
void KPPixmapObject::resizeBy( int _dx, int _dy )
{
    KPObject::resizeBy( _dx, _dy );
    if ( move ) return;

    if ( pixmap )
	pixmapCollection->removeRef( key );

    key = KPPixmapCollection::Key( KPPixmapDataCollection::Key( key.dataKey.filename, key.dataKey.lastModified ), ext );
    pixmap = pixmapCollection->findPixmap( key );

    if ( ext == orig_size && pixmap )
	ext = pixmap->size();

    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );
}

/*================================================================*/
void KPPixmapObject::setPixmap( const QString &_filename, QDateTime _lastModified, const QSize &_size )
{
    if ( !_lastModified.isValid() )
    {
	QFileInfo inf( _filename );
	_lastModified = inf.lastModified();
    }

    if ( pixmap )
	pixmapCollection->removeRef( key );

    key = KPPixmapCollection::Key( KPPixmapDataCollection::Key( _filename, _lastModified ), _size );
    pixmap = pixmapCollection->findPixmap( key );

    if ( ext == orig_size && pixmap )
	ext = pixmap->size();
}

/*================================================================*/
void KPPixmapObject::setFillType( FillType _fillType )
{
    fillType = _fillType;

    if ( fillType == FT_BRUSH && gradient )
    {
	delete gradient;
	gradient = 0;
    }
    if ( fillType == FT_GRADIENT && !gradient )
	gradient = new KPGradient( gColor1, gColor2, gType, getSize(), unbalanced, xfactor, yfactor );
}

/*========================= save =================================*/
void KPPixmapObject::save( ostream& out )
{
    out << indent << "<ORIG x=\"" << orig.x() << "\" y=\"" << orig.y() << "\"/>" << endl;
    out << indent << "<SIZE width=\"" << ext.width() << "\" height=\"" << ext.height() << "\"/>" << endl;
    out << indent << "<SHADOW distance=\"" << shadowDistance << "\" direction=\""
	<< static_cast<int>( shadowDirection ) << "\" red=\"" << shadowColor.red() << "\" green=\"" << shadowColor.green()
	<< "\" blue=\"" << shadowColor.blue() << "\"/>" << endl;
    out << indent << "<EFFECTS effect=\"" << static_cast<int>( effect ) << "\" effect2=\""
	<< static_cast<int>( effect2 ) << "\"/>" << endl;
    out << indent << "<PRESNUM value=\"" << presNum << "\"/>" << endl;
    out << indent << "<ANGLE value=\"" << angle << "\"/>" << endl;
    out << indent << "<KEY " << key << "/>" << endl;
    out << indent << "<FILLTYPE value=\"" << static_cast<int>( fillType ) << "\"/>" << endl;
    out << indent << "<GRADIENT red1=\"" << gColor1.red() << "\" green1=\"" << gColor1.green()
	<< "\" blue1=\"" << gColor1.blue() << "\" red2=\"" << gColor2.red() << "\" green2=\""
	<< gColor2.green() << "\" blue2=\"" << gColor2.blue() << "\" type=\""
	<< static_cast<int>( gType ) << "\" unbalanced=\"" << unbalanced << "\" xfactor=\"" << xfactor 
	<< "\" xfactor=\"" << yfactor << "\"/>" << endl;
    out << indent << "<PEN red=\"" << pen.color().red() << "\" green=\"" << pen.color().green()
	<< "\" blue=\"" << pen.color().blue() << "\" width=\"" << pen.width()
	<< "\" style=\"" << static_cast<int>( pen.style() ) << "\"/>" << endl;
    out << indent << "<BRUSH red=\"" << brush.color().red() << "\" green=\"" << brush.color().green()
	<< "\" blue=\"" << brush.color().blue() << "\" style=\"" << static_cast<int>( brush.style() ) << "\"/>" << endl;
    out << indent << "<DISAPPEAR effect=\"" << static_cast<int>( effect3 ) << "\" doit=\"" << static_cast<int>( disappear )
	<< "\" num=\"" << disappearNum << "\"/>" << endl;
}

/*========================== load ================================*/
void KPPixmapObject::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;

    while ( parser.open( 0L, tag ) )
    {
	KOMLParser::parseTag( tag.c_str(), name, lst );

	// orig
	if ( name == "ORIG" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "x" )
		    orig.setX( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "y" )
		    orig.setY( atoi( ( *it ).m_strValue.c_str() ) );
	    }
	}

	// disappear
	else if ( name == "DISAPPEAR" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "effect" )
		    effect3 = ( Effect3 )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "doit" )
		    disappear = ( bool )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "num" )
		    disappearNum = atoi( ( *it ).m_strValue.c_str() );
	    }
	}

	// size
	else if ( name == "SIZE" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "width" )
		    ext.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "height" )
		    ext.setHeight( atoi( ( *it ).m_strValue.c_str() ) );
	    }
	}

	// shadow
	else if ( name == "SHADOW" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "distance" )
		    shadowDistance = atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "direction" )
		    shadowDirection = ( ShadowDirection )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "red" )
		    shadowColor.setRgb( atoi( ( *it ).m_strValue.c_str() ),
					shadowColor.green(), shadowColor.blue() );
		if ( ( *it ).m_strName == "green" )
		    shadowColor.setRgb( shadowColor.red(), atoi( ( *it ).m_strValue.c_str() ),
					shadowColor.blue() );
		if ( ( *it ).m_strName == "blue" )
		    shadowColor.setRgb( shadowColor.red(), shadowColor.green(),
					atoi( ( *it ).m_strValue.c_str() ) );
	    }
	}

	// effects
	else if ( name == "EFFECTS" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "effect" )
		    effect = ( Effect )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "effect2" )
		    effect2 = ( Effect2 )atoi( ( *it ).m_strValue.c_str() );
	    }
	}

	// angle
	else if ( name == "ANGLE" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    angle = atof( ( *it ).m_strValue.c_str() );
	    }
	}

	// presNum
	else if ( name == "PRESNUM" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    presNum = atoi( ( *it ).m_strValue.c_str() );
	    }
	}


	// key
	else if ( name == "KEY" )
	{
	    int year, month, day, hour, minute, second, msec;

	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "filename" )
		    key.dataKey.filename = ( *it ).m_strValue.c_str();
		else if ( ( *it ).m_strName == "year" )
		    year = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "month" )
		    month = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "day" )
		    day = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "hour" )
		    hour = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "minute" )
		    minute = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "second" )
		    second = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "msec" )
		    msec = atoi( ( *it ).m_strValue.c_str() );
		else if ( ( *it ).m_strName == "width" )
		    key.size.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
		else if ( ( *it ).m_strName == "height" )
		    key.size.setHeight( atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    key.dataKey.lastModified.setDate( QDate( year, month, day ) );
	    key.dataKey.lastModified.setTime( QTime( hour, minute, second, msec ) );
	}

	// pixmap
	else if ( name == "PIXMAP" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();

	    bool openPic = true;
	    QCString _data;
	    QString _fileName;

	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "data" )
		{
		    _data = ( *it ).m_strValue.c_str();
		    if ( _data.isEmpty() )
			openPic = true;
		    else
			openPic = false;
		}
		else if ( ( *it ).m_strName == "filename" )
		{
		    _fileName = ( *it ).m_strValue.c_str();
		    if ( !_fileName.isEmpty() )
		    {
			if ( int _envVarB = _fileName.find( '$' ) >= 0 )
			{
			    int _envVarE = _fileName.find( '/', _envVarB );
			    QString path = getenv( _fileName.mid( _envVarB, _envVarE-_envVarB ).data() );
			    _fileName.replace( _envVarB-1, _envVarE-_envVarB+1, path );
			}
		    }
		}
	    }

	    key.dataKey.filename = _fileName;
	    key.dataKey.lastModified.setDate( pixmapCollection->tmpDate() );
	    key.dataKey.lastModified.setTime( pixmapCollection->tmpTime() );
	    key.size = ext;
	    if ( !openPic )
		pixmapCollection->getPixmapDataCollection().setPixmapOldVersion( key.dataKey, _data );
	    else
		pixmapCollection->getPixmapDataCollection().setPixmapOldVersion( key.dataKey );
	}

	// pen
	else if ( name == "PEN" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red" )
		    pen.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), pen.color().green(), pen.color().blue() ) );
		if ( ( *it ).m_strName == "green" )
		    pen.setColor( QColor( pen.color().red(), atoi( ( *it ).m_strValue.c_str() ), pen.color().blue() ) );
		if ( ( *it ).m_strName == "blue" )
		    pen.setColor( QColor( pen.color().red(), pen.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
		if ( ( *it ).m_strName == "width" )
		    pen.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "style" )
		    pen.setStyle( ( Qt::PenStyle )atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setPen( pen );
	}

	// brush
	else if ( name == "BRUSH" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red" )
		    brush.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), brush.color().green(), brush.color().blue() ) );
		if ( ( *it ).m_strName == "green" )
		    brush.setColor( QColor( brush.color().red(), atoi( ( *it ).m_strValue.c_str() ), brush.color().blue() ) );
		if ( ( *it ).m_strName == "blue" )
		    brush.setColor( QColor( brush.color().red(), brush.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
		if ( ( *it ).m_strName == "style" )
		    brush.setStyle( ( Qt::BrushStyle )atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setBrush( brush );
	}

	// fillType
	else if ( name == "FILLTYPE" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    fillType = static_cast<FillType>( atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setFillType( fillType );
	}

	// gradient
	else if ( name == "GRADIENT" )
	{
	    KOMLParser::parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red1" )
		    gColor1 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor1.green(), gColor1.blue() );
		if ( ( *it ).m_strName == "green1" )
		    gColor1 = QColor( gColor1.red(), atoi( ( *it ).m_strValue.c_str() ), gColor1.blue() );
		if ( ( *it ).m_strName == "blue1" )
		    gColor1 = QColor( gColor1.red(), gColor1.green(), atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "red2" )
		    gColor2 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor2.green(), gColor2.blue() );
		if ( ( *it ).m_strName == "green2" )
		    gColor2 = QColor( gColor2.red(), atoi( ( *it ).m_strValue.c_str() ), gColor2.blue() );
		if ( ( *it ).m_strName == "blue2" )
		    gColor2 = QColor( gColor2.red(), gColor2.green(), atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "type" )
		    gType = static_cast<BCType>( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "unbalanced" )
		    unbalanced = static_cast<bool>( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "xfactor" )
		    xfactor = atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "yfactor" )
		    yfactor = atoi( ( *it ).m_strValue.c_str() );
	    }
	    setGColor1( gColor1 );
	    setGColor2( gColor2 );
	    setGType( gType );
	}

	else
	    cerr << "Unknown tag '" << tag << "' in PIXMAP_OBJECT" << endl;

	if ( !parser.close( tag ) )
	{
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }
}

/*========================= draw =================================*/
void KPPixmapObject::draw( QPainter *_painter, int _diffx, int _diffy )
{
    if ( move )
    {
	KPObject::draw( _painter, _diffx, _diffy );
	return;
    }

    if ( !pixmap ) return;

    int ox = orig.x() - _diffx;
    int oy = orig.y() - _diffy;
    int ow = ext.width();
    int oh = ext.height();
    QRect r;

    _painter->save();
    r = _painter->viewport();

    _painter->setPen( pen );
    _painter->setBrush( brush );

    int penw = pen.width();

    if ( shadowDistance > 0 )
    {
	if ( angle == 0 )
	{
	    int sx = ox;
	    int sy = oy;
	    getShadowCoords( sx, sy, shadowDirection, shadowDistance );

	    _painter->setPen( QPen( shadowColor ) );
	    _painter->setBrush( shadowColor );

	    QSize bs = pixmap->size();

	    _painter->drawRect( sx, sy, bs.width(), bs.height() );
	}
	else
	{
	    r = _painter->viewport();
	    _painter->setViewport( ox, oy, r.width(), r.height() );

	    QRect br = pixmap->rect();
	    int pw = br.width();
	    int ph = br.height();
	    QRect rr = br;
	    int pixYPos = -rr.y();
	    int pixXPos = -rr.x();
	    br.moveTopLeft( QPoint( -br.width() / 2, -br.height() / 2 ) );
	    rr.moveTopLeft( QPoint( -rr.width() / 2, -rr.height() / 2 ) );

	    QWMatrix m, mtx;
	    mtx.rotate( angle );
	    m.translate( pw / 2, ph / 2 );
	    m = mtx * m;

	    _painter->setWorldMatrix( m );

	    _painter->setPen( QPen( shadowColor ) );
	    _painter->setBrush( shadowColor );

	    QSize bs = pixmap->size();
	    int dx = 0, dy = 0;
	    getShadowCoords( dx, dy, shadowDirection, shadowDistance );
	    _painter->drawRect( rr.left() + pixXPos + dx, rr.top() + pixYPos + dy,
				bs.width(), bs.height() );

	    _painter->setViewport( r );
	}
    }
    _painter->restore();
    _painter->save();

    if ( angle == 0 )
    {
	_painter->setPen( Qt::NoPen );
	_painter->setBrush( brush );
	if ( fillType == FT_BRUSH || !gradient )
	    _painter->drawRect( ox + penw, oy + penw, ext.width() - 2 * penw, ext.height() - 2 * penw );
	else
	    _painter->drawPixmap( ox + penw, oy + penw, *gradient->getGradient(), 0, 0, ow - 2 * penw, oh - 2 * penw );

	_painter->drawPixmap( ox, oy, *pixmap );

	_painter->setPen( pen );
	_painter->setBrush( Qt::NoBrush );
	_painter->drawRect( ox + penw, oy + penw, ow - 2 * penw, oh - 2 * penw );
    }
    else
    {
	r = _painter->viewport();
	_painter->setViewport( ox, oy, r.width(), r.height() );

	QRect br = pixmap->rect();
	int pw = br.width();
	int ph = br.height();
	QRect rr = br;
	int pixYPos = -rr.y();
	int pixXPos = -rr.x();
	br.moveTopLeft( QPoint( -br.width() / 2, -br.height() / 2 ) );
	rr.moveTopLeft( QPoint( -rr.width() / 2, -rr.height() / 2 ) );

	QWMatrix m, mtx;
	mtx.rotate( angle );
	m.translate( pw / 2, ph / 2 );
	m = mtx * m;

	_painter->setWorldMatrix( m );

	_painter->setPen( Qt::NoPen );
	_painter->setBrush( brush );

	if ( fillType == FT_BRUSH || !gradient )
	    _painter->drawRect( rr.left() + pixXPos + penw, rr.top() + pixYPos + penw, ext.width() - 2 * penw, ext.height() - 2 * penw );
	else
	    _painter->drawPixmap( rr.left() + pixXPos + penw, rr.top() + pixYPos + penw, *gradient->getGradient(), 0, 0, ow - 2 * penw, oh - 2 * penw );

	_painter->drawPixmap( rr.left() + pixXPos, rr.top() + pixYPos, *pixmap );

	_painter->setPen( pen );
	_painter->setBrush( Qt::NoBrush );
	_painter->drawRect( rr.left() + pixXPos + penw, rr.top() + pixYPos + penw, ow - 2 * penw, oh - 2 * penw );

	_painter->setViewport( r );
    }
    _painter->restore();

    KPObject::draw( _painter, _diffx, _diffy );
}




