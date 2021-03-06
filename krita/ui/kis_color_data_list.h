/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>

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

#ifndef KIS_COLOR_DATA_LIST_H
#define KIS_COLOR_DATA_LIST_H

#include "kis_min_heap.h"
#include <KoColor.h>
#include <QColor>
#include <QList>

class KisColorDataList
{
public:
    static const int MAX_RECENT_COLOR = 12;

    KisColorDataList();
    ~KisColorDataList();

    inline int size () { return m_guiList.size(); };
    inline void printPriorityList () { m_priorityList->printHeap(); };
    inline int leastUsedGuiPos() { return findPos(m_priorityList->valueAt(0)); };

    void printGuiList();
    const KoColor& guiColor (int pos);
    void append(const KoColor&);
    void appendNew(const KoColor&);
    void removeLeastUsed();
    void updateKey (int guiPos);

    /*find position of the color on the gui list*/
    int findPos (const KoColor&);

private:
    KisMinHeap <KoColor, MAX_RECENT_COLOR> *m_priorityList;
    QList <PriorityNode <KoColor>*> m_guiList;

    int m_key;

    int guiInsertPos(const KoColor&);

    /*compares c1 and c2 based on HSV.
      c1 < c2, returns -1
      c1 = c2, returns 0
      c1 > c2, returns 1 */
    int hsvComparison (const KoColor& c1, const KoColor& c2);
};

#endif // KIS_COLOR_DATA_LIST_H
