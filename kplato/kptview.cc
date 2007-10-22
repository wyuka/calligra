/* This file is part of the KDE project
  Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
  Copyright (C) 2002 - 2007 Dag Andersen <danders@get2net.dk>

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

#include "kptview.h"

//#include <kprinter.h>
#include <kmessagebox.h>

#include "KoDocumentInfo.h"
#include <KoMainWindow.h>
#include <KoToolManager.h>
#include <KoToolBox.h>
#include <KoDocumentChild.h>

#include <QApplication>
#include <QDockWidget>
#include <QIcon>
#include <QLayout>
#include <QColor>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <qsize.h>
#include <QStackedWidget>
#include <QHeaderView>
#include <QRect>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QItemDelegate>
#include <QStyle>
#include <QVariant>
#include <QPrinter>
#include <QPrintDialog>

#include <kicon.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <kstandardshortcut.h>
#include <kaccelgen.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kxmlguifactory.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <k3command.h>
#include <ktoggleaction.h>
#include <kfiledialog.h>
#include <kparts/event.h>
#include <kparts/partmanager.h>
#include <KoQueryTrader.h>

#include "kptviewbase.h"
#include "kptaccountsview.h"
#include "kptaccountseditor.h"
#include "kptcalendareditor.h"
#include "kptchartview.h"
#include "kptfactory.h"
#include "kptmilestoneprogressdialog.h"
#include "kptnode.h"
#include "kptpart.h"
#include "kptproject.h"
#include "kptmainprojectdialog.h"
#include "kpttask.h"
#include "kptsummarytaskdialog.h"
#include "kpttaskdialog.h"
#include "kpttaskprogressdialog.h"
#include "kptganttview.h"
//#include "kptreportview.h"
#include "kpttaskeditor.h"
#include "kptdependencyeditor.h"
#include "kptperteditor.h"
#include "kptdatetime.h"
#include "kptcommand.h"
#include "kptrelation.h"
#include "kptrelationdialog.h"
#include "kptresourceappointmentsview.h"
#include "kptresourceeditor.h"
#include "kptscheduleeditor.h"
#include "kptresourcedialog.h"
#include "kptresource.h"
#include "kptresourcesdialog.h"
#include "kptcalendarlistdialog.h"
#include "kptstandardworktimedialog.h"
#include "kptconfigdialog.h"
#include "kptwbsdefinitiondialog.h"
#include "kptaccountsdialog.h"
#include "kptresourceassignmentview.h"
#include "kpttaskstatusview.h"
#include "kptsplitterview.h"
#include "kptpertresult.h"

#include "kptviewlistdialog.h"
#include "kptviewlistdocker.h"
#include "kptviewlist.h"
#include "kptviewlistcommand.h"

#include "KPtViewAdaptor.h"

#include <assert.h>

namespace KPlato
{

//-------------------------------
View::View( Part* part, QWidget* parent )
        : KoView( part, parent ),
        m_currentEstimateType( Estimate::Use_Expected ),
        m_manager( 0 ),
        m_readWrite( false )
{
    //kDebug();
//    getProject().setCurrentSchedule( Schedule::Expected );

    setComponentData( Factory::global() );
    if ( !part->isReadWrite() )
        setXMLFile( "kplato_readonly.rc" );
    else
        setXMLFile( "kplato.rc" );

    m_dbus = new ViewAdaptor( this );
    QDBusConnection::sessionBus().registerObject( '/' + objectName(), this );

    m_sp = new QSplitter( this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin(0);
    layout->addWidget( m_sp );

    ViewListDockerFactory vl(this);
    ViewListDocker *docker = dynamic_cast<ViewListDocker *>(createDockWidget(&vl));
    if (docker->view() != this) docker->setView(this);
    m_viewlist = docker->viewList();
    
    m_tab = new QStackedWidget( m_sp );

////////////////////////////////////////////////////////////////////////////////////////////////////
    // Add sub views
    createViews();
    // Add child documents
    createChildDocumentViews();

    connect( m_viewlist, SIGNAL( activated( ViewListItem*, ViewListItem* ) ), SLOT( slotViewActivated( ViewListItem*, ViewListItem* ) ) );
    connect( m_viewlist, SIGNAL( viewListItemRemoved( ViewListItem* ) ), SLOT( slotViewListItemRemoved( ViewListItem* ) ) );
    connect( m_viewlist, SIGNAL( viewListItemInserted( ViewListItem* ) ), SLOT( slotViewListItemInserted( ViewListItem* ) ) );

    connect( m_tab, SIGNAL( currentChanged( int ) ), this, SLOT( slotCurrentChanged( int ) ) );

    // The menu items
    // ------ Edit
    actionCut = actionCollection()->addAction(KStandardAction::Cut,  "edit_cut", this, SLOT( slotEditCut() ));
    actionCopy = actionCollection()->addAction(KStandardAction::Copy,  "edit_copy", this, SLOT( slotEditCopy() ));
    actionPaste = actionCollection()->addAction(KStandardAction::Paste,  "edit_paste", this, SLOT( slotEditPaste() ));

    // ------ View

    actionViewSelector  = new KToggleAction(i18n("Show Selector"), this);
    actionCollection()->addAction("view_show_selector", actionViewSelector );
    connect( actionViewSelector, SIGNAL( triggered( bool ) ), SLOT( slotViewSelector( bool ) ) );

    m_scheduleActionGroup = new QActionGroup( this );
    m_scheduleActionGroup->setExclusive( true );
    connect( m_scheduleActionGroup, SIGNAL( triggered( QAction* ) ), SLOT( slotViewSchedule( QAction* ) ) );

    actionViewResourceAppointments  = new KToggleAction(i18n("Show allocations"), this);
    actionCollection()->addAction("view_resource_appointments", actionViewResourceAppointments );
    connect( actionViewResourceAppointments, SIGNAL( triggered( bool ) ), SLOT( slotViewResourceAppointments() ) );

    // ------ Insert

    // ------ Project
    actionEditMainProject  = new KAction(KIcon( "edit" ), i18n("Edit Main Project..."), this);
    actionCollection()->addAction("project_edit", actionEditMainProject );
    connect( actionEditMainProject, SIGNAL( triggered( bool ) ), SLOT( slotProjectEdit() ) );

    actionEditStandardWorktime  = new KAction(KIcon( "edit" ), i18n("Edit Standard Worktime..."), this);
    actionCollection()->addAction("project_worktime", actionEditStandardWorktime );
    connect( actionEditStandardWorktime, SIGNAL( triggered( bool ) ), SLOT( slotProjectWorktime() ) );
    actionEditCalendarList  = new KAction(KIcon( "edit" ), i18n("Edit Calendar..."), this);
    actionCollection()->addAction("project_calendar", actionEditCalendarList );
    connect( actionEditCalendarList, SIGNAL( triggered( bool ) ), SLOT( slotProjectCalendar() ) );
    actionEditAccounts  = new KAction(KIcon( "edit" ), i18n("Edit Accounts..."), this);
    actionCollection()->addAction("project_accounts", actionEditAccounts );
    connect( actionEditAccounts, SIGNAL( triggered( bool ) ), SLOT( slotProjectAccounts() ) );
    actionEditResources  = new KAction(KIcon( "edit" ), i18n("Edit Resources..."), this);
    actionCollection()->addAction("project_resources", actionEditResources );
    connect( actionEditResources, SIGNAL( triggered( bool ) ), SLOT( slotProjectResources() ) );


    /*    // ------ Reports
    actionFirstpage = actionCollection()->addAction(KStandardAction::FirstPage, "go_firstpage", m_reportview,SLOT(slotPrevPage()));
        connect(m_reportview, SIGNAL(setFirstPageActionEnabled(bool)), actionFirstpage, SLOT(setEnabled(bool)));
    actionPriorpage = actionCollection()->addAction(KStandardAction::Prior, "go_prevpage", m_reportview,SLOT(slotPrevPage()));
        connect(m_reportview, SIGNAL(setPriorPageActionEnabled(bool)), actionPriorpage, SLOT(setEnabled(bool)));
    actionNextpage = actionCollection()->addAction(KStandardAction::Next,  "go_nextpage", m_reportview,SLOT(slotNextPage()));
        connect(m_reportview, SIGNAL(setNextPageActionEnabled(bool)), actionNextpage, SLOT(setEnabled(bool)));
    actionLastpage = actionCollection()->addAction(KStandardAction::LastPage,  "go_lastpage", m_reportview,SLOT(slotLastPage()));
        connect(m_reportview, SIGNAL(setLastPageActionEnabled(bool)), actionLastpage, SLOT(setEnabled(bool)));
        m_reportview->enableNavigationBtn();*/
    mainWindow() ->toolBar( "report" ) ->hide();

    //     new KAction(i18n("Design..."), "report_design", 0, this,
    //         SLOT(slotReportDesign()), actionCollection(), "report_design");


    // ------ Tools
    actionDefineWBS  = new KAction(KIcon( "tools_define_wbs" ), i18n("Define WBS Pattern..."), this);
    actionCollection()->addAction("tools_generate_wbs", actionDefineWBS );
    connect( actionDefineWBS, SIGNAL( triggered( bool ) ), SLOT( slotDefineWBS() ) );

    actionGenerateWBS  = new KAction(KIcon( "tools_generate_wbs" ), i18n("Generate WBS Code"), this);
    actionCollection()->addAction("tools_define_wbs", actionGenerateWBS );
    connect( actionGenerateWBS, SIGNAL( triggered( bool ) ), SLOT( slotGenerateWBS() ) );

    // ------ Settings
    actionConfigure  = new KAction(KIcon( "configure" ), i18n("Configure KPlato..."), this);
    actionCollection()->addAction("configure", actionConfigure );
    connect( actionConfigure, SIGNAL( triggered( bool ) ), SLOT( slotConfigure() ) );

    // ------ Popup
    actionOpenNode  = new KAction(KIcon( "edit" ), i18n("Edit..."), this);
    actionCollection()->addAction("node_properties", actionOpenNode );
    connect( actionOpenNode, SIGNAL( triggered( bool ) ), SLOT( slotOpenNode() ) );
    actionTaskProgress  = new KAction(KIcon( "edit" ), i18n("Progress..."), this);
    actionCollection()->addAction("task_progress", actionTaskProgress );
    connect( actionTaskProgress, SIGNAL( triggered( bool ) ), SLOT( slotTaskProgress() ) );
    actionDeleteTask  = new KAction(KIcon( "edit-delete" ), i18n("Delete Task"), this);
    actionCollection()->addAction("delete_task", actionDeleteTask );
    connect( actionDeleteTask, SIGNAL( triggered( bool ) ), SLOT( slotDeleteTask() ) );

    actionEditResource  = new KAction(KIcon( "edit" ), i18n("Edit Resource..."), this);
    actionCollection()->addAction("edit_resource", actionEditResource );
    connect( actionEditResource, SIGNAL( triggered( bool ) ), SLOT( slotEditResource() ) );

    actionEditCalendar  = new KAction(KIcon( "edit" ), i18n("Edit Calendar..."), this);
    actionCollection()->addAction("edit_calendar", actionEditCalendar );
    connect( actionEditCalendar, SIGNAL( triggered( bool ) ), SLOT( slotEditCalendar() ) );

    actionEditRelation  = new KAction(KIcon( "edit" ), i18n("Edit Dependency..."), this);
    actionCollection()->addAction("edit_dependency", actionEditRelation );
    connect( actionEditRelation, SIGNAL( triggered( bool ) ), SLOT( slotModifyRelation() ) );
    actionDeleteRelation  = new KAction(KIcon( "edit-delete" ), i18n("Delete Dependency..."), this);
    actionCollection()->addAction("delete_dependency", actionDeleteRelation );
    connect( actionDeleteRelation, SIGNAL( triggered( bool ) ), SLOT( slotDeleteRelation() ) );

    // Viewlist popup
    connect( m_viewlist, SIGNAL( createView() ), SLOT( slotCreateView() ) );
    connect( m_viewlist, SIGNAL( createKofficeDocument( KoDocumentEntry& ) ), SLOT( slotCreateKofficeDocument( KoDocumentEntry& ) ) );

