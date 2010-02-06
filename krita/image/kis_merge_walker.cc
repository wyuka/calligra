/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_merge_walker.h"


KisMergeWalker::KisMergeWalker(QRect cropRect)
    : KisBaseRectsWalker(cropRect)
{
}

KisMergeWalker::~KisMergeWalker()
{
}

void KisMergeWalker::startTrip(KisNodeSP startWith)
{
    visitHigherNode(startWith);

    KisNodeSP prevNode = startWith->prevSibling();
    if(prevNode)
        visitLowerNode(prevNode);
}

void KisMergeWalker::visitHigherNode(KisNodeSP node)
{
    KisNodeSP nextNode = node->nextSibling();

    registerChangeRect(node);

    if (nextNode)
        visitHigherNode(nextNode);
    else if (node->parent())
        startTrip(node->parent());

    registerNeedRect(node, nextNode ? N_NORMAL : N_TOPMOST);
}

void KisMergeWalker::visitLowerNode(KisNodeSP node)
{
    KisNodeSP prevNode = node->prevSibling();
    registerNeedRect(node, prevNode ? N_LOWER : N_BOTTOMMOST);

    if (prevNode)
        visitLowerNode(prevNode);
}