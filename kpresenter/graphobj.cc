/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Graphic Object                                         */
/******************************************************************/

#include "graphobj.h"
#include "graphobj.moc"
#include <math.h>
#define RAD_FACTOR 180.0 / M_PI

/******************************************************************/
/* class GraphObj                                                 */
/******************************************************************/

/*==================== constructor ===============================*/
GraphObj::GraphObj(QWidget* parent=0,const char* name=0,ObjType _objType=OT_LINE,QString fName=0)
  :QWidget(parent,name)
{
  QFileInfo fi(fName);

  lineBegin = L_NORMAL;
  lineEnd = L_NORMAL;
  objType = _objType;
  lineType = LT_HORZ;
  rectType = RT_NORM;
  fileName = fName;
  xRnd = 20;
  yRnd = 20;
  hide();
  if (objType == OT_AUTOFORM) 
    {
      atfInterp = new ATFInterpreter(this,fi.baseName());
      atfInterp->load(fileName);
    }
  pix_data = "";
  pix_data_native = "";
}

/*===================== destructor ===============================*/
GraphObj::~GraphObj()
{
}

/*======================= get pic ================================*/
QPicture* GraphObj::getPic(int,int,int,int)
{
  QPainter painter;
  
  if (objType == OT_CLIPART) return &clip;
  if (objType != OT_PICTURE)
    {
      painter.begin(&pic);
      //painter.setClipping(true);
      paintObj(&painter);
      painter.end();
    }
  return &pic;
}

/*======================= load pixmap ============================*/
void GraphObj::loadPixmap()
{
  if (pix_data.isEmpty())
    {
      QPixmap _pix;
      _pix.load(fileName);
      _pix.save("/tmp/kpresenter_tmp.xpm","XPM");
      pix_data = load_pixmap("/tmp/kpresenter_tmp.xpm");
      pix_data_native = load_pixmap_native_format("/tmp/kpresenter_tmp.xpm");
      
      pix.load(fileName);
      origPix.load(fileName);
    }
  else
    {
      pix = string_to_pixmap(pix_data);
      origPix = string_to_pixmap(pix_data);
    }
}

/*======================= load clipart ===========================*/
void GraphObj::loadClipart()
{
  wmf.load(fileName);
  wmf.paint(&clip);
}

/*======================= get Pixmap =============================*/
QPixmap GraphObj::getPix()
{
  QWMatrix m;

  if (width() != pix.width() || height() != pix.height())
    {
      m.scale((float)width()/origPix.width(),(float)height()/origPix.height());
      pix.operator=(QPixmap(origPix));
      pix = pix.xForm(m);
    }

  return pix;
}

/*======================== set filename ==========================*/
void GraphObj::setFileName(QString fn)
{
  if (fileName == fn) return;

  fileName = fn;
  if (objType == OT_AUTOFORM) atfInterp->load(fileName);
  pix_data = "";
  pix_data_native = "";
}

/*========================== save ================================*/
void GraphObj::save(ostream& out)
{
  out << indent << "<LINETYPE value=\"" << lineType << "\"/>" << endl;
  out << indent << "<LINEBEGIN value=\"" << lineBegin << "\"/>" << endl;
  out << indent << "<LINEEND value=\"" << lineEnd << "\"/>" << endl;
  out << indent << "<RECTTYPE value=\"" << rectType << "\"/>" << endl;
  out << indent << "<PEN red=\"" << oPen.color().red() << "\" green=\"" << oPen.color().green()
      << "\" blue=\"" << oPen.color().blue() << "\" width=\"" << oPen.width()
      << "\" style=\"" << oPen.style() << "\"/>" << endl;
  out << indent << "<BRUSH red=\"" << oBrush.color().red() << "\" green=\"" << oBrush.color().green()
      << "\" blue=\"" << oBrush.color().blue() << "\" style=\"" << oBrush.style() << "\"/>" << endl;
  out << indent << "<XRND value=\"" << xRnd << "\"/>" << endl;
  out << indent << "<YRND value=\"" << yRnd << "\"/>" << endl;
  if (objType == OT_PICTURE)
    out << indent << "<PICTURE data=\"" << toPixString(fileName) << "\"/>" << endl;
  if (objType != OT_AUTOFORM)
    out << indent << "<FILENAME value=\"" << fileName << "\"/>" << endl;
  else
    {
      QString afDir = qstrdup(KApplication::kde_datadir());
      afDir += "/kpresenter/autoforms/";
      int len = afDir.length();
      QString str = qstrdup(fileName);
      str = str.remove(0,len);
      out << indent << "<FILENAME value=\"" << str << "\"/>" << endl;
    }
}