#ifndef NDEBUG
    //new KAction("Print Debug", CTRL+Qt::SHIFT+Qt::Key_P, this, SLOT( slotPrintDebug()), actionCollection(), "print_debug");
    QAction *action  = new KAction("Print Debug", this);
    actionCollection()->addAction("print_debug", action );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( slotPrintSelectedDebug() ) );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_P ) );
    action  = new KAction("Print Calendar Debug", this);
    actionCollection()->addAction("print_calendar_debug", action );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( slotPrintCalendarDebug() ) );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
    //     new KAction("Print Test Debug", CTRL+Qt::SHIFT+Qt::Key_T, this, SLOT(slotPrintTestDebug()), actionCollection(), "print_test_debug");


#endif
    // Stupid compilers ;)
#ifndef NDEBUG
    /*  Q_UNUSED( actPrintSelectedDebug );
        Q_UNUSED( actPrintCalendarDebug );*/
#endif

    m_progress = 0;
    m_estlabel = new QLabel( "", 0 );
    if ( statusBar() ) {
        addStatusBarItem( m_estlabel, 0, true );
        //m_progress = new QProgressBar();
        //addStatusBarItem( m_progress, 0, true );
        //m_progress->hide();
    }
    connect( &getProject(), SIGNAL( scheduleChanged( MainSchedule* ) ), SLOT( slotScheduleChanged( MainSchedule* ) ) );

    connect( &getProject(), SIGNAL( scheduleAdded( const MainSchedule* ) ), SLOT( slotScheduleAdded( const MainSchedule* ) ) );
    connect( &getProject(), SIGNAL( scheduleRemoved( const MainSchedule* ) ), SLOT( slotScheduleRemoved( const MainSchedule* ) ) );
    slotPlugScheduleActions();

    m_viewlist->setSelected( m_viewlist->findItem( "TaskEditor" ) );
    
    loadContext();
    
    connect( part, SIGNAL( changed() ), SLOT( slotUpdate() ) );
    
    //kDebug()<<" end";
}

View::~View()
{
    removeStatusBarItem( m_estlabel );
    delete m_estlabel;
    delete m_scheduleActionGroup;
}

ViewAdaptor* View::dbusObject()
{
    return m_dbus;
}

void View::createViews()
{
    Context *ctx = getPart()->context();
    if ( ctx && ctx->isLoaded() ) {
        kDebug()<<"isLoaded"<<endl;
        KoXmlNode n = ctx->context().namedItem( "categories" );
        if ( n.isNull() ) {
            kWarning()<<"No categories"<<endl;
        } else {
            n = n.firstChild();
            for ( ; ! n.isNull(); n = n.nextSibling() ) {
                if ( ! n.isElement() ) {
                    continue;
                }
                KoXmlElement e = n.toElement();
                if (e.tagName() != "category") {
                    continue;
                }
                kDebug()<<"category: "<<e.attribute( "tag" )<<endl;
                ViewListItem *cat;
                cat = m_viewlist->addCategory( e.attribute( "tag" ), e.attribute( "name" ) );
                KoXmlNode n1 = e.firstChild();
                for ( ; ! n1.isNull(); n1 = n1.nextSibling() ) {
                    if ( ! n1.isElement() ) {
                        continue;
                    }
                    KoXmlElement e1 = n1.toElement();
                    if (e1.tagName() != "view") {
                        continue;
                    }
                    ViewBase *v = 0;
                    QString type = e1.attribute( "viewtype" );
                    QString tag = e1.attribute( "tag" );
                    QString name = e1.attribute( "name" );
                    QString tip = e1.attribute( "tooltip" );
                    //FIXME: Remove KPlato:: from type
                    if ( type == "KPlato::CalendarEditor" ) {
                        v = createCalendarEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::AccountsEditor" ) {
                        v = createAccountsEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::ResourceEditor" ) {
                        v = createResourcEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::TaskEditor" ) {
                        v = createTaskEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::DependencyEditor" ) {
                        v = createDependencyEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::PertEditor" ) {
                        v = createPertEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::ScheduleEditor" ) {
                        v = createScheduleEditor( cat, tag, name, tip );
                    } else if ( type == "KPlato::ScheduleHandlerView" ) {
                        v = createScheduleHandler( cat, tag, name, tip );
                    } else if ( type == "KPlato::TaskStatusView" ) {
                        v = createTaskStatusView( cat, tag, name, tip );
                    } else if ( type == "KPlato::GanttView" ) {
                        v = createGanttView( cat, tag, name, tip );
                    } else if ( type == "KPlato::MilestoneGanttView" ) {
                        v = createMilestoneGanttView( cat, tag, name, tip );
                    } else if ( type == "KPlato::ResourceAppointmentsView" ) {
                        v = createResourceAppointmentsView( cat, tag, name, tip );
                    } else if ( type == "KPlato::AccountsView" ) {
                        v = createAccountsView( cat, tag, name, tip );
                    } else if ( type == "KPlato::ResourceAssignmentView" ) {
                        v = createResourceAssignmentView( cat, tag, name, tip );
                    } else if ( type == "KPlato::ChartView" ) {
                        v = createChartView( cat, tag, name, tip );
                    } else  {
                        kWarning()<<"Unknown viewtype: "<<type<<endl;
                    }
                    //KoXmlNode settings = e1.namedItem( "settings " ); ????
                    KoXmlNode settings = e1.firstChild();
                    for ( ; ! settings.isNull(); settings = settings.nextSibling() ) {
                        if ( settings.nodeName() == "settings" ) {
                            break;
                        }
                    }
                    if ( v && settings.isElement() ) {
                        kDebug()<<" settings"<<endl;
                        v->loadContext( settings.toElement() );
                    }
                }
            }
        }
    }
    if ( m_tab->count() == 0 ) {
        kDebug()<<"Default"<<endl;
        ViewListItem *cat;
        cat = m_viewlist->addCategory( "Editors", i18n( "Editors" ) );
        
        createCalendarEditor( cat, "CalendarEditor", i18n( "Work & Vacation" ), i18n( "Edit working- and vacation days for resources" ) );
        
        createAccountsEditor( cat, "AccountEditor", i18n( "Accounts" ), i18n( "Edit cost breakdown structure." ) );
        
        createResourcEditor( cat, "ResourceEditor", i18n( "Resources" ), i18n( "Edit resource breakdown structure." ) );

        createTaskEditor( cat, "TaskEditor", i18n( "Tasks" ), i18n( "Edit work breakdown structure" ) );
        
        createDependencyEditor( cat, "DependencyEditor", i18n( "Dependencies" ), i18n( "Edit task dependenies" ) );
        
        createPertEditor( cat, "PertEditor", i18n( "Pert" ), i18n( "Edit task dependencies" ) );
        
        createScheduleHandler( cat, "ScheduleHandler", i18n( "Schedules" ), i18n( "Calculate and analyze project schedules" ) );
    
        cat = m_viewlist->addCategory( "Views", i18n( "Views" ) );
        createTaskStatusView( cat, "TaskStatusView", i18n( "Task Status" ), i18n( "View task progress information" ) );
        
        createGanttView( cat, "GanttView", i18n( "Gantt" ), i18n( "View gantt chart" ) );
        
        createMilestoneGanttView( cat, "MilestoneGanttView", i18n( "Milestone Gantt" ), i18n( "View milestone gantt chart" ) );
        
        createResourceAppointmentsView( cat, "ResourceAppointmentsView", i18n( "Resource Assignments" ), i18n( "View resource assignments" ) );

        createAccountsView( cat, "AccountsView", i18n( "Accounts" ), i18n( "View planned cost" ) );

        createResourceAssignmentView( cat, "ResourceAssignmentView", i18n( "Tasks by resources" ), i18n( "View task status per resource" ) );

        createChartView( cat, "PerformanceChart", i18n( "Performance Chart" ), i18n( "Cost and schedule monitoring" ) );
    }
}

ViewBase *View::createResourceAppointmentsView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ResourceAppointmentsView *v = new ResourceAppointmentsView( getPart(), m_tab );
    m_tab->addWidget( v );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, v, getPart(), "resource_view" );
    i->setToolTip( 0, tip );

    connect( v, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), v, SLOT( setScheduleManager( ScheduleManager* ) ) );
    
    connect( v, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );

    v->setProject( &( getProject() ) );
    v->setScheduleManager( currentScheduleManager() );
    v->updateReadWrite( m_readWrite );
    return v;
}

