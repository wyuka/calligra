/* This file is part of the KDE project
   Copyright (C) 2002 Bo Thorsen  bo@sonofthor.dk
   Copyright (C) 2004 Dag Andersen <danders@get2net.dk>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPTTASKDIALOG_H
#define KPTTASKDIALOG_H

#include "kplatoui_export.h"

#include <kpagedialog.h>


namespace KPlato
{

class Accounts;
class TaskGeneralPanel;
class RequestResourcesPanel;
class DocumentsPanel;
class TaskCostPanel;
class Task;
class StandardWorktime;
class MacroCommand;

/**
 * The dialog that shows and allows you to alter any task.
 */
class KPLATOUI_EXPORT TaskDialog : public KPageDialog {
    Q_OBJECT
public:
    /**
     * The constructor for the task settings dialog.
     * @param task the task to show
     * @param accounts all defined accounts
     * @param workTime defines the number of hours pr day and week
     * @param parent parent widget
     */
    TaskDialog(Task &task, Accounts &accounts, StandardWorktime *workTime=0, QWidget *parent=0);

    MacroCommand *buildCommand();

protected slots:
    void slotOk();

private:
    TaskGeneralPanel *m_generalTab;
    RequestResourcesPanel *m_resourcesTab;
    DocumentsPanel *m_documentsTab;
    TaskCostPanel *m_costTab;
};

} //KPlato namespace

#endif // TASKDIALOG_H
