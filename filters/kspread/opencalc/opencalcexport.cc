/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2000 Norbert Andres <nandres@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <float.h>
#include <math.h>

#include <opencalcexport.h>

#include <qdatetime.h>
#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <koDocumentInfo.h>
#include <koFilterChain.h>
#include <koGlobal.h>

#include <kspread_aboutdata.h>
#include <kspread_cell.h>
#include <kspread_doc.h>
#include <kspread_format.h>
#include <kspread_map.h>
#include <kspread_sheet.h>

typedef KGenericFactory<OpenCalcExport, KoFilter> OpenCalcExportFactory;
K_EXPORT_COMPONENT_FACTORY( libopencalcexport, 
                            OpenCalcExportFactory( "opencalcfilter" ) );

#define STOPEXPORT \
  do \
  { \
    delete store; \
    return false; \
  } while(0)

OpenCalcExport::OpenCalcExport( KoFilter *, const char *, const QStringList & ) 
  : KoFilter() 
{
}

KoFilter::ConversionStatus OpenCalcExport::convert( const QCString & from, 
                                                    const QCString & to )
{
  /* later...
     KSpreadLeader  * leader = new KSpreadLeader( m_chain );
     OpenCalcWorker * worker = new OpenCalcWorker();
     leader->setWorker( worker );
     
     KoFilter::ConversionStatus status = leader->convert();
     
     delete worker;
     delete leader;
     
     return status;
  */

  KoDocument * document = m_chain->inputDocument();

  if ( !document )
    return KoFilter::StupidError;

  if ( strcmp(document->className(), "KSpreadDoc") != 0)
  {
    kdWarning(30501) << "document isn't a KSpreadDoc but a " 
                     << document->className() << endl;
    return KoFilter::NotImplemented;
  }

  if ( ( to != "application/x-opencalc") || (from != "application/x-kspread" ) )
  {
    kdWarning(30501) << "Invalid mimetypes " << to << " " << from << endl;
    return KoFilter::NotImplemented;
  }

  KSpreadDoc const * const ksdoc = static_cast<const KSpreadDoc *>(document);

  if ( ksdoc->mimeType() != "application/x-kspread" )
  {
    kdWarning(30501) << "Invalid document mimetype " << ksdoc->mimeType() << endl;
    return KoFilter::NotImplemented;
  }

  if ( !writeFile( ksdoc ) )
    return KoFilter::CreationError;

  emit sigProgress( 100 );
  
  return KoFilter::OK;
}

bool OpenCalcExport::writeFile( KSpreadDoc const * const ksdoc )
{
  KoStore * store = KoStore::createStore( m_chain->outputFile(), KoStore::Write, "", KoStore::Zip );

  if ( !store )
    return false;

  uint filesWritten = 0;

  if ( !exportContent( store, ksdoc ) )
    STOPEXPORT;
  else
    filesWritten |= contentXML;    

  // TODO: pass sheet number and cell number
  if ( !exportDocInfo( store, ksdoc ) )
    STOPEXPORT;
  else
    filesWritten |= metaXML;

  if ( !exportStyles( store, ksdoc ) )
    STOPEXPORT;
  else
    filesWritten |= stylesXML;

  if ( !writeMetaFile( store, filesWritten ) )
    STOPEXPORT;

  // writes zip file to disc
  delete store;
  store = 0;

  return true;
}