ViewBase *View::createResourcEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ResourceEditor *resourceeditor = new ResourceEditor( getPart(), m_tab );
    m_tab->addWidget( resourceeditor );
    resourceeditor->draw( getProject() );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, resourceeditor, getPart(), "resource_editor" );
    i->setToolTip( 0, tip );

    connect( resourceeditor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( resourceeditor, SIGNAL( addResource( ResourceGroup* ) ), SLOT( slotAddResource( ResourceGroup* ) ) );
    connect( resourceeditor, SIGNAL( deleteObjectList( QObjectList ) ), SLOT( slotDeleteResourceObjects( QObjectList ) ) );

    connect( resourceeditor, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    resourceeditor->updateReadWrite( m_readWrite );
    return resourceeditor;
}

ViewBase *View::createTaskEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    TaskEditor *taskeditor = new TaskEditor( getPart(), m_tab );
    m_tab->addWidget( taskeditor );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, taskeditor, getPart(), "task_editor" );
    i->setToolTip( 0, tip );

    taskeditor->draw( getProject() );

    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), taskeditor, SLOT( slotCurrentScheduleManagerChanged( ScheduleManager* ) ) );
    
    connect( taskeditor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( taskeditor, SIGNAL( addTask() ), SLOT( slotAddTask() ) );
    connect( taskeditor, SIGNAL( addMilestone() ), SLOT( slotAddMilestone() ) );
    connect( taskeditor, SIGNAL( addSubtask() ), SLOT( slotAddSubTask() ) );
    connect( taskeditor, SIGNAL( deleteTaskList( QList<Node*> ) ), SLOT( slotDeleteTask( QList<Node*> ) ) );
    connect( taskeditor, SIGNAL( moveTaskUp() ), SLOT( slotMoveTaskUp() ) );
    connect( taskeditor, SIGNAL( moveTaskDown() ), SLOT( slotMoveTaskDown() ) );
    connect( taskeditor, SIGNAL( indentTask() ), SLOT( slotIndentTask() ) );
    connect( taskeditor, SIGNAL( unindentTask() ), SLOT( slotUnindentTask() ) );
    


    connect( taskeditor, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    taskeditor->updateReadWrite( m_readWrite );
    return taskeditor;
}

ViewBase *View::createAccountsEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    AccountsEditor *ae = new AccountsEditor( getPart(), m_tab );
    m_tab->addWidget( ae );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, ae, getPart(), "accounts_editor" );
    i->setToolTip( 0, tip );

    ae->draw( getProject() );

    connect( ae, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
    ae->updateReadWrite( m_readWrite );
    return ae;
}

ViewBase *View::createCalendarEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    CalendarEditor *calendareditor = new CalendarEditor( getPart(), m_tab );
    m_tab->addWidget( calendareditor );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, calendareditor, getPart(), "calendar_editor" );
    i->setToolTip( 0, tip );

    calendareditor->draw( getProject() );

    connect( calendareditor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( calendareditor, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    calendareditor->updateReadWrite( m_readWrite );
    return calendareditor;
}

ViewBase *View::createScheduleHandler( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ScheduleHandlerView *handler = new ScheduleHandlerView( getPart(), m_tab );
    m_tab->addWidget( handler );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, handler, getPart(), "schedule_editor" );
    i->setToolTip( 0, tip );

    connect( handler->scheduleEditor(), SIGNAL( addScheduleManager( Project* ) ), SLOT( slotAddScheduleManager( Project* ) ) );
    connect( handler->scheduleEditor(), SIGNAL( deleteScheduleManager( Project*, ScheduleManager* ) ), SLOT( slotDeleteScheduleManager( Project*, ScheduleManager* ) ) );

    connect( handler->scheduleEditor(), SIGNAL( calculateSchedule( Project*, ScheduleManager* ) ), SLOT( slotCalculateSchedule( Project*, ScheduleManager* ) ) );

    connect( handler, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), handler, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ) );
    
    handler->draw( getProject() );
    handler->updateReadWrite( m_readWrite );
    return handler;
}

ScheduleEditor *View::createScheduleEditor( QWidget *parent )
{
    ScheduleEditor *scheduleeditor = new ScheduleEditor( getPart(), parent );
    
    connect( scheduleeditor, SIGNAL( addScheduleManager( Project* ) ), SLOT( slotAddScheduleManager( Project* ) ) );
    connect( scheduleeditor, SIGNAL( deleteScheduleManager( Project*, ScheduleManager* ) ), SLOT( slotDeleteScheduleManager( Project*, ScheduleManager* ) ) );

    connect( scheduleeditor, SIGNAL( calculateSchedule( Project*, ScheduleManager* ) ), SLOT( slotCalculateSchedule( Project*, ScheduleManager* ) ) );

    scheduleeditor->updateReadWrite( m_readWrite );
    return scheduleeditor;
}

ViewBase *View::createScheduleEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ScheduleEditor *scheduleeditor = new ScheduleEditor( getPart(), m_tab );
    m_tab->addWidget( scheduleeditor );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, scheduleeditor, getPart(), "schedule_editor" );
    i->setToolTip( 0, tip );

    scheduleeditor->setProject( &( getProject() ) );

    connect( scheduleeditor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( scheduleeditor, SIGNAL( addScheduleManager( Project* ) ), SLOT( slotAddScheduleManager( Project* ) ) );
    connect( scheduleeditor, SIGNAL( deleteScheduleManager( Project*, ScheduleManager* ) ), SLOT( slotDeleteScheduleManager( Project*, ScheduleManager* ) ) );

    connect( scheduleeditor, SIGNAL( calculateSchedule( Project*, ScheduleManager* ) ), SLOT( slotCalculateSchedule( Project*, ScheduleManager* ) ) );
    scheduleeditor->updateReadWrite( m_readWrite );
    return scheduleeditor;
}


ViewBase *View::createDependencyEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    DependencyEditor *editor = new DependencyEditor( getPart(), m_tab );
    m_tab->addWidget( editor );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, editor, getPart(), "task_editor" );
    i->setToolTip( 0, tip );

    editor->draw( getProject() );

    connect( editor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( editor, SIGNAL( addRelation( Node*, Node*, int ) ), SLOT( slotAddRelation( Node*, Node*, int ) ) );
    connect( editor, SIGNAL( modifyRelation( Relation*, int ) ), SLOT( slotModifyRelation( Relation*, int ) ) );
    connect( editor, SIGNAL( modifyRelation( Relation* ) ), SLOT( slotModifyRelation( Relation* ) ) );

    connect( editor, SIGNAL( editNode( Node * ) ), SLOT( slotOpenNode( Node * ) ) );
    connect( editor, SIGNAL( addTask() ), SLOT( slotAddTask() ) );
    connect( editor, SIGNAL( addMilestone() ), SLOT( slotAddMilestone() ) );
    connect( editor, SIGNAL( addSubtask() ), SLOT( slotAddSubTask() ) );
    connect( editor, SIGNAL( deleteTaskList( QList<Node*> ) ), SLOT( slotDeleteTask( QList<Node*> ) ) );

    connect( editor, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    editor->updateReadWrite( m_readWrite );
    return editor;
}

ViewBase *View::createPertEditor( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    PertEditor *perteditor = new PertEditor( getPart(), m_tab );
    m_tab->addWidget( perteditor );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, perteditor, getPart(), "task_editor" );
    i->setToolTip( 0, tip );

    perteditor->draw( getProject() );

    connect( perteditor, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
    m_updatePertEditor = true;
    perteditor->updateReadWrite( m_readWrite );
    return perteditor;
}

ViewBase *View::createTaskStatusView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    TaskStatusView *taskstatusview = new TaskStatusView( getPart(), m_tab );
    m_tab->addWidget( taskstatusview );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, taskstatusview, getPart(), "status_view" );
    i->setToolTip( 0, tip );

    connect( taskstatusview, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), taskstatusview, SLOT( slotCurrentScheduleManagerChanged( ScheduleManager* ) ) );
    
    connect( taskstatusview, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    taskstatusview->updateReadWrite( m_readWrite );
    return taskstatusview;
}

ViewBase *View::createGanttView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    GanttView *ganttview = new GanttView( getPart(), m_tab, getPart()->isReadWrite() );
    m_tab->addWidget( ganttview );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, ganttview, getPart(), "gantt_chart" );
    i->setToolTip( 0, tip );

    ganttview->setProject( &( getProject() ) );
    ganttview->setScheduleManager( currentScheduleManager() );

    connect( ganttview, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
/*  TODO: Review these
    connect( ganttview, SIGNAL( addRelation( Node*, Node*, int ) ), SLOT( slotAddRelation( Node*, Node*, int ) ) );
    connect( ganttview, SIGNAL( modifyRelation( Relation*, int ) ), SLOT( slotModifyRelation( Relation*, int ) ) );
    connect( ganttview, SIGNAL( modifyRelation( Relation* ) ), SLOT( slotModifyRelation( Relation* ) ) );
    connect( ganttview, SIGNAL( itemDoubleClicked() ), SLOT( slotOpenNode() ) );
    connect( ganttview, SIGNAL( itemRenamed( Node*, const QString& ) ), this, SLOT( slotRenameNode( Node*, const QString& ) ) );*/
    
    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), ganttview, SLOT( setScheduleManager( ScheduleManager* ) ) );
    
    connect( ganttview, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    ganttview->updateReadWrite( m_readWrite );
    return ganttview;
}

ViewBase *View::createMilestoneGanttView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    MilestoneGanttView *ganttview = new MilestoneGanttView( getPart(), m_tab, getPart()->isReadWrite() );
    m_tab->addWidget( ganttview );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, ganttview, getPart(), "gantt_chart" );
    i->setToolTip( 0, tip );

    ganttview->setProject( &( getProject() ) );
    ganttview->setScheduleManager( currentScheduleManager() );

    connect( ganttview, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
    
    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), ganttview, SLOT( setScheduleManager( ScheduleManager* ) ) );
    
    connect( ganttview, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    ganttview->updateReadWrite( m_readWrite );
    return ganttview;
}


ViewBase *View::createAccountsView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    AccountsView *accountsview = new AccountsView( &getProject(), getPart(), m_tab );
    m_updateAccountsview = true;
    m_tab->addWidget( accountsview );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, accountsview, getPart(), "accounts" );
    i->setToolTip( 0, tip );

    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), accountsview, SLOT( setScheduleManager( ScheduleManager* ) ) );
    
    connect( accountsview, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
    accountsview->updateReadWrite( m_readWrite );
    return accountsview;
}

