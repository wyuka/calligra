/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KEXIRELATIONDESIGNSHAPEPLUGIN_H
#define KEXIRELATIONDESIGNSHAPEPLUGIN_H

#include <QObject>

class KexiRelationDesignShapePlugin : public QObject {
    Q_OBJECT
    public:
        KexiRelationDesignShapePlugin(QObject *parent, const QStringList&);
};


#endif // KEXIRELATIONDESIGNSHAPEPLUGIN_H
