/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SectionShapeContainerModel.h"

#include <KoShapeContainer.h>

SectionShapeContainerModel::SectionShapeContainerModel()
{
}

SectionShapeContainerModel::~SectionShapeContainerModel()
{
}

void SectionShapeContainerModel::add(KoShape *child) {
    if (m_members.contains(child))
        return;
    m_members.append(child);
}
void SectionShapeContainerModel::setClipping(const KoShape *, bool) { }
bool SectionShapeContainerModel::childClipped(const KoShape *) const {
    return false;
}
void SectionShapeContainerModel::remove(KoShape *child) {
    m_members.removeAll(child);
}
int SectionShapeContainerModel::count() const {
    return m_members.count();
}
QList<KoShape*> SectionShapeContainerModel::iterator() const {
    return QList<KoShape*>(m_members);
}
void SectionShapeContainerModel::containerChanged(KoShapeContainer *) {
}

void SectionShapeContainerModel::childChanged(KoShape *, KoShape::ChangeType) {
}

bool SectionShapeContainerModel::isChildLocked(const KoShape *child) const {
    Q_ASSERT(child->parent());
    return child->isGeometryProtected() || child->parent()->isGeometryProtected();
}
