/***************************************************************************
                          kudesignerview.h  -  description
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

#ifndef KUDESIGNERVIEW_H
#define KUDESIGNERVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for Qt

// application specific includes
#include "mycanvas.h"
#include "cv.h"

class KuDesignerDoc;
class KPrinter;

/** The KuDesignerView class provides the view widget for the KuDesignerApp instance.	
 * The View instance inherits QWidget as a base class and represents the view object of a KTMainWindow. As KuDesignerView is part of the
 * docuement-view model, it needs a reference to the document object connected with it by the KuDesignerApp class to manipulate and display
 * the document structure provided by the KuDesignerDoc class.
 * 	
 * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
 * @version KDevelop version 0.4 code generation
 */
class KuDesignerView : public ReportCanvas
{
  Q_OBJECT
  public:
    /** Constructor for the main view */
    KuDesignerView(QWidget *parent = 0, const char *name=0);
    /** Destructor for the main view */
    ~KuDesignerView();

    /** returns a pointer to the document connected to the view instance. Mind that this method requires a KuDesignerApp instance as a parent
     * widget to get to the window document pointer by calling the KuDesignerApp::getDocument() method.
     *
     * @see KuDesignerApp#getDocument
     */
    KuDesignerDoc *getDocument() const;

    /** contains the implementation for printing functionality */
    void print(KPrinter *pPrinter);
	
  private:
};

#endif // KUDESIGNERVIEW_H