bool OpenCalcExport::exportDocInfo( KoStore * store, KSpreadDoc const * const ksdoc )
{
  if ( !store->open( "meta.xml" ) )
    return false;

  KoDocumentInfo       * docInfo    = ksdoc->documentInfo();
  KoDocumentInfoAbout  * aboutPage  = static_cast<KoDocumentInfoAbout *>( docInfo->page( "about" ) );
  KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor*>( docInfo->page( "author" ) );

  QDomDocument meta;
  meta.appendChild( meta.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

  QDomElement content = meta.createElement( "office:document-meta" );
  content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office");
  content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  content.setAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );
  content.setAttribute( "xmlns:meta", "http://openoffice.org/2000/meta" );
  content.setAttribute( "office:version", "1.0" );

  QDomNode officeMeta = meta.createElement( "office:meta" );

  QDomElement data = meta.createElement( "meta:generator" );
  QString app( "KSpread " );
  app += version;
  data.appendChild( meta.createTextNode( app ) );
  officeMeta.appendChild( data );

  data = meta.createElement( "meta:initial-creator" );
  data.appendChild( meta.createTextNode( authorPage->fullName() ) );
  officeMeta.appendChild( data );
  
  data = meta.createElement( "meta:creator" );
  data.appendChild( meta.createTextNode( authorPage->fullName() ) );
  officeMeta.appendChild( data );

  data = meta.createElement( "meta:user-defined" );
  data.setAttribute( "meta:name", "Info 1" );
  data.appendChild( meta.createTextNode( aboutPage->title() ) );
  officeMeta.appendChild( data );

  data = meta.createElement( "meta:user-defined" );
  data.setAttribute( "meta:name", "Info 2" );
  data.appendChild( meta.createTextNode( aboutPage->abstract() ) );
  officeMeta.appendChild( data );

  /* TODO:
    <meta:creation-date>2003-01-08T23:57:31</meta:creation-date>
    <dc:date>2003-01-08T23:58:05</dc:date>
    <dc:language>en-US</dc:language>
    <meta:editing-cycles>2</meta:editing-cycles>
    <meta:editing-duration>PT38S</meta:editing-duration>
    <meta:user-defined meta:name="Info 3"/>
    <meta:user-defined meta:name="Info 4"/>
  */

  data = meta.createElement( "meta:document-statistic" );
  data.setAttribute( "meta:table-count", QString::number( ksdoc->map()->count() ) );
  //  TODO: data.setAttribute( "meta:cell-count",  );
  officeMeta.appendChild( data );

  content.appendChild( officeMeta );
  meta.appendChild( content );

  QCString doc( meta.toCString() );
  kdDebug() << "Meta: " << doc << endl;

  store->write( doc, doc.length() );

  if ( !store->close() )
    return false;

  return true;
}