/*============================ load ==============================*/
void GraphObj::load(KOMLParser& parser,vector<KOMLAttrib>& lst)
{
  string tag;
  string name;

  setLineBegin(L_NORMAL);
  setLineEnd(L_NORMAL);

  while (parser.open(0L,tag))
    {
      KOMLParser::parseTag(tag.c_str(),name,lst);
      
      // lineType
      if (name == "LINETYPE")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		lineType = (LineType)atoi((*it).m_strValue.c_str());
	    }
	}
      
      // lineBegin
      else if (name == "LINEBEGIN")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		lineBegin = (LineEnd)atoi((*it).m_strValue.c_str());
	    }
	}
 
      // lineEnd
      else if (name == "LINEEND")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		lineEnd = (LineEnd)atoi((*it).m_strValue.c_str());
	    }
	}

      // rectType
      else if (name == "RECTTYPE")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		rectType = (RectType)atoi((*it).m_strValue.c_str());
	    }
	}
      
      // rectType
      else if (name == "PICTURE")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "data")
		{
		  pix_data_native = qstrdup((*it).m_strValue.c_str());
		  pix_data = qstrdup((*it).m_strValue.c_str());
		}
	    }
	}

      // pen
      else if (name == "PEN")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "red")
		oPen.setColor(QColor(atoi((*it).m_strValue.c_str()),oPen.color().green(),oPen.color().blue()));
	      if ((*it).m_strName == "green")
		oPen.setColor(QColor(oPen.color().red(),atoi((*it).m_strValue.c_str()),oPen.color().blue()));
	      if ((*it).m_strName == "blue")
		oPen.setColor(QColor(oPen.color().red(),oPen.color().green(),atoi((*it).m_strValue.c_str())));
	      if ((*it).m_strName == "width")
		oPen.setWidth(atoi((*it).m_strValue.c_str()));
	      if ((*it).m_strName == "style")
		oPen.setStyle((PenStyle)atoi((*it).m_strValue.c_str()));
	    }
	  setObjPen(oPen);
	}
      
      // brush
      else if (name == "BRUSH")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "red")
		oBrush.setColor(QColor(atoi((*it).m_strValue.c_str()),oBrush.color().green(),oBrush.color().blue()));
	      if ((*it).m_strName == "green")
		oBrush.setColor(QColor(oBrush.color().red(),atoi((*it).m_strValue.c_str()),oBrush.color().blue()));
	      if ((*it).m_strName == "blue")
		oBrush.setColor(QColor(oBrush.color().red(),oBrush.color().green(),atoi((*it).m_strValue.c_str())));
	      if ((*it).m_strName == "style")
		oBrush.setStyle((BrushStyle)atoi((*it).m_strValue.c_str()));
	    }
	  setObjBrush(oBrush);
	}
      
      // xRnd
      else if (name == "XRND")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		xRnd = atoi((*it).m_strValue.c_str());
	    }
	}
      
      // yRnd
      else if (name == "YRND")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		yRnd = atoi((*it).m_strValue.c_str());
	    }
	}
      
      // filename
      else if (name == "FILENAME")
	{
	  KOMLParser::parseTag(tag.c_str(),name,lst);
	  vector<KOMLAttrib>::const_iterator it = lst.begin();
	  for(;it != lst.end();it++)
	    {
	      if ((*it).m_strName == "value")
		{
		  fileName = (*it).m_strValue.c_str();
		  if (!fileName.isEmpty())
		    {
		      if (int _envVarB = fileName.find('$') >= 0)
			{
			  int _envVarE = fileName.find('/',_envVarB);
			  QString path = (const char*)getenv((const char*)fileName.mid(_envVarB,_envVarE-_envVarB));
			  fileName.replace(_envVarB-1,_envVarE-_envVarB+1,path);
			}
		    }
		  if (objType == OT_AUTOFORM)
		    {
		      QString afDir = qstrdup(KApplication::kde_datadir());
		      afDir += "/kpresenter/autoforms/";
		      fileName.insert(0,qstrdup(afDir));
		      QFileInfo fi(fileName);
		      atfInterp = new ATFInterpreter(this,fi.baseName());
		      atfInterp->load(fileName);
		    }
		}
	    }
	}
      
      else
	cerr << "Unknown tag '" << tag << "' in GRAPHOBJ" << endl;    
      
      if (!parser.close(tag))
	{
	  cerr << "ERR: Closing Child" << endl;
	  return;
	}
    }
}