ViewBase *View::createResourceAssignmentView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ResourceAssignmentView *resourceAssignmentView = new ResourceAssignmentView( getPart(), m_tab );
    m_tab->addWidget( resourceAssignmentView );
    m_updateResourceAssignmentView = true;

    ViewListItem *i = m_viewlist->addView( cat, tag, name, resourceAssignmentView, getPart(), "resource_assignment" );
    i->setToolTip( 0, tip );

    resourceAssignmentView->draw( getProject() );

    connect( resourceAssignmentView, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );

    connect( resourceAssignmentView, SIGNAL( requestPopupMenu( const QString&, const QPoint & ) ), this, SLOT( slotPopupMenu( const QString&, const QPoint& ) ) );
    resourceAssignmentView->updateReadWrite( m_readWrite );
    return resourceAssignmentView;
}

ViewBase *View::createChartView( ViewListItem *cat, const QString tag, const QString &name, const QString &tip )
{
    ChartView *v = new ChartView( getPart(), m_tab );
    m_tab->addWidget( v );

    ViewListItem *i = m_viewlist->addView( cat, tag, name, v, getPart(), "chart" );
    i->setToolTip( 0, tip );

    v->setProject( &( getProject() ) );
    
    connect( this, SIGNAL( currentScheduleManagerChanged( ScheduleManager* ) ), v, SLOT( setScheduleManager( ScheduleManager* ) ) );

    connect( v, SIGNAL( guiActivated( ViewBase*, bool ) ), SLOT( slotGuiActivated( ViewBase*, bool ) ) );
    v->updateReadWrite( m_readWrite );
    return v;

}

Project& View::getProject() const
{
    return getPart() ->getProject();
}

void View::setZoom( double )
{
    //TODO
}

void View::setupPrinter( QPrinter &printer, QPrintDialog &printDialog )
{
    //kDebug();
}

void View::print( QPrinter &printer, QPrintDialog &printDialog )
{
    //kDebug();
/*    if ( printer.previewOnly() ) {
        //HACK: KoMainWindow shows setup on print, but not on print preview!
        if ( !printer.setup() ) {
            return ;
        }
    }*/
/* TODO
    if ( m_tab->currentWidget() == ganttview ) {
        ganttview->print( printer );
    } else if ( m_tab->currentWidget() == m_resourceview ) {
        m_resourceview->print( printer );
    } else if ( m_tab->currentWidget() == m_accountsview ) {
        m_accountsview->print( printer );
    }*/
    // 	else if (m_tab->currentWidget() == m_reportview)
    // 	{
    //         m_reportview->print(printer);
    // 	}

}

void View::slotEditCut()
{
    //kDebug();
}

void View::slotEditCopy()
{
    //kDebug();
}

void View::slotEditPaste()
{
    //kDebug();
}

void View::slotViewSelector( bool show )
{
    //kDebug();
    m_viewlist->setVisible( show );
}


void View::slotProjectEdit()
{
    MainProjectDialog * dia = new MainProjectDialog( getProject() );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            getPart() ->addCommand( cmd );
        }
    }
    delete dia;
}

void View::slotEditCalendar()
{
    slotEditCalendar( currentCalendar() );
}

void View::slotEditCalendar( Calendar *calendar )
{
    if ( calendar == 0 ) {
        return;
    }
    CalendarEditDialog * dia = new CalendarEditDialog( getProject(), calendar );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            //kDebug()<<"Modifying calendar";
            getPart() ->addCommand( cmd ); //also executes
        }
    }
    delete dia;
}

void View::slotProjectCalendar()
{
    CalendarListDialog * dia = new CalendarListDialog( getProject() );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            //kDebug()<<"Modifying calendar(s)";
            getPart() ->addCommand( cmd ); //also executes
        }
    }
    delete dia;
}

void View::slotProjectAccounts()
{
    AccountsDialog * dia = new AccountsDialog( getProject(), getProject().accounts() );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            //kDebug()<<"Modifying account(s)";
            getPart() ->addCommand( cmd ); //also executes
        }
    }
    delete dia;
}

void View::slotProjectWorktime()
{
    StandardWorktimeDialog * dia = new StandardWorktimeDialog( getProject() );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            //kDebug()<<"Modifying calendar(s)";
            getPart() ->addCommand( cmd ); //also executes
        }
    }
    delete dia;
}

void View::slotProjectResources()
{
    ResourcesDialog * dia = new ResourcesDialog( getProject() );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            //kDebug()<<"Modifying resources";
            getPart() ->addCommand( cmd ); //also executes
        }
    }
    delete dia;
}

void View::slotScheduleRemoved( const MainSchedule *sch )
{
    QAction *a = 0;
    QAction *checked = m_scheduleActionGroup->checkedAction();
    QMapIterator<QAction*, Schedule*> i( m_scheduleActions );
    while (i.hasNext()) {
        i.next();
        if ( i.value() == sch ) {
            a = i.key();
            break;
        }
    }
    if ( a ) {
        unplugActionList( "view_schedule_list" );
        delete a;
        plugActionList( "view_schedule_list", m_scheduleActions.keys() );
        if ( checked && checked != a ) {
            checked->setChecked( true );
        } else if ( ! m_scheduleActions.isEmpty() ) {
            m_scheduleActions.keys().first()->setChecked( true );
        }
    }
    slotViewSchedule( m_scheduleActionGroup->checkedAction() );
}

void View::slotScheduleAdded( const MainSchedule *sch )
{
    if ( sch->type() != Schedule::Expected ) {
        return; // Only view expected
    }
    MainSchedule *s = const_cast<MainSchedule*>( sch ); // FIXME
    //kDebug()<<sch->name()<<" deleted="<<sch->isDeleted();
    QAction *checked = m_scheduleActionGroup->checkedAction();
    if ( ! sch->isDeleted() && sch->isScheduled() ) {
        unplugActionList( "view_schedule_list" );
        QAction *act = addScheduleAction( s );
        plugActionList( "view_schedule_list", m_scheduleActions.keys() );
        if ( checked ) {
            checked->setChecked( true );
        } else if ( act ) {
            act->setChecked( true );
        } else if ( ! m_scheduleActions.isEmpty() ) {
            m_scheduleActions.keys().first()->setChecked( true );
        }
    }
    slotViewSchedule( m_scheduleActionGroup->checkedAction() );
}

void View::slotScheduleChanged( MainSchedule *sch )
{
    //kDebug()<<sch->name()<<" deleted="<<sch->isDeleted();
    if ( sch->isDeleted() || ! sch->isScheduled() ) {
        slotScheduleRemoved( sch );
        return;
    }
    if ( m_scheduleActions.values().contains( sch ) ) {
        slotScheduleRemoved( sch ); // hmmm, how to avoid this?
    }
    slotScheduleAdded( sch );
}

QAction *View::addScheduleAction( Schedule *sch )
{
    QAction *act = 0;
    if ( ! sch->isDeleted() ) {
        QString n = sch->name();
        QAction *act = new KToggleAction( n, this);
        actionCollection()->addAction(n, act );
        m_scheduleActions.insert( act, sch );
        m_scheduleActionGroup->addAction( act );
        //kDebug()<<"Add:"<<n;
        connect( act, SIGNAL(destroyed( QObject* ) ), SLOT( slotActionDestroyed( QObject* ) ) );
    }
    return act;
}

void View::slotViewSchedule( QAction *act )
{
    //kDebug();
    if ( act != 0 ) {
        Schedule *sch = m_scheduleActions.value( act, 0 );
        m_manager = sch->manager();
    } else {
        m_manager = 0;
    }
    kDebug()<<m_manager<<endl;
    setLabel();
    emit currentScheduleManagerChanged( m_manager );
}

void View::slotActionDestroyed( QObject *o )
{
    //kDebug()<<o->name();
    m_scheduleActions.remove( static_cast<QAction*>( o ) );
    slotViewSchedule( m_scheduleActionGroup->checkedAction() );
}

void View::slotPlugScheduleActions()
{
    //kDebug();
    unplugActionList( "view_schedule_list" );
    foreach( QAction *act, m_scheduleActions.keys() ) {
        m_scheduleActionGroup->removeAction( act );
        delete act;
    }
    m_scheduleActions.clear();
    Schedule *cs = getProject().currentSchedule();
    QAction *ca = 0;
    foreach( ScheduleManager *sm, getProject().allScheduleManagers() ) {
        Schedule *sch = sm->expected();
        if ( sch == 0 ) {
            continue;
        }
        QAction *act = addScheduleAction( sch );
        if ( act ) {
            if ( ca == 0 && cs == sch ) {
                ca = act;
            }
        }
    }
    plugActionList( "view_schedule_list", m_scheduleActions.keys() );
    if ( ca == 0 && m_scheduleActionGroup->actions().count() > 0 ) {
        ca = m_scheduleActionGroup->actions().first();
    }
    if ( ca ) {
        ca->setChecked( true );
    }
    slotViewSchedule( ca );
}

void View::slotProgressChanged( int )
{
}

void View::slotCalculateSchedule( Project *project, ScheduleManager *sm )
{
    if ( project == 0 || sm == 0 ) {
        return;
    }
    statusBar()->showMessage( i18n( "%1: Calculating...", sm->name() ) );
//    connect( project, SIGNAL( sigProgress( int ) ), SLOT(slotProgressChanged( int ) ) );
    QApplication::setOverrideCursor( Qt::WaitCursor );
    CalculateScheduleCmd *cmd =  new CalculateScheduleCmd( *project, *sm, i18n( "Calculate %1", sm->name() ) );
    getPart() ->addCommand( cmd );
    QApplication::restoreOverrideCursor();
    statusBar()->clearMessage();
    statusBar()->showMessage( i18n( "%1: Calculating done", sm->name() ), 2000 );
    slotUpdate();
}

void View::slotAddScheduleManager( Project *project )
{
    if ( project == 0 ) {
        return;
    }
    ScheduleManager *sm = project->createScheduleManager();
    AddScheduleManagerCmd *cmd =  new AddScheduleManagerCmd( *project, sm, i18n( "Add Schedule %1", sm->name() ) );
    getPart() ->addCommand( cmd );
}