bool OpenCalcExport::exportContent( KoStore * store, KSpreadDoc const * const ksdoc )
{
  if ( !store->open( "content.xml" ) )
    return false;

  createDefaultStyles();

  QDomDocument doc;
  doc.appendChild( doc.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

  QDomElement content = doc.createElement( "office:document-content" );
  content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office");
  content.setAttribute( "xmlns:style", "http://openoffice.org/2000/style" );
  content.setAttribute( "xmlns:text", "http://openoffice.org/2000/text" );
  content.setAttribute( "xmlns:table", "http://openoffice.org/2000/table" );
  content.setAttribute( "xmlns:draw", "http://openoffice.org/2000/drawing" );
  content.setAttribute( "xmlns:fo", "http://www.w3.org/1999/XSL/Format" );
  content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  content.setAttribute( "xmlns:number", "http://openoffice.org/2000/datastyle" );
  content.setAttribute( "xmlns:svg", "http://www.w3.org/2000/svg" );
  content.setAttribute( "xmlns:chart", "http://openoffice.org/2000/chart" );
  content.setAttribute( "xmlns:dr3d", "http://openoffice.org/2000/dr3d" );
  content.setAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
  content.setAttribute( "xmlns:form", "http://openoffice.org/2000/form" );
  content.setAttribute( "xmlns:script", "http://openoffice.org/2000/script" );
  content.setAttribute( "office:class", "spreadsheet" );
  content.setAttribute( "office:version", "1.0" );

  QDomElement data = doc.createElement( "office:script" );
  content.appendChild( data );

  if ( !exportBody( doc, content, ksdoc ) )
    return false;

  doc.appendChild( content );

  QCString f( doc.toCString() );
  kdDebug() << "Content: " << (char const * ) f << endl;

  store->write( f, f.length() );

  if ( !store->close() )
    return false;

  return true;
}

bool OpenCalcExport::exportBody( QDomDocument & doc, QDomElement & content, KSpreadDoc const * const ksdoc )
{
  QDomElement fontDecls  = doc.createElement( "office:font-decls" );
  QDomElement autoStyles = doc.createElement( "office:automatic-styles" );
  QDomElement body       = doc.createElement( "office:body" );

  QPtrListIterator<KSpreadSheet> it( ksdoc->map()->tableList() );

  for( it.toFirst(); it.current(); ++it )
  {
    SheetStyle ts;
    int maxCols         = 0;
    int maxRows         = 0;
    KSpreadSheet * sheet = it.current();

    ts.visible = !sheet->isHidden();

    QDomElement tabElem = doc.createElement( "table:table" );
    tabElem.setAttribute( "table:style-name", m_styles.sheetStyle( ts ) );
    tabElem.setAttribute( "table:name", sheet->tableName() );

    maxRowCols( sheet, maxCols, maxRows );

    exportSheet( doc, tabElem, sheet, maxCols, maxRows );
    
    body.appendChild( tabElem );
  }

  m_styles.writeStyles( doc, autoStyles );
  m_styles.writeFontDecl( doc, fontDecls );

  content.appendChild( fontDecls );
  content.appendChild( autoStyles );
  content.appendChild( body );

  return true;
}

void OpenCalcExport::exportSheet( QDomDocument & doc, QDomElement & tabElem, 
                                  KSpreadSheet const * const sheet, int maxCols, int maxRows )
{
  kdDebug() << "exportSheet: " << sheet->tableName() << endl;
  int i = 1;

  // TODO: handle empty sheets
  for ( ; i <= maxCols; ++i )
  {
    ColumnFormat const * const column = sheet->columnFormat( i );
    ColumnStyle cs;
    cs.breakB = Style::automatic;
    cs.size   = column->mmWidth() / 10;

    QDomElement colElem = doc.createElement( "table:table-column" );
    colElem.setAttribute( "table:style-name", m_styles.columnStyle( cs ) );
    colElem.setAttribute( "table:default-cell-style-name", "Default" );
    // TODO: colElem.setAttribute( "table:number-columns-repeated", x );

    tabElem.appendChild( colElem );
  }

  for ( i = 1; i <= maxRows; ++i )
  {
    RowFormat const * const row = sheet->rowFormat( i );
    RowStyle rs;
    rs.breakB = Style::automatic;
    rs.size   = row->mmHeight() / 10;

    QDomElement rowElem = doc.createElement( "table:table-row" );
    rowElem.setAttribute( "table:style-name", m_styles.rowStyle( rs ) );

    exportCells( doc, rowElem, sheet, i, maxCols );

    tabElem.appendChild( rowElem );
  }
}

void OpenCalcExport::exportCells( QDomDocument & doc, QDomElement & rowElem, 
                                  KSpreadSheet const * const sheet, int row, int maxCols )
{
  int i = 1;
  for ( ; i <= maxCols; ++i )
  {
    KSpreadCell const * const cell = sheet->cellAt( i, row );
    QDomElement cellElem = doc.createElement( "table:table-cell" );

    KSpreadValue const value( cell->value() );

    QFont font = cell->font();
    m_styles.addFont( font );
    CellStyle c;
    c.font    = font;

    if ( cell->hasProperty( KSpreadFormat::PTextPen ) )
      c.color   = cell->textColor( cell->column(), cell->row() );
    if ( cell->hasProperty( KSpreadFormat::PBackgroundColor ) )
      c.bgColor = cell->bgColor( cell->column(), cell->row() );

    cellElem.setAttribute( "table:style-name", m_styles.cellStyle( c ) );

    if ( value.isBoolean() )
    {
      kdDebug() << "Type: Boolean" << endl;
      cellElem.setAttribute( "table:value-type", "boolean" );
      cellElem.setAttribute( "table:boolean-value", ( value.asBoolean() ? "true" : "false" ) );
    }
    else if ( value.isNumber() )
    {
      kdDebug() << "Type: Number" << endl;
      KSpreadFormat::FormatType type = cell->getFormatType( i, row );

      if ( ( type == KSpreadFormat::Percentage ) 
           || ( type == KSpreadFormat::CustomPercentage ) )
        cellElem.setAttribute( "table:value-type", "percentage" );
      else
        cellElem.setAttribute( "table:value-type", "float" );

      cellElem.setAttribute( "table:value", QString::number( value.asFloat() ) );
    }      
    else
    {
      kdDebug() << "Type: " << value.type() << endl;
    }

    if ( cell->isFormula() )
    {
      kdDebug() << "Formula found" << endl;

      QString formula( convertFormula( cell->text() ) );
      cellElem.setAttribute( "table:formula", formula );
    }

    if ( !cell->isEmpty() )
    {
      QDomElement textElem = doc.createElement( "text:p" );
      textElem.appendChild( doc.createTextNode( cell->strOutText() ) );

      cellElem.appendChild( textElem );
    }
    kdDebug() << "Cell StrOut: " << cell->strOutText() << endl;

    rowElem.appendChild( cellElem );
  }
}

void OpenCalcExport::maxRowCols( KSpreadSheet const * const sheet, 
                                 int & maxCols, int & maxRows )
{
  KSpreadCell const * cell = sheet->firstCell();

  while ( cell )
  {
    if ( cell->column() > maxCols )
      maxCols = cell->column();

    if ( cell->row() > maxRows )
      maxRows = cell->row();

    cell = cell->nextCell();
  }
}

bool OpenCalcExport::exportStyles( KoStore * store, KSpreadDoc const * const ksdoc )
{
  if ( !store->open( "styles.xml" ) )
    return false;

  QDomDocument doc;
  doc.appendChild( doc.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

  QDomElement content = doc.createElement( "office:document-styles" );
  content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office" );
  content.setAttribute( "xmlns:style", "http://openoffice.org/2000/style" );
  content.setAttribute( "xmlns:text", "http://openoffice.org/2000/text" );
  content.setAttribute( "xmlns:table", "http://openoffice.org/2000/table" );
  content.setAttribute( "xmlns:draw", "http://openoffice.org/2000/drawing" );
  content.setAttribute( "xmlns:fo", "http://www.w3.org/1999/XSL/Format" );
  content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  content.setAttribute( "xmlns:number", "http://openoffice.org/2000/datastyle" );
  content.setAttribute( "xmlns:svg", "http://www.w3.org/2000/svg" );
  content.setAttribute( "xmlns:chart", "http://openoffice.org/2000/chart" );
  content.setAttribute( "xmlns:dr3d", "http://openoffice.org/2000/dr3d" );
  content.setAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
  content.setAttribute( "xmlns:form", "http://openoffice.org/2000/form" );
  content.setAttribute( "xmlns:script", "http://openoffice.org/2000/script" );
  content.setAttribute( "office:version", "1.0" );

  // order important here!
  QDomElement officeStyles = doc.createElement( "office:styles" );
  exportDefaultCellStyle( doc, officeStyles );

  QDomElement fontDecls = doc.createElement( "office:font-decls" );  
  m_styles.writeFontDecl( doc, fontDecls );

  // TODO: needs in new number/date/time parser...
  //  exportDefaultNumberStyles( doc, officeStyles );
  
  QDomElement defaultStyle = doc.createElement( "style:style" );
  defaultStyle.setAttribute( "style:name", "Default" );
  defaultStyle.setAttribute( "style:family", "table-cell" );
  officeStyles.appendChild( defaultStyle );

  QDomElement autoStyles = doc.createElement( "office:automatic-styles" );
  exportPageAutoStyles( doc, autoStyles, ksdoc );
  
  QDomElement masterStyles = doc.createElement( "office:master-styles" );
  exportMasterStyles( doc, masterStyles, ksdoc );

  content.appendChild( fontDecls );
  content.appendChild( officeStyles );
  content.appendChild( autoStyles );
  content.appendChild( masterStyles );

  doc.appendChild( content );

  QCString f( doc.toCString() );
  kdDebug() << "Content: " << (char const * ) f << endl;

  store->write( f, f.length() );

  if ( !store->close() )
    return false;

  return true;
}

void OpenCalcExport::exportDefaultCellStyle( QDomDocument & doc, QDomElement & officeStyles )
{
  QDomElement defStyle = doc.createElement( "style:default-style" );
  defStyle.setAttribute( "style:family", "table-cell" );

  KoDocument * document = m_chain->inputDocument();
  KSpreadDoc * ksdoc    = static_cast<KSpreadDoc *>(document);

  KSpreadFormat * format = new KSpreadFormat( 0 );
  KLocale const * const locale = ksdoc->locale();
  QString language;
  QString country;
  QString charSet;

  QString l( locale->language() );
  KLocale::splitLocale( l, language, country, charSet );
  QFont font( format->font() );
  m_styles.addFont( font, true );

  QDomElement style = doc.createElement( "style:properties" );
  style.setAttribute( "style:font-name", font.family() );
  style.setAttribute( "fo:font-size", QString( "%1pt" ).arg( font.pointSize() ) );
  style.setAttribute( "style:decimal-places", QString::number( locale->fracDigits() ) );
  style.setAttribute( "fo:language", language );
  style.setAttribute( "fo:country", country );
  style.setAttribute( "style:font-name-asian", "HG Mincho Light J" );
  style.setAttribute( "style:language-asian", "none" );
  style.setAttribute( "style:country-asian", "none" );
  style.setAttribute( "style:font-name-complex", "Arial Unicode MS" );
  style.setAttribute( "style:language-complex", "none" );
  style.setAttribute( "style:country-complex", "none" );
  style.setAttribute( "style:tab-stop-distance", "1.25cm" );
                                   
  defStyle.appendChild( style );
  officeStyles.appendChild( defStyle );
  delete format;
}

void OpenCalcExport::createDefaultStyles()
{
  // TODO: default number styles, currency styles,...
}

void OpenCalcExport::exportPageAutoStyles( QDomDocument & doc, QDomElement & autoStyles,
                                           KSpreadDoc const * const ksdoc )
{
  QPtrListIterator<KSpreadSheet> it( ksdoc->map()->tableList() );
  KSpreadSheet const * const sheet = it.toFirst();

  float width  = 20.999;
  float height = 29.699;

  if ( sheet )
  {
    width  = sheet->paperWidth() / 10;
    height = sheet->paperHeight() / 10;
  }

  QString sWidth  = QString( "%1cm" ).arg( width  );
  QString sHeight = QString( "%1cm" ).arg( height );
  
  QDomElement pageMaster = doc.createElement( "style:page-master" );
  pageMaster.setAttribute( "style:name", "pm1" );

  QDomElement properties = doc.createElement( "style:properties" );
  properties.setAttribute( "fo:page-width",  sWidth  );
  properties.setAttribute( "fo:page-height", sHeight ); 
  properties.setAttribute( "fo:border", "0.002cm solid #000000" );
  properties.setAttribute( "fo:padding", "0cm" );
  properties.setAttribute( "fo:background-color", "transparent" );

  pageMaster.appendChild( properties );

  QDomElement header = doc.createElement( "style:header-style" );
  properties = doc.createElement( "style:properties" );
  properties.setAttribute( "fo:min-height", "0.75cm" );
  properties.setAttribute( "fo:margin-left", "0cm" );
  properties.setAttribute( "fo:margin-right", "0cm" );
  properties.setAttribute( "fo:margin-bottom", "0.25cm" );

  header.appendChild( properties );

  QDomElement footer = doc.createElement( "style:header-style" );
  properties = doc.createElement( "style:properties" );
  properties.setAttribute( "fo:min-height", "0.75cm" );
  properties.setAttribute( "fo:margin-left", "0cm" );
  properties.setAttribute( "fo:margin-right", "0cm" );
  properties.setAttribute( "fo:margin-bottom", "0.25cm" );

  footer.appendChild( properties );

  pageMaster.appendChild( header );
  pageMaster.appendChild( footer );

  autoStyles.appendChild( pageMaster );
}

void OpenCalcExport::exportMasterStyles( QDomDocument & doc, QDomElement & masterStyles,
                                         KSpreadDoc const * const ksdoc )
{
  QDomElement masterPage = doc.createElement( "style:master-page" );
  masterPage.setAttribute( "style:name", "Default" );
  masterPage.setAttribute( "style:page-master-name", "pm1" );

  QPtrListIterator<KSpreadSheet> it( ksdoc->map()->tableList() );
  KSpreadSheet const * const sheet = it.toFirst();

  QString headerLeft;
  QString headerCenter;
  QString headerRight;
  QString footerLeft;
  QString footerCenter;
  QString footerRight;
  
  if ( sheet )
  {
    headerLeft   = sheet->headLeft();
    headerCenter = sheet->headMid();
    headerRight  = sheet->headRight();
    footerLeft   = sheet->footLeft();
    footerCenter = sheet->footMid();
    footerRight  = sheet->footRight();
  }

  if ( ( headerLeft.length() > 0 ) || ( headerCenter.length() > 0 )
       || ( headerRight.length() > 0 ) )
  {
    QDomElement header = doc.createElement( "style:header" );
    QDomElement left   = doc.createElement( "style:region-left" );
    QDomElement text   = doc.createElement( "text:p" );
    convertPart( headerLeft, doc, text, ksdoc );
    left.appendChild( text );

    QDomElement center = doc.createElement( "style:region-center" );
    QDomElement text1  = doc.createElement( "text:p" );
    convertPart( headerCenter, doc, text1, ksdoc );
    center.appendChild( text1 );

    QDomElement right = doc.createElement( "style:region-right" );
    QDomElement text2 = doc.createElement( "text:p" );
    convertPart( headerRight, doc, text2, ksdoc );
    right.appendChild( text2 );

    header.appendChild( left   );
    header.appendChild( center );
    header.appendChild( right  );

    masterPage.appendChild( header );
  }
  else
  {
    QDomElement header = doc.createElement( "style:header" );
    QDomElement text   = doc.createElement( "text:p" );
    QDomElement name   = doc.createElement( "text:sheet-name" );
    name.appendChild( doc.createTextNode( "???" ) );
    text.appendChild( name );
    header.appendChild( text );

    masterPage.appendChild( header );
  }

  if ( ( footerLeft.length() > 0 ) || ( footerCenter.length() > 0 )
       || ( footerRight.length() > 0 ) )
  {
    QDomElement footer = doc.createElement( "style:footer" );
    QDomElement left   = doc.createElement( "style:region-left" );
    QDomElement text   = doc.createElement( "text:p" );
    convertPart( footerLeft, doc, text, ksdoc );
    left.appendChild( text );

    QDomElement center = doc.createElement( "style:region-center" );
    QDomElement text1  = doc.createElement( "text:p" );
    convertPart( footerCenter, doc, text1, ksdoc );
    center.appendChild( text1 );

    QDomElement right = doc.createElement( "style:region-right" );
    QDomElement text2  = doc.createElement( "text:p" );
    convertPart( footerRight, doc, text2, ksdoc );
    right.appendChild( text2 );

    footer.appendChild( left   );
    footer.appendChild( center );
    footer.appendChild( right  );

    masterPage.appendChild( footer );
  }
  else
  {
    QDomElement footer = doc.createElement( "style:footer" );
    QDomElement text   = doc.createElement( "text:p" );
    text.appendChild( doc.createTextNode( i18n( "Page " ) ) );
    QDomElement number = doc.createElement( "text:page-number" );
    number.appendChild( doc.createTextNode( "1" ) );
    text.appendChild( number );
    footer.appendChild( text );

    masterPage.appendChild( footer );
  }

  masterStyles.appendChild( masterPage );       
}

void OpenCalcExport::addText( QString const & text, QDomDocument & doc, 
                              QDomElement & parent )
{
  if (text.length() > 0 )
    parent.appendChild( doc.createTextNode( text ) );
}

void OpenCalcExport::convertPart( QString const & part, QDomDocument & doc, 
                                  QDomElement & parent, KSpreadDoc const * const ksdoc )
{
  QString text;
  QString var;

  bool inVar = false;
  uint i = 0;
  uint l = part.length();
  while ( i < l )
  {
    if ( inVar || part[i] == '<' )
    {
      inVar = true;
      var += part[i];
      if ( part[i] == '>' )
      {
        inVar = false;
        if ( var == "<page>" )
        {
          addText( text, doc, parent );
          
          QDomElement page = doc.createElement( "text:page-number" );
          page.appendChild( doc.createTextNode( "1" ) );
          parent.appendChild( page );
        }
        else if ( var == "<pages>" )
        {
          addText( text, doc, parent );
          
          QDomElement page = doc.createElement( "text:page-count" );
          page.appendChild( doc.createTextNode( "99" ) );
          parent.appendChild( page );          
        }
        else if ( var == "<date>" )
        {
          addText( text, doc, parent );

          QDomElement t = doc.createElement( "text:date" );
          t.setAttribute( "text:date-value", "0-00-00" );
          // todo: "style:data-style-name", "N2" 
          t.appendChild( doc.createTextNode( QDate::currentDate().toString() ) );
          parent.appendChild( t );
        }
        else if ( var == "<time>" )
        {
          addText( text, doc, parent );

          QDomElement t = doc.createElement( "text:time" );
          t.appendChild( doc.createTextNode( QTime::currentTime().toString() ) );
          parent.appendChild( t );
        }
        else if ( var == "<file>" ) // filepath + name
        {
          addText( text, doc, parent );

          QDomElement t = doc.createElement( "text:file-name" );
          t.setAttribute( "text:display", "full" );
          t.appendChild( doc.createTextNode( "???" ) );
          parent.appendChild( t );
        }
        else if ( var == "<name>" ) // filename
        {
          addText( text, doc, parent );

          QDomElement t = doc.createElement( "text:title" );
          t.appendChild( doc.createTextNode( "???" ) );
          parent.appendChild( t );
        }
        else if ( var == "<author>" )
        {
          KoDocumentInfo       * docInfo    = ksdoc->documentInfo();
          KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor*>( docInfo->page( "author" ) );

          text += authorPage->fullName();

          addText( text, doc, parent );
        }
        else if ( var == "<email>" )
        {
          KoDocumentInfo       * docInfo    = ksdoc->documentInfo();
          KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor*>( docInfo->page( "author" ) );

          text += authorPage->email();

          addText( text, doc, parent );
        }
        else if ( var == "<org>" )
        {
          KoDocumentInfo       * docInfo    = ksdoc->documentInfo();
          KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor*>( docInfo->page( "author" ) );

          text += authorPage->company();

          addText( text, doc, parent );
        }
        else if ( var == "<sheet>" )
        {
          addText( text, doc, parent );

          QDomElement s = doc.createElement( "text:sheet-name" );
          s.appendChild( doc.createTextNode( "???" ) );
          parent.appendChild( s );
        }
        else
        {
          // no known variable:
          text += var;
          addText( text, doc, parent );          
        }

        text = "";
        var  = "";
      }
    }
    else
    {
      text += part[i];
    }
    ++i;
  }
}

void insertBracket( QString & s )
{
  QChar c;
  int i = (int) s.length() - 1;
  while ( i >= 0 )
  {
    c = s[i];
    if ( !(c.isLetterOrNumber() || c == ' ' || c == '.' 
           || c == '_') )
    {
      s.insert( i + 1, '[' );
      return;
    }
    --i;
  }
}

QString OpenCalcExport::convertFormula( QString const & formula ) const
{
  QString s;
  QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)");
  int n = exp.search( formula, 0 );
  kdDebug() << "Exp: " << formula << ", n: " << n << ", Length: " << formula.length() 
            << ", Matched length: " << exp.matchedLength() << endl;

  bool inQuote1 = false;  
  bool inQuote2 = false;  
  int i = 0;
  int l = (int) formula.length();
  if ( l <= 0 )
    return formula;
  while ( i < l )
  {
    if ( ( n != -1 ) && ( n < i ) )
    {
      n = exp.search( formula, i );
      kdDebug() << "Exp: " << formula.right( l - i ) << ", n: " << n << endl;
    }
    if ( formula[i] == '"' )
    {
      inQuote1 = !inQuote1;
      s += formula[i];      
      ++i;
      continue;
    }
    if ( formula[i] == '\'' )
    {
      s += formula[i];
      inQuote2 = !inQuote2;
      ++i;
      continue;
    }
    if ( inQuote1 || inQuote2 )
    {
      s += formula[i];
      ++i;
      continue;
    }
    if ( ( formula[i] == '=' ) && ( formula[i + 1] == '=' ) )
    {
      s += '=';
      ++i;++i;
      continue;
    }
    if ( formula[i] == '!' )
    {
      insertBracket( s );
      s += '.';
      ++i;
      continue;
    }
    if ( n == i )
    {
      int ml = exp.matchedLength();
      if ( formula[ i + ml ] == '!' )
      {
        kdDebug() << "No cell ref but sheet name" << endl;
        s += formula[i];
        ++i;
        continue;
      }
      if ( ( i > 0 ) && ( formula[i - 1] != '!' ) )
        s += "[.";
      for ( int j = 0; j < ml; ++j )
      {
        s += formula[i];
        ++i;
      }
      s += ']';
      continue;
    }

    s += formula[i];
    ++i;
  }

  return s;
}

bool OpenCalcExport::writeMetaFile( KoStore * store, uint filesWritten )
{
  if ( !store->open( "manifest.xml" ) )
    return false;

  QDomImplementation impl;
  QDomDocumentType type( impl.createDocumentType( "manifest:manifest", "-//OpenOffice.org//DTD Manifest 1.0//EN", "Manifest.dtd" ) );

  QDomDocument meta( type );
  meta.appendChild( meta.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );
  
  QDomElement content = meta.createElement( "manifest:manifest" );
  content.setAttribute( "xmlns:manifest", "http://openoffice.org/2001/manifest" );

  QDomElement entry = meta.createElement( "manifest:file-entry" );
  entry.setAttribute( "manifest:media-type", "application/vnd.sun.xml.calc" );
  entry.setAttribute( "manifest:full-path", "/" );
  content.appendChild( entry );

  entry = meta.createElement( "manifest:file-entry" );
  entry.setAttribute( "manifest:media-type", "" );
  entry.setAttribute( "manifest:full-path", "Pictures/" );
  content.appendChild( entry );

  if ( filesWritten & contentXML )
  {
    entry = meta.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "content.xml" );
    content.appendChild( entry );
  }

  if ( filesWritten & stylesXML )
  {
    entry = meta.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "styles.xml" );
    content.appendChild( entry );
  }

  if ( filesWritten & metaXML )
  {
    entry = meta.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "meta.xml" );
    content.appendChild( entry );
  }

  if ( filesWritten & settingsXML )
  {
    entry = meta.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "settings.xml" );
    content.appendChild( entry );
  }

  meta.appendChild( content );

  QCString doc( meta.toCString() );
  kdDebug() << "Manifest: " << doc << endl;

  store->write( doc, doc.length() );

  if ( !store->close() )
    return false;

  return true;
}

#include <opencalcexport.moc>
