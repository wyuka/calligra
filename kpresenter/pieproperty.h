// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef PIEPROPERTY_H
#define PIEPROPERTY_H

#include <qwidget.h>

#include "kprcommand.h"

class PiePropertyUI;

class PieProperty : public QWidget
{
    Q_OBJECT
public:
    PieProperty( QWidget *parent, const char *name, PieValueCmd::PieValues pieValues );
    ~PieProperty();

    int getPiePropertyChange() const;
    PieValueCmd::PieValues getPieValues() const;

    void apply();
private:
    PiePropertyUI *m_ui;

    PieValueCmd::PieValues m_pieValues;

protected slots:
    void slotReset();
    void slotTypeChanged( int pos );
    void slotAngleChanged( int num );
    void slotLengthChanged( int num );
};

#endif /* PIEPROPERTY_H */
