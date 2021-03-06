/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Christian Muehlhaeuser <chris@chris.de>
 *  Copyright (c) 2004-2006 Seb Ruiz <ruiz@kde.org>
 *  Copyright (c) 2004,2005 Max Howell <max.howell@methylblue.com>
 *  Copyright (c) 2005 Gabor Lehel <illissius@gmail.com>
 *  Copyright (c) 2008,2009 Mark Kretschmann <kretschmann@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_FLOATING_MESSAGE_H
#define KIS_FLOATING_MESSAGE_H

#include <QWidget>
#include <QString>
#include <QImage>
#include <QTimer>
#include <QTimeLine>

/**
 * @brief The KisFloatingMessage class shows the given message in a semi-transparent
 * bubble that doesn' take focus and slowly fades away.
 *
 * Heavily based on Amarok's Osd.cpp
 */
class KisFloatingMessage : public QWidget
{
    Q_OBJECT

public:

    explicit KisFloatingMessage(const QString &message, QWidget *parent);
    void showMessage();

protected:

    void paintEvent(QPaintEvent *e);

private slots:

    void startFade();
    void removeMessage();
    void updateOpacity(int value);
private:

    QRect determineMetrics(const int M);

    QString m_message;
    QImage m_icon;
    QPixmap m_scaledIcon;
    QTimer m_timer;
    int m_m;
    QTimeLine m_fadeTimeLine;
};

#endif // KIS_FLOATING_MESSAGE_H