void View::slotDeleteScheduleManager( Project *project, ScheduleManager *sm )
{
    if ( project == 0 || sm == 0) {
        return;
    }
    DeleteScheduleManagerCmd *cmd =  new DeleteScheduleManagerCmd( *project, sm, i18n( "Delete Schedule %1", sm->name() ) );
    getPart() ->addCommand( cmd );
}

void View::slotViewReportDesign()
{
    //kDebug();
}

void View::slotViewReports()
{
    //kDebug();
    //m_tab->setCurrentWidget(m_reportview);
}

void View::slotAddSubTask()
{
    // If we are positionend on the root project, then what we really want to
    // do is to add a first project. We will silently accept the challenge
    // and will not complain.
    Task * node = getProject().createTask( getPart() ->config().taskDefaults(), currentTask() );
    TaskDialog *dia = new TaskDialog( *node, getProject().accounts(), getProject().standardWorktime() );
    if ( dia->exec()  == QDialog::Accepted) {
        Node * currNode = currentTask();
        if ( currNode ) {
            QUndoCommand *m = dia->buildCommand();
            m->redo(); // do changes to task
            delete m;
            SubtaskAddCmd *cmd = new SubtaskAddCmd( &( getProject() ), node, currNode, i18n( "Add Subtask" ) );
            getPart() ->addCommand( cmd ); // add task to project
            delete dia;
            return ;
        } else
            kDebug() <<"Cannot insert new project. Hmm, no current node!?";
    }
    delete node;
    delete dia;
}

void View::slotAddTask()
{
    Task * node = getProject().createTask( getPart() ->config().taskDefaults(), currentTask() );
    TaskDialog *dia = new TaskDialog( *node, getProject().accounts(), getProject().standardWorktime() );
    if ( dia->exec()  == QDialog::Accepted) {
        Node * currNode = currentTask();
        if ( currNode ) {
            QUndoCommand * m = dia->buildCommand();
            m->redo(); // do changes to task
            delete m;
            TaskAddCmd *cmd = new TaskAddCmd( &( getProject() ), node, currNode, i18n( "Add Task" ) );
            getPart() ->addCommand( cmd ); // add task to project
            delete dia;
            return ;
        } else
            kDebug() <<"Cannot insert new task. Hmm, no current node!?";
    }
    delete node;
    delete dia;
}

void View::slotAddMilestone()
{
    Task * node = getProject().createTask( currentTask() );
    node->estimate() ->set( Duration::zeroDuration );

    TaskDialog *dia = new TaskDialog( *node, getProject().accounts(), getProject().standardWorktime() );
    if ( dia->exec() == QDialog::Accepted ) {
        Node * currNode = currentTask();
        if ( currNode ) {
            QUndoCommand * m = dia->buildCommand();
            m->redo(); // do changes to task
            delete m;
            TaskAddCmd *cmd = new TaskAddCmd( &( getProject() ), node, currNode, i18n( "Add Milestone" ) );
            getPart() ->addCommand( cmd ); // add task to project
            delete dia;
            return ;
        } else
            kDebug() <<"Cannot insert new milestone. Hmm, no current node!?";
    }
    delete node;
    delete dia;
}

void View::slotDefineWBS()
{
    //kDebug();
    WBSDefinitionDialog * dia = new WBSDefinitionDialog( getPart() ->wbsDefinition() );
    dia->exec();

    delete dia;
}

void View::slotGenerateWBS()
{
    //kDebug();
    getPart() ->generateWBS();
    slotUpdate();
}

void View::slotConfigure()
{
    //kDebug();
    ConfigDialog * dia = new ConfigDialog( getPart() ->config(), getProject() );
    dia->exec();
    delete dia;
}


Calendar *View::currentCalendar()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return 0;
    }
    return v->currentCalendar();
}

Node *View::currentTask()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return 0;
    }
    Node * task = v->currentNode();
    if ( 0 != task ) {
        return task;
    }
    return &( getProject() );
}

Resource *View::currentResource()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return 0;
    }
    return v->currentResource();
}

ResourceGroup *View::currentResourceGroup()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return 0;
    }
    return v->currentResourceGroup();
}


void View::slotOpenNode()
{
    //kDebug();
    Node * node = currentTask();
    slotOpenNode( node );
}

void View::slotOpenNode( Node *node )
{
    //kDebug();
    if ( !node )
        return ;

    switch ( node->type() ) {
        case Node::Type_Project: {
                Project * project = dynamic_cast<Project *>( node );
                MainProjectDialog *dia = new MainProjectDialog( *project );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                delete dia;
                break;
            }
        case Node::Type_Subproject:
            //TODO
            break;
        case Node::Type_Task: {
            kDebug()<<0;
            Task *task = dynamic_cast<Task *>( node );
                Q_ASSERT( task );
                TaskDialog *dia = new TaskDialog( *task, getProject().accounts(), getProject().standardWorktime() );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                kDebug()<<1;
                delete dia;
                kDebug()<<2;
                break;
            }
        case Node::Type_Milestone: {
                // Use the normal task dialog for now.
                // Maybe milestone should have it's own dialog, but we need to be able to
                // enter a duration in case we accidentally set a tasks duration to zero
                // and hence, create a milestone
                Task *task = dynamic_cast<Task *>( node );
                Q_ASSERT( task );
                TaskDialog *dia = new TaskDialog( *task, getProject().accounts(), getProject().standardWorktime() );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                delete dia;
                break;
            }
        case Node::Type_Summarytask: {
                Task *task = dynamic_cast<Task *>( node );
                Q_ASSERT( task );
                SummaryTaskDialog *dia = new SummaryTaskDialog( *task );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                delete dia;
                break;
            }
        default:
            break; // avoid warnings
    }
}

ScheduleManager *View::currentScheduleManager() const
{
/*    Schedule *s = getProject().findSchedule( getProject().currentViewScheduleId() );
    if ( s == 0 ) {*/
        return 0;
/*    }
    return s->manager();*/
}

void View::slotTaskProgress()
{
    //kDebug();
    Node * node = currentTask();
    if ( !node )
        return ;

    switch ( node->type() ) {
        case Node::Type_Project: {
                break;
            }
        case Node::Type_Subproject:
            //TODO
            break;
        case Node::Type_Task: {
                Task *task = dynamic_cast<Task *>( node );
                Q_ASSERT( task );
                TaskProgressDialog *dia = new TaskProgressDialog( *task, currentScheduleManager(),  getProject().standardWorktime() );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                delete dia;
                break;
            }
        case Node::Type_Milestone: {
                Task *task = dynamic_cast<Task *>( node );
                MilestoneProgressDialog *dia = new MilestoneProgressDialog( *task );
                if ( dia->exec()  == QDialog::Accepted) {
                    QUndoCommand * m = dia->buildCommand();
                    if ( m ) {
                        getPart() ->addCommand( m );
                    }
                }
                delete dia;
                break;
            }
        case Node::Type_Summarytask: {
                // TODO
                break;
            }
        default:
            break; // avoid warnings
    }
}

void View::slotDeleteTask( QList<Node*> lst )
{
    //kDebug();
    foreach ( Node *n, lst ) {
        if ( n->isScheduled() ) {
            int res = KMessageBox::warningContinueCancel( this, i18n( "A task that has been scheduled will be deleted. This will invalidate the schedule." ) );
            if ( res == KMessageBox::Cancel ) {
                return;
            }
            break;
        }
    }
    if ( lst.count() == 1 ) {
        getPart()->addCommand( new NodeDeleteCmd( lst.takeFirst(), i18n( "Delete Task" ) ) );
        return;
    }
    int num = 0;
    MacroCommand *cmd = new MacroCommand( i18n( "Delete Tasks" ) );
    while ( !lst.isEmpty() ) {
        Node *node = lst.takeFirst();
        if ( node == 0 || node->parentNode() == 0 ) {
            kDebug() << ( node ?"Task is main project" :"No current task" );
            continue;
        }
        bool del = true;
        foreach ( Node *n, lst ) {
            if ( node->isChildOf( n ) ) {
                del = false; // node is going to be deleted when we delete n
                break;
            }
        }
        if ( del ) {
            //kDebug()<<num<<": delete:"<<node->name();
            cmd->addCommand( new NodeDeleteCmd( node, i18n( "Delete Task" ) ) );
            num++;
        }
    }
    if ( num > 0 ) {
        getPart()->addCommand( cmd );
    } else {
        delete cmd;
    }
}

void View::slotDeleteTask( Node *node )
{
    //kDebug();
    if ( node == 0 || node->parentNode() == 0 ) {
        kDebug() << ( node ?"Task is main project" :"No current task" );
        return ;
    }
    if ( node->isScheduled() ) {
        int res = KMessageBox::warningContinueCancel( this, i18n( "This task has been scheduled. This will invalidate the schedule." ) );
        if ( res == KMessageBox::Cancel ) {
            return;
        }
    }
    NodeDeleteCmd *cmd = new NodeDeleteCmd( node, i18n( "Delete Task" ) );
    getPart() ->addCommand( cmd );
}

void View::slotDeleteTask()
{
    //kDebug();
    return slotDeleteTask( currentTask() );
}

void View::slotIndentTask()
{
    //kDebug();
    Node * node = currentTask();
    if ( node == 0 || node->parentNode() == 0 ) {
        kDebug() << ( node ?"Task is main project" :"No current task" );
        return ;
    }
    if ( getProject().canIndentTask( node ) ) {
        NodeIndentCmd * cmd = new NodeIndentCmd( *node, i18n( "Indent Task" ) );
        getPart() ->addCommand( cmd );
    }
}

void View::slotUnindentTask()
{
    //kDebug();
    Node * node = currentTask();
    if ( node == 0 || node->parentNode() == 0 ) {
        kDebug() << ( node ?"Task is main project" :"No current task" );
        return ;
    }
    if ( getProject().canUnindentTask( node ) ) {
        NodeUnindentCmd * cmd = new NodeUnindentCmd( *node, i18n( "Unindent Task" ) );
        getPart() ->addCommand( cmd );
    }
}

void View::slotMoveTaskUp()
{
    //kDebug();

    Node * task = currentTask();
    if ( 0 == task ) {
        // is always != 0. At least we would get the Project, but you never know who might change that
        // so better be careful
        kError() << "No current task" << endl;
        return ;
    }

    if ( Node::Type_Project == task->type() ) {
        kDebug() <<"The root node cannot be moved up";
        return ;
    }
    if ( getProject().canMoveTaskUp( task ) ) {
        NodeMoveUpCmd * cmd = new NodeMoveUpCmd( *task, i18n( "Move Task Up" ) );
        getPart() ->addCommand( cmd );
    }
}