/*======================== operator= =============================*/
GraphObj& GraphObj::operator=(GraphObj& go)
{
  objType = go.getObjType();
  setLineType((LineType)go.getLineType());
  setRectType((RectType)go.getRectType());
  setObjPen(go.getObjPen());
  setObjBrush(go.getObjBrush());
  setFileName(go.getFileName());
  setRnds(go.getRndX(),go.getRndY());
  setLineBegin(go.getLineBegin());
  setLineEnd(go.getLineEnd());

  resize(go.size());

  if (objType == OT_PICTURE)
    loadPixmap();
  
  if (objType == OT_CLIPART)
    loadClipart();

  return *this;
}

/*======================= paint event ============================*/
void GraphObj::paintEvent(QPaintEvent* paintEvent)
{
  QPainter painter;
  QRect clipRect = paintEvent->rect();

  painter.begin(this);
  painter.setClipping(true);
  painter.setClipRect(clipRect);
  paintObj(&painter);
  painter.end();
} 

/*====================== paint object ============================*/
void GraphObj::paintObj(QPainter *painter)
{
  int ox = 0,oy = 0,ow = width(),oh = height();
  unsigned int pw = 0,pwOrig = 0,px,py;
  QPicture p;

  switch (objType)
    {
   case OT_LINE: /* line */
      {
	painter->setPen(oPen);
	switch (lineType)
	  {
	  case LT_HORZ:
	    {
	      QSize diff1(0,0),diff2(0,0);
	      int _w = oPen.width();
	      
	      if (lineBegin != L_NORMAL)
		diff1 = getBoundingSize(lineBegin,_w);
	      
	      if (lineEnd != L_NORMAL)
		diff2 = getBoundingSize(lineEnd,_w);

	      if (lineBegin != L_NORMAL)
		drawFigure(lineBegin,painter,QPoint(diff1.width() / 2,oh / 2),oPen.color(),_w,180.0);

	      if (lineEnd != L_NORMAL)
		drawFigure(lineEnd,painter,QPoint(ow - diff2.width() / 2,oh / 2),oPen.color(),_w,0.0);

	      painter->setPen(oPen);
	      painter->drawLine(ox + diff1.width() / 2,oy + oh / 2,ox + ow - diff2.width() / 2,oy + oh / 2);
	    } break;
	  case LT_VERT:
	    {
	      QSize diff1(0,0),diff2(0,0);
	      int _w = oPen.width();
	      
	      if (lineBegin != L_NORMAL)
		diff1 = getBoundingSize(lineBegin,_w);
	      
	      if (lineEnd != L_NORMAL)
		diff2 = getBoundingSize(lineEnd,_w);

	      if (lineBegin != L_NORMAL)
		drawFigure(lineBegin,painter,QPoint(ow / 2,diff1.width() / 2),oPen.color(),_w,270.0);

	      if (lineEnd != L_NORMAL)
		drawFigure(lineEnd,painter,QPoint(ow / 2,oh - diff2.width() / 2),oPen.color(),_w,90.0);

	      painter->setPen(oPen);
	      painter->drawLine(ox + ow / 2,oy + diff1.width() / 2,ox + ow / 2,oy + oh - diff2.width() / 2);
	    } break;
	  case LT_LU_RD:
	    {
	      QSize diff1(0,0),diff2(0,0);
	      int _w = oPen.width();

 	      if (lineBegin != L_NORMAL)
 		diff1 = getBoundingSize(lineBegin,_w);
	      
 	      if (lineEnd != L_NORMAL)
 		diff2 = getBoundingSize(lineEnd,_w);

	      QPoint pnt1(0,0),pnt2(ow,oh);
	      float angle;

	      angle = getAngle(pnt1,pnt2);

 	      if (lineBegin != L_NORMAL)
		{
		  painter->resetXForm();
		  painter->translate(diff1.height() / 2,diff1.width() / 2);
		  drawFigure(lineBegin,painter,QPoint(0,0),oPen.color(),_w,angle);
		}
 	      if (lineEnd != L_NORMAL)
		{
		  painter->resetXForm();
		  painter->translate(ow - diff2.height() / 2,oh - diff2.width() / 2);
		  drawFigure(lineEnd,painter,QPoint(0,0),oPen.color(),_w,angle - 180);
		}

	      painter->resetXForm();
	      painter->setPen(oPen);
	      painter->drawLine(ox + diff1.height() / 2,oy + diff1.width() / 2,ox + ow - diff2.height() / 2,oy + oh - diff2.width() / 2);
	    } break;
	  case LT_LD_RU:
	    {
	      QSize diff1(0,0),diff2(0,0);
	      int _w = oPen.width();

 	      if (lineBegin != L_NORMAL)
 		diff1 = getBoundingSize(lineBegin,_w);
	      
 	      if (lineEnd != L_NORMAL)
 		diff2 = getBoundingSize(lineEnd,_w);

	      QPoint pnt1(0,oh),pnt2(ow,0);
	      float angle;

	      angle = getAngle(pnt1,pnt2);

 	      if (lineBegin != L_NORMAL)
		{
		  painter->resetXForm();
		  painter->translate(diff1.height() / 2,oh - diff1.width() / 2);
		  drawFigure(lineBegin,painter,QPoint(0,0),oPen.color(),_w,angle);
		}
 	      if (lineEnd != L_NORMAL)
		{
		  painter->resetXForm();
		  painter->translate(ow - diff2.height() / 2,diff2.width() / 2);
		  drawFigure(lineEnd,painter,QPoint(0,0),oPen.color(),_w,angle - 180);
		}

	      painter->resetXForm();
	      painter->setPen(oPen);
	      painter->drawLine(ox + diff1.height() / 2,oy + oh - diff1.width() / 2,ox + ow - diff2.height() / 2,oy + diff2.width() / 2);
	    } break;
	  }
      } break;
    case OT_RECT: /* rectangle */
      {
	painter->setPen(oPen);
	pw = oPen.width();
	painter->setBrush(oBrush);
	if (rectType == RT_NORM)
	  painter->drawRect(ox + pw,oy + pw,ow - 2*pw,oh - 2*pw);
	else
	  painter->drawRoundRect(ox + pw,oy + pw,ow - 2*pw,oh - 2*pw,xRnd,yRnd);
      } break;
    case OT_CIRCLE: /* circle */
      {
	painter->setPen(oPen);
	pw = oPen.width();
	painter->setBrush(oBrush);
	painter->drawEllipse(ox + pw,oy + pw,ow - 2*pw,oh - 2*pw);
      } break;
    case OT_AUTOFORM: /* autoforms */
      {
	painter->setPen(oPen);
	pwOrig = oPen.width() + 3;
	painter->setBrush(oBrush);

	if (atfInterp)
	  {
	    QPointArray pntArray = atfInterp->getPointArray(width(),height());
	    QList<ATFInterpreter::AttribList> atrLs = atfInterp->getAttribList();
	    QPointArray pntArray2(pntArray.size());
	    for (unsigned int i=0;i < pntArray.size();i++)
	      {
		px = pntArray.at(i).x();
		py = pntArray.at(i).y();
		if (atrLs.at(i)->pwDiv > 0)
		  {
		    pw = pwOrig / atrLs.at(i)->pwDiv;
		    if (px < (unsigned int)width() / 2) px += pw;
		    if (py < (unsigned int)height() / 2) py += pw;
		    if (px > (unsigned int)width() / 2) px -= pw;
		    if (py > (unsigned int)height() / 2) py -= pw;
		  }
		pntArray2.setPoint(i,px,py);
	      }

	    if (pntArray2.size() > 0)
	      {
		if (pntArray2.at(0) == pntArray2.at(pntArray2.size() - 1))
		  painter->drawPolygon(pntArray2);
		else
		  {
		    QSize diff1(0,0),diff2(0,0);
		    int _w = oPen.width();

		    if (lineBegin != L_NORMAL)
		      diff1 = getBoundingSize(lineBegin,_w);
		    
		    if (lineEnd != L_NORMAL)
		      diff2 = getBoundingSize(lineEnd,_w);

		    if (pntArray.size() > 1)
		      {
			if (lineBegin != L_NORMAL)
			  {
			    QPoint pnt1(pntArray2.at(0)),pnt2(pntArray2.at(1)),pnt3,pnt4(pntArray.at(0));
			    float angle = getAngle(pnt1,pnt2);

			    switch ((int)angle)
			      {
			      case 0:
				{
				  pnt3.setX(pnt4.x() - diff1.width() / 2);
				  pnt3.setY(pnt1.y());
				} break;
			      case 180:
				{
				  pnt3.setX(pnt4.x() + diff1.width() / 2);
				  pnt3.setY(pnt1.y());
				} break;
			      case 90:
				{
				  pnt3.setX(pnt1.x());
				  pnt3.setY(pnt4.y() - diff1.width() / 2);
				} break;
			      case 270:
				{
				  pnt3.setX(pnt1.x());
				  pnt3.setY(pnt4.y() + diff1.width() / 2);
				} break;
			      default: 
				pnt3 = pnt1;
				break;
			      }

			    drawFigure(lineBegin,painter,pnt3,oPen.color(),_w,angle);
			  }
				
			if (lineEnd != L_NORMAL)
			  {
			    QPoint pnt1(pntArray2.at(pntArray2.size() - 1)),pnt2(pntArray2.at(pntArray2.size() - 2));
			    QPoint  pnt3,pnt4(pntArray.at(pntArray.size() - 1));
			    float angle = getAngle(pnt1,pnt2);
			    
			    switch ((int)angle)
			      {
			      case 0:
				{
				  pnt3.setX(pnt4.x() - diff2.width() / 2);
				  pnt3.setY(pnt1.y());
				} break;
			      case 180:
				{
				  pnt3.setX(pnt4.x() + diff2.width() / 2);
				  pnt3.setY(pnt1.y());
				} break;
			      case 90:
				{
				  pnt3.setX(pnt1.x());
				  pnt3.setY(pnt4.y() - diff2.width() / 2);
				} break;
			      case 270:
				{
				  pnt3.setX(pnt1.x());
				  pnt3.setY(pnt4.y() + diff2.width() / 2);
				} break;
			      default: 
				pnt3 = pnt1;
				break;
			      }
			  
			    drawFigure(lineEnd,painter,pnt3,oPen.color(),_w,angle);
			  }
		      }
		    
		    painter->setPen(oPen);
		    painter->drawPolyline(pntArray2);
		  }
	      }

	    if (isVisible())
	      {
		// paint hot-points
	      }
	  }

      } break;
    default: break;
    }
}

