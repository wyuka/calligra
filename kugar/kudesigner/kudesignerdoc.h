/***************************************************************************
                          kudesignerdoc.h  -  description
                             -------------------
    begin                : Thu Jun  6 11:31:39 EEST 2002
    copyright            : (C) 2002 by Alexander Dymo
    email                : cloudtemple@mksat.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KUDESIGNERDOC_H
#define KUDESIGNERDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

// include files for KDE
#include <kapplication.h>
#include <kurl.h>

// forward declaration of the KuDesigner classes
class KuDesignerView;
class MyCanvas;
class CanvasReportItem;
class CanvasBand;
class QDomNode;

/**	KuDesignerDoc provides a document object for a document-view model.
  *
  * The KuDesignerDoc class provides a document object that can be used in conjunction with the classes KuDesignerApp and KuDesignerView
  * to create a document-view model for standard KDE applications based on KApplication and KMainWindow. Thereby, the document object
  * is created by the KuDesignerApp instance and contains the document structure with the according methods for manipulation of the document
  * data by KuDesignerView objects. Also, KuDesignerDoc contains the methods for serialization of the document data from and to files.
  *
  * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team. 	
  * @version KDevelop version 1.2 code generation
  */
class KuDesignerDoc : public QObject
{
  Q_OBJECT
  public:
    /** Constructor for the fileclass of the application */
    KuDesignerDoc(QWidget *parent, const char *name=0);
    /** Destructor for the fileclass of the application */
    ~KuDesignerDoc();

    /** adds a view to the document which represents the document contents. Usually this is your main view. */
    void addView(KuDesignerView *view);
    /** removes a view from the list of currently connected views */
    void removeView(KuDesignerView *view);
    /** sets the modified flag for the document after a modifying action on the view connected to the document.*/
    void setModified(bool _m=true){ modified=_m; };
    /** returns if the document is modified or not. Use this to determine if your document needs saving by the user on closing.*/
    bool isModified(){ return modified; };
    /** "save modified" - asks the user for saving if the document is modified */
    bool saveModified();	
    /** deletes the document's contents */
    void deleteContents();
    /** initializes the document generally */
    bool newDocument(QSize pSize, int top, int bottom, int left, int right,
        bool orientation,int pageSize);
    /** closes the acutal document */
    void closeDocument();
    /** loads the document by filename and format and emits the updateViews() signal */
    bool openDocument(const KURL& url, const char *format=0);
    /** saves the document under filename and format.*/	
    bool saveDocument(const KURL& url, const char *format=0);
    /** returns the KURL of the document */
    const KURL& URL() const;
    /** sets the URL of the document */
	  void setURL(const KURL& url);
	
  public slots:
    /** calls repaint() on all views connected to the document object and is called by the view by which the document has been changed.
     * As this view normally repaints itself, it is excluded from the paintEvent.
     */
    void slotUpdateAllViews(KuDesignerView *sender);
 	
  public:	
    /** the list of the views currently connected to the document */
    static QPtrList<KuDesignerView> *pViewList;	

    MyCanvas *canvas();
    void setCanvas(MyCanvas *c);

  private:
    /** the modified flag of the current document */
    bool modified;
    KURL doc_url;
    MyCanvas *docCanvas;


    void setReportItemAttributes(QDomNode *node, CanvasReportItem *item);
    void addReportItems(QDomNode *node, CanvasBand *section);
    void setReportHeaderAttributes(QDomNode *node);
    void setReportFooterAttributes(QDomNode *node);
    void setPageHeaderAttributes(QDomNode *node);
    void setPageFooterAttributes(QDomNode *node);
    void setDetailHeaderAttributes(QDomNode *node);
    void setDetailAttributes(QDomNode *node);
    void setDetailFooterAttributes(QDomNode *node);
};

#endif // KUDESIGNERDOC_H