void View::slotMoveTaskDown()
{
    //kDebug();

    Node * task = currentTask();
    if ( 0 == task ) {
        // is always != 0. At least we would get the Project, but you never know who might change that
        // so better be careful
        return ;
    }

    if ( Node::Type_Project == task->type() ) {
        kDebug() <<"The root node cannot be moved down";
        return ;
    }
    if ( getProject().canMoveTaskDown( task ) ) {
        NodeMoveDownCmd * cmd = new NodeMoveDownCmd( *task, i18n( "Move Task Down" ) );
        getPart() ->addCommand( cmd );
    }
}

void View::slotAddRelation( Node *par, Node *child )
{
    //kDebug();
    Relation * rel = new Relation( par, child );
    AddRelationDialog *dia = new AddRelationDialog( getProject(), rel, this );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd )
            getPart() ->addCommand( cmd );
    } else {
        delete rel;
    }
    delete dia;
}

void View::slotAddRelation( Node *par, Node *child, int linkType )
{
    //kDebug();
    if ( linkType == Relation::FinishStart ||
            linkType == Relation::StartStart ||
            linkType == Relation::FinishFinish ) {
        Relation * rel = new Relation( par, child, static_cast<Relation::Type>( linkType ) );
        getPart() ->addCommand( new AddRelationCmd( getProject(), rel, i18n( "Add Relation" ) ) );
    } else {
        slotAddRelation( par, child );
    }
}

void View::slotModifyRelation( Relation *rel )
{
    //kDebug();
    ModifyRelationDialog * dia = new ModifyRelationDialog( getProject(), rel, this );
    if ( dia->exec()  == QDialog::Accepted) {
        if ( dia->relationIsDeleted() ) {
            getPart() ->addCommand( new DeleteRelationCmd( getProject(), rel, i18n( "Delete Relation" ) ) );
        } else {
            QUndoCommand *cmd = dia->buildCommand();
            if ( cmd ) {
                getPart() ->addCommand( cmd );
            }
        }
    }
    delete dia;
}

void View::slotModifyRelation( Relation *rel, int linkType )
{
    //kDebug();
    if ( linkType == Relation::FinishStart ||
            linkType == Relation::StartStart ||
            linkType == Relation::FinishFinish ) {
        getPart() ->addCommand( new ModifyRelationTypeCmd( rel, static_cast<Relation::Type>( linkType ) ) );
    } else {
        slotModifyRelation( rel );
    }
}

void View::slotModifyRelation()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return;
    }
    Relation *rel = v->currentRelation();
    if ( rel ) {
        slotModifyRelation( rel );
    }
}

void View::slotDeleteRelation()
{
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v == 0 ) {
        return;
    }
    Relation *rel = v->currentRelation();
    if ( rel ) {
        getPart()->addCommand( new DeleteRelationCmd( getProject(), rel, i18n( "Delete Task Dependency" ) ) );
    }
}

void View::slotAddResource( ResourceGroup *group )
{
    //kDebug();
    if ( group == 0 ) {
        return;
    }
    Resource *r = new Resource();
    ResourceDialog *dia = new ResourceDialog( getProject(), r );
    if ( dia->exec()  == QDialog::Accepted) {
        MacroCommand *m = new MacroCommand( i18n( "Add resource" ) );
        m->addCommand( new AddResourceCmd( group, r ) );
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd ) {
            m->addCommand( cmd );
        }
        getPart()->addCommand( m );
        delete dia;
        return;
    }
    delete r;
    delete dia;
}

void View::slotEditResource()
{
    //kDebug();
    Resource * r = currentResource();
    if ( r == 0 ) {
        return ;
    }
    ResourceDialog *dia = new ResourceDialog( getProject(), r );
    if ( dia->exec()  == QDialog::Accepted) {
        QUndoCommand * cmd = dia->buildCommand();
        if ( cmd )
            getPart() ->addCommand( cmd );
    }
    delete dia;
}

void View::slotDeleteResource( Resource *resource )
{
    getPart()->addCommand( new RemoveResourceCmd( resource->parentGroup(), resource, i18n( "Delete Resource" ) ) );
}

void View::slotDeleteResourceGroup( ResourceGroup *group )
{
    getPart()->addCommand( new RemoveResourceGroupCmd( group->project(), group, i18n( "Delete Resourcegroup" ) ) );
}

void View::slotDeleteResourceObjects( QObjectList lst )
{
    //kDebug();
    foreach ( QObject *o, lst ) {
        Resource *r = qobject_cast<Resource*>( o );
        if ( r && r->isScheduled() ) {
            int res = KMessageBox::warningContinueCancel( this, i18n( "A resource that has been scheduled will be deleted. This will invalidate the schedule." ) );
            if ( res == KMessageBox::Cancel ) {
                return;
            }
            break;
        }
        ResourceGroup *g = qobject_cast<ResourceGroup*>( o );
        if ( g && g->isScheduled() ) {
            int res = KMessageBox::warningContinueCancel( this, i18n( "A resource that has been scheduled will be deleted. This will invalidate the schedule." ) );
            if ( res == KMessageBox::Cancel ) {
                return;
            }
            break;
        }
    }
    if ( lst.count() == 1 ) {
        Resource *r = qobject_cast<Resource*>( lst.first() );
        if ( r ) {
            slotDeleteResource( r );
        } else {
            ResourceGroup *g = qobject_cast<ResourceGroup*>( lst.first() );
            if ( g ) {
                slotDeleteResourceGroup( g );
            }
        }
        return;
    }
    int num = 0;
    MacroCommand *cmd = 0, *rc = 0, *gc = 0;
    foreach ( QObject *o, lst ) {
        Resource *r = qobject_cast<Resource*>( o );
        if ( r ) {
            if ( rc == 0 )  rc = new MacroCommand( "" );
            rc->addCommand( new RemoveResourceCmd( r->parentGroup(), r ) );
            continue;
        }
        ResourceGroup *g = qobject_cast<ResourceGroup*>( o );
        if ( g ) {
            if ( gc == 0 )  gc = new MacroCommand( "" );
            gc->addCommand( new RemoveResourceGroupCmd( g->project(), g ) );
        }
    }
    if ( rc || gc ) {
        cmd = new MacroCommand( i18n( "Delete Resource Objects" ) );
    }
    if ( rc )
        cmd->addCommand( rc );
    if ( gc )
        cmd->addCommand( gc );
    if ( cmd )
        getPart()->addCommand( cmd );
}


void View::updateReadWrite( bool readwrite )
{
    m_readWrite = readwrite;
    m_viewlist->setReadWrite( readwrite );
}

Part *View::getPart() const
{
    return ( Part * ) koDocument();
}

void View::slotConnectNode()
{
    //kDebug();
    /*    NodeItem *curr = ganttview->currentItem();
        if (curr) {
            kDebug()<<"node="<<curr->getNode().name();
        }*/
}

QMenu * View::popupMenu( const QString& name )
{
    //kDebug();
    Q_ASSERT( factory() );
    if ( factory() )
        return ( ( QMenu* ) factory() ->container( name, this ) );
    return 0L;
}

void View::slotUpdate()
{
    //kDebug()<<"calculate="<<calculate;

//    m_updateResourceview = true;
    m_updateResourceAssignmentView = true;
    m_updatePertEditor = true;
    updateView( m_tab->currentWidget() );
}

void View::slotGuiActivated( ViewBase *view, bool activate )
{
    if ( activate ) {
        foreach( QString name, view->actionListNames() ) {
            //kDebug()<<"activate"<<name<<","<<view->actionList( name ).count();
            plugActionList( name, view->actionList( name ) );
        }
    } else {
        foreach( QString name, view->actionListNames() ) {
            //kDebug()<<"deactivate"<<name;
            unplugActionList( name );
        }
    }
}

void View::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    //kDebug()<<ev->activated();
    KoView::guiActivateEvent( ev );
    if ( ev->activated() ) {
        // plug my own actionlists, they may be gone
        slotPlugScheduleActions();
    }
    // propagate to sub-view
    ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
    if ( v ) {
        v->setGuiActive( ev->activated() );
    }
}

KoDocument *View::hitTest( const QPoint &pos )
{
    // TODO: The gui handling can certainly be simplified (at least I think so),
    // by someone who have a better understanding of all the possibilities of KParts
    // than I have.
    kDebug()<<pos;
    // pos is in m_tab->currentWidget() coordinates
    QPoint gl = m_tab->currentWidget()->mapToGlobal(pos);
    if ( m_tab->currentWidget()->frameGeometry().contains( m_tab->currentWidget()->mapFromGlobal( gl ) ) ) {
        if ( koDocument() == dynamic_cast<KoDocument*>(partManager()->activePart() ) ) {
            // just activating new view on the same doc
            SplitterView *sp = dynamic_cast<SplitterView*>( m_tab->currentWidget() );
            if ( sp ) {
                // Check which view has actually been hit (can aslo be the splitter)
                ViewBase *v = sp->findView( pos );
                if ( v ) {
                    kDebug()<<"Hit on:"<<v;
                    v->setGuiActive( true );
                }
            }
        }
        return koDocument()->hitTest( pos, this );
    }
    // get a 0 based geometry
    QRect r = m_viewlist->frameGeometry();
    r.translate( -r.topLeft() );
    if ( r.contains( m_viewlist->mapFromGlobal( gl ) ) ) {
        if ( getPart()->isEmbedded() ) {
            // TODO: Needs testing
            return dynamic_cast<KoDocument*>(partManager()->activePart()); // NOTE: We only handle koffice parts!
        }
        return 0;
    }
    for (int i = 0; i < m_sp->count(); ++i ) {
        QWidget *w = m_sp->handle( i );
        r = w->frameGeometry();
        r.translate( -r.topLeft() );
        if ( r.contains( w->mapFromGlobal( gl ) ) ) {
            if ( getPart()->isEmbedded() ) {
            // TODO: Needs testing
                return dynamic_cast<KoDocument*>(partManager()->activePart()); // NOTE: We only handle koffice parts!
            }
            return 0;
        }
    }
    kDebug()<<"No hit:"<<pos;
    return 0;

}