/*====================== mouse press =============================*/
void GraphObj::mousePressEvent(QMouseEvent*)
{
  // manage hotpoints
}

/*====================== mouse release ===========================*/
void GraphObj::mouseReleaseEvent(QMouseEvent*)
{
  // manage hotpoints
}

/*====================== mouse move= =============================*/
void GraphObj::mouseMoveEvent(QMouseEvent*)
{
  // manage hotpoints
}

/*==================== convert picture to string =================*/
QString GraphObj::toPixString(QString _filename)
{
  if (true) // if save pic in file
    {
      if (!pix_data.isEmpty())
	{
// 	  QString str = qstrdup(pix_data);
// 	  str.replace(QRegExp("\x22"),"\xfe");
// 	  return str;
	  return pix_data_native;
	}

      QPixmap pix;
      pix.load(_filename);
      
      pix.save("/tmp/kpresenter_tmp.xpm","XPM");
      
      FILE *f = fopen("/tmp/kpresenter_tmp.xpm","r");
      if (f == 0L)
	{
	  warning("Could not open pixmap\n");
	  return QString();
	}
      
      char buffer[2048];
      
      QString str;
      str = "";
      QString str2;
      while(!feof(f))
	{
	  int i = fread(buffer,1,2047,f);
	  if (i > 0)
	    {
	      buffer[i] = 0;
	      str2 = buffer;
	      str2 = str2.replace(QRegExp("\x22"),"\x1");
	      str += str2;
	    }
	}
      
      fclose(f);
    }
  else
    return QString();
}

/*===================== get angle ================================*/
float GraphObj::getAngle(QPoint p1,QPoint p2)
{
  float angle = 0.0;

  if (p1.x() == p2.x()) 
    {
      if (p1.y() < p2.y())
	angle = 270.0;
      else
	angle = 90.0;
    }
  else
    {
      float x1, x2, y1, y2;
      
      if (p1.x() <= p2.x()) 
	{
	  x1 = p1.x(); y1 = p1.y();
	  x2 = p2.x(); y2 = p2.y();
	}
      else 
	{
	  x2 = p1.x(); y2 = p1.y();
	  x1 = p2.x(); y1 = p2.y();
	}

    float m = -(y2 - y1) / (x2 - x1);
    angle = atan(m) * RAD_FACTOR;

    if (p1.x() < p2.x()) 
      angle = 180.0 - angle;
    else 
      angle = -angle;
    }

  return angle;
}


