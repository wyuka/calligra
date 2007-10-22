/* This file is part of the KDE project
   Copyright (C) 2002 Bo Thorsen  bo@sonofthor.dk
   Copyright (C) 2004, 2006 Dag Andersen <danders@get2net.dk>

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

#ifndef KPTSUMMARYSUMMARYTASKDIALOG_H
#define KPTSUMMARYSUMMARYTASKDIALOG_H

#include "kplatoui_export.h"

#include <kdialog.h>

namespace KPlato
{

class SummaryTaskGeneralPanel;
class Task;
class MacroCommand;

/**
 * The dialog that shows and allows you to alter summary tasks.
 */
class KPLATOUI_EXPORT SummaryTaskDialog : public KDialog {
    Q_OBJECT
public:
    /**
     * The constructor for the summary task settings dialog.
     * @param task the task to edit
     * @param parent parent widget
     */
    explicit SummaryTaskDialog(Task &task,  QWidget *parent=0);

    MacroCommand *buildCommand();

protected slots:
    void slotOk();

private:
    SummaryTaskGeneralPanel *m_generalTab;
};

} //KPlato namespace

#endif // SUMMARYTASKDIALOG_H