void View::createChildDocumentViews()
{
    foreach ( KoDocumentChild *ch, getPart()->children() ) {
        if ( ! ch->isDeleted() ) {
            DocumentChild *c = static_cast<DocumentChild*>( ch );
            QTreeWidgetItem *cat = m_viewlist->findItem( c->category(), 0 );
            if ( cat == 0 ) {
                kDebug()<<"New category: Documents"<<endl;
                cat = m_viewlist->addCategory( "Documents", i18n( "Documents" ) );
                cat->setIcon( 0, KIcon( "koshell" ) );
            }
            ViewListItem *i = createChildDocumentView( c );
            cat->insertChild( cat->childCount(), i );
        }
    }
}

ViewListItem *View::createChildDocumentView( DocumentChild *ch )
{
    kDebug()<<ch->title()<<endl;
    KoDocument *doc = ch->document();

    QString title = ch->title();
    if ( title.isEmpty() && doc->documentInfo() ) {
        title = doc->documentInfo()->aboutInfo( "title" );
    }
    if ( title.isEmpty() ) {
        title = doc->url().pathOrUrl();
    }
    if ( title.isEmpty() ) {
        title = "Untitled";
    }
    KoView *v = doc->createView( this );
    ch->setGeometry( geometry(), true );
    m_tab->addWidget( v );
    ViewListItem *i = m_viewlist->createChildDocumentView( doc->objectName(), title, v, ch );
    if ( ! ch->icon().isEmpty() ) {
        i->setIcon( 0, KIcon( ch->icon() ) );
    }
    return i;
}

void View::slotViewListItemRemoved( ViewListItem *item )
{
    if ( item->documentChild() ) {
        // This restores basic ui
        item->documentChild()->setActivated( false, this );
    }
    m_tab->removeWidget( item->view() );
    if ( item->type() == ViewListItem::ItemType_SubView ) {
        delete item->view();
        delete item;
    }
}

void View::slotViewListItemInserted( ViewListItem *item )
{
    m_tab->addWidget( item->view() );
}

void View::slotCreateView()
{
    ViewListDialog dlg( this, *m_viewlist );
    dlg.exec();
}

void View::slotCreateKofficeDocument( KoDocumentEntry &entry)
{
    QString e;
    KoDocument *doc = entry.createDoc( &e, getPart() );
    if ( doc == 0 ) {
        return;
    }
    if ( ! doc->showEmbedInitDialog( this ) ) {
        delete doc;
        return;
    }
    QTreeWidgetItem *cat = m_viewlist->addCategory( "Documents", i18n( "Documents" ) );
    cat->setIcon( 0, KIcon( "koshell" ) );

    DocumentChild *ch = getPart()->createChild( doc );
    ch->setIcon( entry.service()->icon() );
    ViewListItem *i = createChildDocumentView( ch );
    getPart()->addCommand( new InsertEmbeddedDocumentCmd( m_viewlist, i, cat, i18n( "Insert Document" ) ) );
    m_viewlist->setSelected( i );
}

void View::slotViewActivated( ViewListItem *item, ViewListItem *prev )
{
    //kDebug() <<"item=" << item <<","<<prev;
    if ( prev && prev->type() != ViewListItem::ItemType_ChildDocument ) {
        // Remove sub-view specific gui
        //kDebug()<<"Deactivate:"<<prev;
        ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
        if ( v ) {
            v->setGuiActive( false );
        }
    }
    if ( item->type() == ViewListItem::ItemType_SubView ) {
        //kDebug()<<"Activate:"<<item;
        m_tab->setCurrentWidget( item->view() );
        if (  prev && prev->type() != ViewListItem::ItemType_SubView ) {
            // Put back my own gui (removed when (if) viewing different doc)
            getPart()->activate( this );
        }
        // Add sub-view specific gui
        ViewBase *v = dynamic_cast<ViewBase*>( m_tab->currentWidget() );
        if ( v ) {
            v->setGuiActive( true );
        }
        return;
    }
    if ( item->type() == ViewListItem::ItemType_ChildDocument ) {
        //kDebug()<<"Activated:"<<item->view();
        // changing doc also takes care of all gui
        m_tab->setCurrentWidget( item->view() );
        item->documentChild()->setActivated( true, item->view() );
        return;
    }
}

QWidget *View::canvas() const
{
    return m_tab->currentWidget();//KoView::canvas();
}

void View::slotCurrentChanged( int )
{
    kDebug()<<m_tab->currentIndex();
    ViewListItem *item = m_viewlist->findItem( m_tab->currentWidget() );
    if ( item == 0 ) {
        return;
    }
    kDebug()<<item->text(0);
    item->setSelected( true );
}

void View::updateView( QWidget * )
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
    //setScheduleActionsEnabled();

    QWidget *widget2;

    widget2 = m_viewlist->findView( "AccountsView" );
    if ( m_updateAccountsview )
        static_cast<ViewBase*>( widget2 ) ->draw();
    m_updateAccountsview = false;

    widget2 = m_viewlist->findView( "ResourceAssignmentView" );
    if ( m_updateResourceAssignmentView )
        static_cast<ViewBase*>( widget2 ) ->draw( getProject() );
    m_updateResourceAssignmentView = false;

    widget2 = m_viewlist->findView( "PertEditor" );
        static_cast<ViewBase*>( widget2 ) -> draw( getProject() );

    QApplication::restoreOverrideCursor();
}

void View::slotRenameNode( Node *node, const QString& name )
{
    //kDebug()<<name;
    if ( node ) {
        NodeModifyNameCmd * cmd = new NodeModifyNameCmd( *node, name, i18n( "Modify Name" ) );
        getPart() ->addCommand( cmd );
    }
}

void View::slotPopupMenu( const QString& menuname, const QPoint & pos )
{
    QMenu * menu = this->popupMenu( menuname );
    if ( menu ) {
        //kDebug()<<menu<<":"<<menu->actions().count();
        menu->exec( pos );
    }
}

void View::slotPopupMenu( const QString& menuname, const QPoint &pos, ViewListItem *item )
{
    //kDebug()<<menuname;
    m_viewlistItem = item;
    slotPopupMenu( menuname, pos );
}

bool View::loadContext()
{
    //kDebug()<<endl;
    Context *ctx = getPart()->context();
    if ( ctx == 0 || ! ctx->isLoaded() ) {
        return true;
    }
    KoXmlNode n = ctx->context().namedItem( "current-view" );
    if ( n.isNull() || ! n.isElement() ) {
        kWarning()<<"No current view"<<endl;
    } else {
        QString cv = n.toElement().attribute( "current-view" );
        if ( ! cv.isEmpty() ) {
            m_viewlist->setSelected( m_viewlist->findItem( cv ) );
        }
    }

    n = ctx->context().namedItem( "current-schedule" );
    if ( n.isNull() || ! n.isElement() ) {
        kWarning()<<"No current schedule"<<endl;
    } else {
        QString id = n.toElement().attribute( "current-schedule", "" );
        if ( ! id.isEmpty() ) {
            kDebug()<<"current-schedule="<<id<<endl;
        }
    }
    
//    slotUpdate();
    return true;
}

void View::saveContext( QDomElement &me ) const
{
    //kDebug()<<endl;
    if ( currentScheduleManager() ) {
        me.setAttribute( "current-schedule", currentScheduleManager()->name() );
    }
    ViewListItem *item = m_viewlist->findItem( m_tab->currentWidget() );
    if ( item ) {
        me.setAttribute("current-view", item->tag() );
        kDebug()<<"Context currentview: "<<item->text( 0 )<<endl;
    }
    m_viewlist->save( me );
}

void View::setLabel()
{
    //kDebug();
    Schedule *s = m_manager == 0 ? 0 : m_manager->expected();
    if ( s && !s->isDeleted() && s->isScheduled() ) {
        m_estlabel->setText( m_manager->name() );
        return;
    }
    m_estlabel->setText( i18n( "Not scheduled" ) );
}

