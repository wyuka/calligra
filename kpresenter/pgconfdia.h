// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2002, 2003 Ariya Hidayat <ariya@kde.org>

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

#ifndef PGCONFDIA_H
#define PGCONFDIA_H

#include "global.h"

#include <qmap.h>
#include <qpen.h>
#include <qvaluelist.h>

#include <kdialogbase.h>

class KPresenterDoc;

class QCheckBox;
class QComboBox;
class QColor;
class QListView;
class QRadioButton;

class KIntNumInput;
class KColorButton;
class QSlider;

class PgConfDia : public KDialogBase
{
    Q_OBJECT

public:

    // constructor - destructor
    PgConfDia( QWidget* parent, KPresenterDoc* doc );
    ~PgConfDia();
    bool getInfiniteLoop() const;
    bool getManualSwitch() const;
    bool getPresentationDuration() const;
    QPen getPen() const;
    QValueList<bool> getSelectedSlides() const;

    QString presentationName() const;

protected:

    KPresenterDoc* m_doc;

    QCheckBox *infiniteLoop, *presentationDuration;
    QRadioButton *m_manualButton, *m_autoButton;
    KColorButton* penColor;
    KIntNumInput* penWidth;
    QComboBox *m_customSlideCombobox;
    QListView *slides;
    QRadioButton *m_customSlide, *m_selectedSlide;
    void setupPageGeneral();
    void setupPageSlides();

public slots:
    void confDiaOk() { emit pgConfDiaOk(); }

signals:
    void pgConfDiaOk();

protected slots:
    void selectAllSlides();
    void deselectAllSlides();
};

#endif
