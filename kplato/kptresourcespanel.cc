/* This file is part of the KDE project
   Copyright (C) 2003 Thomas Zander <zander@kde.org>

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

#include "kptresourcespanel.h"
#include "kptproject.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>

KPTResourcesPanel::KPTResourcesPanel(QWidget *parent, KPTProject *p) : ResourcesPanelBase(parent) {
    project = p;
    m_groupItem = NULL;
    connect( bAdd, SIGNAL( clicked() ), this, SLOT( slotAddGroup() ) );
    connect (bRemove, SIGNAL( clicked() ), this, SLOT ( slotDeleteGroup() ));
    connect (listOfGroups, SIGNAL( selectionChanged( QListBoxItem*) ), this, SLOT( slotGroupChanged( QListBoxItem*) ));
    connect (bAddResouce, SIGNAL( clicked() ), this, SLOT ( slotAddResource() ));
    connect (bRemoveResource, SIGNAL( clicked() ), this, SLOT ( slotDeleteResource() ));
    connect (listOfResources, SIGNAL( selectionChanged( QListBoxItem*) ), this, SLOT ( slotResourceChanged( QListBoxItem*) ));
    connect (resourceName, SIGNAL( textChanged( const QString&) ), this, SLOT ( slotResourceRename( const QString&) ));
    connect (groupName, SIGNAL( textChanged( const QString&) ), this, SLOT ( slotGroupRename( const QString&) ));
}

void KPTResourcesPanel::slotAddGroup() {
    if(groupName->text().isEmpty()) return;

    KPTResourceGroup *r = new KPTResourceGroup();
    r->setName(groupName->text());
    project->addResourceGroup(r);
    KPTGroupItem *groupItem = new KPTGroupItem(r);
    listOfGroups->insertItem(groupItem);
    listOfGroups->setCurrentItem(groupItem);
}

void KPTResourcesPanel::slotDeleteGroup() {
    KPTGroupItem *groupItem = dynamic_cast<KPTGroupItem*> (listOfGroups->selectedItem());
    if(groupItem == NULL) return;

    listOfGroups->removeItem(listOfGroups->currentItem());
    // TODO remove from project as well..
}

void KPTResourcesPanel::slotAddResource() {
    if(resourceName->text().isEmpty()) return;
    if (!m_groupItem) {
        KMessageBox::sorry(this, i18n("Resources belong to resource groups, select the group first to add a new resource to"));
        return;
    }

    KPTResource *res = new KPTResource();
    res->setName(resourceName->text());
    //res->addWorkingHour(); // TODO
    m_groupItem->m_group->addResource(res, 0);
    KPTResourceItem *resourceItem = new KPTResourceItem(res);
    listOfResources->insertItem(resourceItem);
    resourceName->clear();
}

void KPTResourcesPanel::slotDeleteResource() {
    kdDebug()<<k_funcinfo<<endl; // TODO
}

/* Selected another group */
void KPTResourcesPanel::slotGroupChanged( QListBoxItem *item) {
    //kdDebug()<<k_funcinfo<<endl;
    if (!item)
        return;

    resourceName->clear();

    m_groupItem = (KPTGroupItem *)item;
    listOfResources->clear();
    QPtrListIterator<KPTResource> it(m_groupItem->m_group->resources());
    for ( ; it.current(); ++it ) {
        listOfResources->insertItem(new KPTResourceItem(it.current()));
    }
    groupName->setText(m_groupItem->m_group->name());
    //kdDebug()<<k_funcinfo<<" Groups now: "<<it.count()<<endl;
}

void KPTResourcesPanel::slotGroupRename( const QString &newName) {
    QListBoxItem *item = listOfGroups->selectedItem();
    if(!item) return;

    ((KPTGroupItem *)item)->setName(newName);
    listOfGroups->triggerUpdate(false);
}

/* Select another resource */
void KPTResourcesPanel::slotResourceChanged( QListBoxItem *item) {
    if (!item)
        return;
    resourceName->setText( ((KPTResourceItem *)item)->m_resource->name());
}

void KPTResourcesPanel::slotResourceRename( const QString &newName) {
    QListBoxItem *item = listOfResources->selectedItem();
    if(!item) return;

    ((KPTResourceItem *)item)->setName(newName);
    listOfResources->triggerUpdate(false);
}

#include "kptresourcespanel.moc"