#ifndef NDEBUG
void View::slotPrintDebug()
{
    kDebug() <<"-------- Debug printout: Node list";
    /*    Node *curr = ganttview->currentNode();
        if (curr) {
            curr->printDebug(true,"");
        } else*/
    getPart() ->getProject().printDebug( true, "" );
}
void View::slotPrintSelectedDebug()
{
/*TODO
    if ( m_tab->currentWidget() == ganttview ) {
        Node * curr = ganttview->currentNode();
        if ( curr ) {
            kDebug() <<"-------- Debug printout: Selected node";
            curr->printDebug( true, "" );
        } else
            slotPrintDebug();
        return;
    } else if ( m_tab->currentWidget() == m_viewlist->findView( "ResourceEditor" ) ) {
        Resource *r = static_cast<ViewBase*>( m_tab->currentWidget() )->currentResource();
        if ( r ) {
            kDebug() <<"-------- Debug printout: Selected resource";
            r->printDebug("  !");
            return;
        }
        ResourceGroup *g = static_cast<ViewBase*>( m_tab->currentWidget() )->currentResourceGroup();
        if ( g ) {
            kDebug() <<"-------- Debug printout: Selected group";
            g->printDebug("  !");
            return;
        }
    }
    slotPrintDebug();*/
}
void View::slotPrintCalendarDebug()
{
    //kDebug() <<"-------- Debug printout: Calendars";
    /*    Node *curr = ganttview->currentNode();
        if (curr) {
            curr->printDebug(true,"");
        } else*/
    getPart() ->getProject().printCalendarDebug( "" );
}
void View::slotPrintTestDebug()
{
    const QStringList & lst = getPart() ->xmlLoader().log();

    for ( QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
        kDebug() << *it;
    }
    //     kDebug()<<"------------Test 1---------------------";
    //     {
    //     DateTime d1(QDate(2006,1,2), QTime(8,0,0));
    //     DateTime d2 = d1.addSecs(3600);
    //     Duration d = d2 - d1;
    //     bool b = d==Duration(0,0,0,3600);
    //     kDebug()<<"1: Success="<<b<<""<<d2.toString()<<"-"<<d1.toString()<<"="<<d.toString();
    //     d = d1 - d2;
    //     b = d==Duration(0,0,0,3600);
    //     kDebug()<<"2: Success="<<b<<""<<d1.toString()<<"-"<<d2.toString()<<"="<<d.toString();
    //     d2 = d2.addDays(-2);
    //     d = d1 - d2;
    //     b = d==Duration(2,0,0)-Duration(0,0,0,3600);
    //     kDebug()<<"3: Success="<<b<<""<<d1.toString()<<"-"<<d2.toString()<<"="<<d.toString();
    //     d = d2 - d1;
    //     b = d==Duration(2,0,0)-Duration(0,0,0,3600);
    //     kDebug()<<"4: Success="<<b<<""<<d2.toString()<<"-"<<d1.toString()<<"="<<d.toString();
    //     kDebug();
    //     b = (d2 + d)==d1;
    //     kDebug()<<"5: Success="<<b<<""<<d2<<"+"<<d.toString()<<"="<<d1;
    //     b = (d1 - d)==d2;
    //     kDebug()<<"6: Success="<<b<<""<<d1<<"-"<<d.toString()<<"="<<d2;
    //     } // end test 1
    //     kDebug();
    //     kDebug()<<"------------Test 2 Single calendar-----------------";
    //     {
    //     Calendar *t = new Calendar("Test 2");
    //     QDate wdate(2006,1,2);
    //     DateTime before = DateTime(wdate.addDays(-1));
    //     DateTime after = DateTime(wdate.addDays(1));
    //     QTime t1(8,0,0);
    //     QTime t2(10,0,0);
    //     DateTime wdt1(wdate, t1);
    //     DateTime wdt2(wdate, t2);
    //     CalendarDay *day = new CalendarDay(QDate(2006,1,2), CalendarDay::Working);
    //     day->addInterval(TimeInterval(t1, t2));
    //     if (!t->addDay(day)) {
    //         kDebug()<<"Failed to add day";
    //         delete day;
    //         delete t;
    //         return;
    //     }
    //     kDebug()<<"Added     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay();
    //     kDebug()<<"Found     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay();
    //
    //     CalendarDay *d = t->findDay(wdate);
    //     bool b = (day == d);
    //     kDebug()<<"1: Success="<<b<<"      Find same day";
    //
    //     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
    //     b = !dt.isValid();
    //     kDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after<<"): ="<<dt;
    //
    //     dt = t->firstAvailableBefore(before, before.addDays(-10));
    //     b = !dt.isValid();
    //     kDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<dt;
    //
    //     dt = t->firstAvailableAfter(before, after);
    //     b = dt == wdt1;
    //     kDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before<<"): ="<<dt;
    //
    //     dt = t->firstAvailableBefore(after, before);
    //     b = dt == wdt2;
    //     kDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after<<"): ="<<dt;
    //
    //     b = t->hasInterval(before, after);
    //     kDebug()<<"6: Success="<<b<<"      hasInterval("<<before<<","<<after<<")";
    //
    //     b = !t->hasInterval(after, after.addDays(1));
    //     kDebug()<<"7: Success="<<b<<"      !hasInterval("<<after<<","<<after.addDays(1)<<")";
    //
    //     b = !t->hasInterval(before, before.addDays(-1));
    //     kDebug()<<"8: Success="<<b<<"      !hasInterval("<<before<<","<<before.addDays(-1)<<")";
    //
    //     Duration e1(0, 2, 0); // 2 hours
    //     Duration e2 = t->effort(before, after);
    //     b = e1==e2;
    //     kDebug()<<"9: Success="<<b<<"      effort"<<e1.toString()<<" ="<<e2.toString();
    //
    //     delete t;
    //     }// end test 2
    //
    //     kDebug();
    //     kDebug()<<"------------Test 3 Parent calendar-----------------";
    //     {
    //     Calendar *t = new Calendar("Test 3");
    //     Calendar *p = new Calendar("Test 3 parent");
    //     t->setParent(p);
    //     QDate wdate(2006,1,2);
    //     DateTime before = DateTime(wdate.addDays(-1));
    //     DateTime after = DateTime(wdate.addDays(1));
    //     QTime t1(8,0,0);
    //     QTime t2(10,0,0);
    //     DateTime wdt1(wdate, t1);
    //     DateTime wdt2(wdate, t2);
    //     CalendarDay *day = new CalendarDay(QDate(2006,1,2), CalendarDay::Working);
    //     day->addInterval(TimeInterval(t1, t2));
    //     if (!p->addDay(day)) {
    //         kDebug()<<"Failed to add day";
    //         delete day;
    //         delete t;
    //         return;
    //     }
    //     kDebug()<<"Added     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay().toString();
    //     kDebug()<<"Found     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay().toString();
    //
    //     CalendarDay *d = p->findDay(wdate);
    //     bool b = (day == d);
    //     kDebug()<<"1: Success="<<b<<"      Find same day";
    //
    //     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
    //     b = !dt.isValid();
    //     kDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after.toString()<<"): ="<<!b;
    //
    //     dt = t->firstAvailableBefore(before, before.addDays(-10));
    //     b = !dt.isValid();
    //     kDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<!b;
    //
    //     dt = t->firstAvailableAfter(before, after);
    //     b = dt == wdt1;
    //     kDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before.toString()<<"): ="<<dt.toString();
    //
    //     dt = t->firstAvailableBefore(after, before);
    //     b = dt == wdt2;
    //     kDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after.toString()<<"): ="<<dt.toString();
    //
    //     b = t->hasInterval(before, after);
    //     kDebug()<<"6: Success="<<b<<"      hasInterval("<<before.toString()<<","<<after<<")";
    //
    //     b = !t->hasInterval(after, after.addDays(1));
    //     kDebug()<<"7: Success="<<b<<"      !hasInterval("<<after.toString()<<","<<after.addDays(1)<<")";
    //
    //     b = !t->hasInterval(before, before.addDays(-1));
    //     kDebug()<<"8: Success="<<b<<"      !hasInterval("<<before.toString()<<","<<before.addDays(-1)<<")";
    //     Duration e1(0, 2, 0); // 2 hours
    //     Duration e2 = t->effort(before, after);
    //     b = e1==e2;
    //     kDebug()<<"9: Success="<<b<<"      effort"<<e1.toString()<<"=="<<e2.toString();
    //
    //     delete t;
    //     delete p;
    //     }// end test 3
    //     kDebug();
    //     kDebug()<<"------------Test 4 Parent calendar/weekdays-------------";
    //     {
    //     QTime t1(8,0,0);
    //     QTime t2(10,0,0);
    //     Calendar *p = new Calendar("Test 4 parent");
    //     CalendarDay *wd1 = p->weekday(0); // monday
    //     if (wd1 == 0) {
    //         kDebug()<<"Failed to get weekday";
    //     }
    //     wd1->setState(CalendarDay::NonWorking);
    //
    //     CalendarDay *wd2 = p->weekday(2); // wednesday
    //     if (wd2 == 0) {
    //         kDebug()<<"Failed to get weekday";
    //     }
    //     wd2->addInterval(TimeInterval(t1, t2));
    //     wd2->setState(CalendarDay::Working);
    //
    //     Calendar *t = new Calendar("Test 4");
    //     t->setParent(p);
    //     QDate wdate(2006,1,2); // monday jan 2
    //     DateTime before = DateTime(wdate.addDays(-4)); //Thursday dec 29
    //     DateTime after = DateTime(wdate.addDays(4)); // Friday jan 6
    //     DateTime wdt1(wdate, t1);
    //     DateTime wdt2(QDate(2006, 1, 4), t2); // Wednesday
    //     CalendarDay *day = new CalendarDay(QDate(2006,1,2), CalendarDay::Working);
    //     day->addInterval(TimeInterval(t1, t2));
    //     if (!p->addDay(day)) {
    //         kDebug()<<"Failed to add day";
    //         delete day;
    //         delete t;
    //         return;
    //     }
    //     kDebug()<<"Added     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay().toString();
    //     kDebug()<<"Found     date="<<day->date().toString()<<""<<day->startOfDay().toString()<<" -"<<day->endOfDay().toString();
    //
    //     CalendarDay *d = p->findDay(wdate);
    //     bool b = (day == d);
    //     kDebug()<<"1: Success="<<b<<"      Find same day";
    //
    //     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
    //     b = (dt.isValid() && dt == DateTime(QDate(2006,1,11), t1));
    //     kDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after<<"): ="<<dt;
    //
    //     dt = t->firstAvailableBefore(before, before.addDays(-10));
    //     b = (dt.isValid() && dt == DateTime(QDate(2005, 12, 28), t2));
    //     kDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<dt;
    //
    //     dt = t->firstAvailableAfter(before, after);
    //     b = dt == wdt1; // We find the day jan 2
    //     kDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before.toString()<<"): ="<<dt.toString();
    //
    //     dt = t->firstAvailableBefore(after, before);
    //     b = dt == wdt2; // We find the weekday (wednesday)
    //     kDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after.toString()<<"): ="<<dt.toString();
    //
    //     b = t->hasInterval(before, after);
    //     kDebug()<<"6: Success="<<b<<"      hasInterval("<<before.toString()<<","<<after<<")";
    //
    //     b = !t->hasInterval(after, after.addDays(1));
    //     kDebug()<<"7: Success="<<b<<"      !hasInterval("<<after.toString()<<","<<after.addDays(1)<<")";
    //
    //     b = !t->hasInterval(before, before.addDays(-1));
    //     kDebug()<<"8: Success="<<b<<"      !hasInterval("<<before.toString()<<","<<before.addDays(-1)<<")";
    //     Duration e1(0, 4, 0); // 2 hours
    //     Duration e2 = t->effort(before, after);
    //     b = e1==e2;
    //     kDebug()<<"9: Success="<<b<<"      effort"<<e1.toString()<<"="<<e2.toString();
    //
    //     DateTimeInterval r = t->firstInterval(before, after);
    //     b = r.first == wdt1; // We find the monday jan 2
    //     kDebug()<<"10: Success="<<b<<"      firstInterval("<<before<<"): ="<<r.first<<","<<r.second;
    //     r = t->firstInterval(r.second, after);
    //     b = r.first == DateTime(QDate(2006, 1, 4),t1); // We find the wednesday jan 4
    //     kDebug()<<"11: Success="<<b<<"      firstInterval("<<r.second<<"): ="<<r.first<<","<<r.second;
    //
    //     delete t;
    //     delete p;
    //     }// end test 4
}
#endif

}  //KPlato namespace

#include "kptview.moc"
