/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kwdoc.h"
#include "kwframe.h"
#include "kwtextframeset.h"
#include "framedia.h"
#include "framedia.moc"
#include "defs.h"
#include "kwcommand.h"
#include "kwtableframeset.h"

#include <klocale.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qpixmap.h>
#include <qradiobutton.h>
#include <qevent.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qhbox.h>
#include <qheader.h>
#include <kmessagebox.h>
#include <qvalidator.h>

#include <stdlib.h>
#include <limits.h>

#include <kdebug.h>

/******************************************************************/
/* Class: KWFrameDia                                              *
 *
 *  TAB Frame Options
 *      Set options dependend of frametype
 *  TAB Text Runaround
 *      Set the text behaviour of this frame
 *  TAB Frameset
 *      here the user can select from the current TEXT framesets, a new one is
 *      included in the list.
 *      Afterwards (on ok) the frame should be checked if it is already owned by a
 *      frameset, if so that connection must be disconnected (if different) and
 *      framebehaviour will be copied from the frameset
 *      then the new connection should be made.
 *
 *  TAB Geometry
 *      position/size
 ******************************************************************/

/*================================================================*/
KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame)
    : KDialogBase( Tabbed, i18n("Frame settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    frame = _frame;
    KWFrameSet * fs = frame->getFrameSet();
    KWTableFrameSet *table = fs->getGroupManager();
    if(table)
        frameType = table->getFrameType();
    else
        frameType = frame->getFrameSet() ? frame->getFrameSet()->getFrameType() : (FrameType) -1;
    doc = 0;
    init();
}

/* Contructor when the dialog is used on creation of frame */
KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame, KWDocument *_doc, FrameType _ft )
    : KDialogBase( Tabbed, i18n("Frame settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    frameType=_ft;
    doc = _doc;
    frame= _frame;
    init();
}

void KWFrameDia::init() {

    tab1 = tab2 = tab3 = tab4 = 0;
    if (frame) {
        KoRect r = frame->normalize();
        frame->setRect( r.x(), r.y(), r.width(), r.height() );
        if(!doc && frame->getFrameSet())
        {
            doc=frame->getFrameSet()->kWordDocument();
        }
        if(!doc)
        {
            kdDebug() << "ERROR: KWFrameDia::init frame has no reference to doc.."<<endl;
            return;
        }
        if(doc->processingType() != KWDocument::DTP &&
           frame->getFrameSet() == doc->getFrameSet(0))
        {
            setupTab2();
            setupTab4();
            runGroup->setEnabled(false);
        }
        else if(frameType == FT_TEXT)
        {
            setupTab1();
            setupTab2();
            setupTab3();
            setupTab4();
            if(! frame->getFrameSet()) // first creation
                showPage(2);
        }
        else if(frameType == FT_PICTURE)
        {
            setupTab1();
            setupTab2();
            setupTab4();
            showPage(1); // while options are not implemented..
        }
        else if(frameType == FT_PART)
        {
            setupTab2();
            setupTab4();
        }
        else if(frameType == FT_FORMULA)
        {
            setupTab1();
            setupTab2();
            setupTab4();
            showPage(1); // while options are not implemented..
        }
        else if(frameType == FT_TABLE)
        {
             setupTab4();
             grp1->setEnabled(false);
        }
    }
    else
        kdDebug() << "ERROR: KWFrameDia::init  no frame.."<<endl;
    setInitialSize( QSize(550, 400) );
}

/*================================================================*/

void KWFrameDia::setupTab1(){ // TAB Frame Options
    //kdDebug() << "setup tab 1 Frame options"<<endl;
    tab1 = addPage( i18n("Options") );

    int rows=2;
    if(frameType == FT_FORMULA || frameType == FT_PICTURE) {
        rows++;
        grid1 = new QGridLayout( tab1, rows, 1, KDialog::marginHint(), KDialog::spacingHint() );
    }
    if(frameType == FT_TEXT){
        rows+=2;
        grid1 = new QGridLayout( tab1, rows, 2, KDialog::marginHint(), KDialog::spacingHint() );
    }

    rows--;
    for(int i=0;i<rows;i++)
        grid1->setRowStretch( i, 0 );
    grid1->setRowStretch( rows, 1 );

    // formula frame
    if(frameType==FT_FORMULA) {
        autofit = new QCheckBox (i18n("Autofit framesize"),tab1);
        autofit->setEnabled(false);
        grid1->addWidget(autofit,1,0);

        // Picture frame
    } else if(frameType==FT_PICTURE) {
        aspectRatio = new QCheckBox (i18n("Retain original aspect-ratio"),tab1);
        aspectRatio->setEnabled(false);
        grid1->addWidget(aspectRatio,1,0);

        // Text frame
    } else if(frameType==FT_TEXT) {

        // AutoCreateNewFrame policy.
        endOfFrame = new QGroupBox(i18n("If text is too long for frame:"), tab1 );
        grid1->addWidget( endOfFrame, 1, 0 );

        eofGrid= new QGridLayout (endOfFrame, 4, 1, KDialog::marginHint(), KDialog::spacingHint());
        rAppendFrame = new QRadioButton( i18n( "Create a new page" ), endOfFrame );
        eofGrid->addWidget( rAppendFrame, 1, 0 );

        rResizeFrame = new QRadioButton( i18n( "Resize last frame" ), endOfFrame );
        eofGrid->addWidget( rResizeFrame, 2, 0 );

        rNoShow = new QRadioButton( i18n( "Don't show the extra text" ), endOfFrame );
        eofGrid->addWidget( rNoShow, 3, 0 );
        QButtonGroup *grp = new QButtonGroup( endOfFrame );
        grp->hide();
        grp->setExclusive( true );
        grp->insert( rAppendFrame );
        grp->insert( rResizeFrame );
        grp->insert( rNoShow );

        eofGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );

        if(frame->getFrameBehaviour() == AutoExtendFrame) {
            rResizeFrame->setChecked(true);
        } else if (frame->getFrameBehaviour() == AutoCreateNewFrame) {
            rAppendFrame->setChecked(true);
        } else {
            rNoShow->setChecked(true);
        }

        // NewFrameBehaviour
        onNewPage = new QGroupBox(i18n("On new page creation:"),tab1);
        grid1->addWidget( onNewPage, 1, 1 );

        onpGrid = new QGridLayout( onNewPage, 4, 1, KDialog::marginHint(), KDialog::spacingHint() );
        reconnect = new QRadioButton (i18n ("Reconnect frame to current flow"), onNewPage);
        connect( reconnect, SIGNAL( clicked() ), this, SLOT( setFrameBehaviourInputOn() ) );
        onpGrid ->addWidget( reconnect, 1, 0 );

        noFollowup = new QRadioButton (i18n ("Don't create a followup frame"), onNewPage);
        connect( noFollowup, SIGNAL( clicked() ), this, SLOT( setFrameBehaviourInputOn() ) );
        onpGrid ->addWidget( noFollowup, 2, 0 );

        copyRadio= new QRadioButton (i18n ("Place a copy of this frame"), onNewPage);
        connect( copyRadio, SIGNAL( clicked() ), this, SLOT( setFrameBehaviourInputOff() ) );
        onpGrid ->addWidget( copyRadio, 3, 0);

        QButtonGroup *grp2 = new QButtonGroup( onNewPage );
        grp2->hide();
        grp2->setExclusive( true );
        grp2->insert( reconnect );
        grp2->insert( noFollowup );
        grp2->insert( copyRadio );
        grid1->addRowSpacing(1,onNewPage->height());
        if(frame->getNewFrameBehaviour() == Reconnect) {
            reconnect->setChecked(true);
        } else if(frame->getNewFrameBehaviour() == NoFollowup) {
            noFollowup->setChecked(true);
        } else {
            copyRadio->setChecked(true);
            setFrameBehaviourInputOff();
        }

        // SideHeads definition
        sideHeads = new QGroupBox(i18n("SideHead definition"),tab1);
        sideHeads->setEnabled(false);
        grid1->addWidget(sideHeads,2,0);

        sideGrid = new QGridLayout( sideHeads, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );
        sideTitle1 = new QLabel ( i18n("Size ( %1 ):").arg(doc->getUnitName()),sideHeads);
        sideTitle1->resize(sideTitle1->sizeHint());
        sideGrid->addWidget(sideTitle1,1,0);
        sideWidth= new QLineEdit(sideHeads,"");
        sideWidth->setMaxLength(6);
        sideGrid->addWidget(sideWidth,1,1);
        sideTitle2 = new QLabel( i18n("Gap size ( %1 ):").arg(doc->getUnitName()),sideHeads);
        sideTitle2->resize(sideTitle2->sizeHint());
        sideGrid->addWidget(sideTitle2,2,0);
        sideGap = new QLineEdit(sideHeads,"");
        sideGap->setMaxLength(6);
        sideGrid->addWidget(sideGap,2,1);
        sideAlign = new QComboBox (false,sideHeads);
        sideAlign->setAutoResize(false);
        sideAlign->insertItem ( i18n("Left"));
        sideAlign->insertItem ( i18n("Right"));
        sideAlign->insertItem ( i18n("Closest to binding"));
        sideAlign->insertItem ( i18n("Closest to page edge"));
        sideAlign->resize(sideAlign->sizeHint());
        sideGrid->addMultiCellWidget(sideAlign,3,3,0,1);
        sideGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );

        // init for sideheads.
        sideWidth->setText("0");
        sideWidth->setValidator( new QDoubleValidator( sideWidth) );

        sideGap->setText("0");
        sideGap->setValidator( new QDoubleValidator( sideGap) );
        // add rest of sidehead init..
    }

    //kdDebug() << "setup tab 1 exit"<<endl;
}


void KWFrameDia::setupTab2() // TAB Text Runaround
{
    //kdDebug() << "setup tab 2 text runaround"<<endl;

    tab2 = addPage( i18n( "Text run around" ) );

    grid2 = new QGridLayout( tab2, 3, 2, KDialog::marginHint(), KDialog::spacingHint() );

    runGroup = new QGroupBox( i18n( "Text in other frames will:" ), tab2 );

    runGrid = new QGridLayout( runGroup, 4, 3, KDialog::marginHint(), KDialog::spacingHint() );

    QPixmap pixmap = KWBarIcon( "run_not" );
    lRunNo = new QLabel( runGroup );
    lRunNo->setBackgroundPixmap( pixmap );
    lRunNo->setFixedSize( pixmap.size() );
    runGrid->addWidget( lRunNo, 1, 0 );
    runGrid->addColSpacing( 0, pixmap.width());

    pixmap = KWBarIcon( "run_bounding" );
    lRunBounding = new QLabel( runGroup );
    lRunBounding->setBackgroundPixmap( pixmap );
    lRunBounding->setFixedSize( pixmap.size() );
    runGrid->addWidget( lRunBounding, 2, 0 );

    pixmap = KWBarIcon( "run_skip" );
    lRunContur = new QLabel( runGroup );
    lRunContur->setBackgroundPixmap( pixmap );
    lRunContur->setFixedSize( pixmap.size() );
    runGrid->addWidget( lRunContur, 3, 0 );

    rRunNo = new QRadioButton( i18n( "&Run through this frame" ), runGroup );
    runGrid->addWidget( rRunNo, 1, 1 );
    connect( rRunNo, SIGNAL( clicked() ), this, SLOT( runNoClicked() ) );

    rRunBounding = new QRadioButton( i18n( "Run around the &bounding rectangle of this frame" ), runGroup );
    runGrid->addWidget( rRunBounding, 2, 1 );
    connect( rRunBounding, SIGNAL( clicked() ), this, SLOT( runBoundingClicked() ) );

    rRunContur = new QRadioButton( i18n( "Do&n't run around this frame" ), runGroup );
    runGrid->addWidget( rRunContur, 3, 1 );
    connect( rRunContur, SIGNAL( clicked() ), this, SLOT( runConturClicked() ) );

    runGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );

    grid2->addWidget( runGroup, 0, 0 );
    grid2->addMultiCellWidget( runGroup, 0, 0, 0, 1 );

    lRGap = new QLabel( i18n( "Run around gap ( %1 ):" ).arg(doc->getUnitName()), tab2 );
    lRGap->resize( lRGap->sizeHint() );
    lRGap->setAlignment( AlignRight | AlignVCenter );
    grid2->addWidget( lRGap, 1, 0 );

    eRGap = new QLineEdit( tab2 );
    eRGap->setValidator( new QDoubleValidator( eRGap ) );
    eRGap->setText( "0.00" );
    eRGap->setMaxLength( 5 );
    eRGap->setEchoMode( QLineEdit::Normal );
    eRGap->setFrame( true );
    eRGap->resize( eRGap->sizeHint() );
    grid2->addWidget( eRGap, 1, 1 );

    RunAround ra = RA_NO;
    if ( frame )
        ra = frame->getRunAround();
    else
    {
        KWFrame *firstFrame = doc->getFirstSelectedFrame();
        if ( firstFrame )
            ra = firstFrame->getRunAround();
    }

    switch ( ra ) {
    case RA_NO: rRunNo->setChecked( true );
        break;
    case RA_BOUNDINGRECT: rRunBounding->setChecked( true );
        break;
    case RA_SKIP: rRunContur->setChecked( true );
        break;
    }

    double ragap = 0;
    if ( frame )
        ragap = frame->getRunAroundGap();
    else
    {
        KWFrame *firstFrame = doc->getFirstSelectedFrame();
        if ( firstFrame )
            ragap = firstFrame->getRunAroundGap();
    }

    QString str;
    str.setNum( KWUnit::userValue( ragap, doc->getUnit() ) );
    eRGap->setText( str );

    //kdDebug() << "setup tab 2 exit"<<endl;
}

void KWFrameDia::setupTab3(){ // TAB Frameset
    /*
     * here the user can select from the current TEXT framesets, a new one is
     * included in the list.
     * Afterwards (on ok) the frame should be checked if it is allready owned by a
     * frameset, if so that connection must be disconnected (if different) and
     * framebehaviour will be copied from the frameset
     * then the new connection should be made.
 */
    //kdDebug() << "setup tab 3 frameSet"<<endl;
    tab3 = addPage( i18n( "Connect text frames" ) );

    grid3 = new QGridLayout( tab3, 3, 1, KDialog::marginHint(), KDialog::spacingHint() );

    lFrameSet = new QLabel( i18n( "Choose a frameset to which the current frame should be connected:" ), tab3 );
    lFrameSet->resize( lFrameSet->sizeHint() );
    grid3->addWidget( lFrameSet, 0, 0 );

    lFrameSList = new QListView( tab3 );
    lFrameSList->addColumn( i18n( "Nr." ) );
    lFrameSList->addColumn( i18n( "Frameset name" ) );
    lFrameSList->setAllColumnsShowFocus( true );
    lFrameSList->header()->setMovingEnabled( false );

    int numTxtFrameSets=0;
    for ( unsigned int i = 0; i < doc->getNumFrameSets(); i++ ) {
        if ( i == 0 && doc->processingType() == KWDocument::WP )
            continue;
        if ( doc->getFrameSet( i )->getFrameType() != FT_TEXT ||
             dynamic_cast<KWTextFrameSet*>( doc->getFrameSet( i ) )->getFrameInfo() != FI_BODY )
            continue;
        if ( doc->getFrameSet( i )->getGroupManager() )
            continue;
        QListViewItem *item = new QListViewItem( lFrameSList );
        item->setText( 0, QString( "%1" ).arg( i + 1 ) );
        item->setText( 1, doc->getFrameSet( i )->getName() );
        if(frame->getFrameSet() && frame->getFrameSet()==doc->getFrameSet(i))
            lFrameSList->setSelected(item, TRUE );
        numTxtFrameSets++;
    }

    if (! frame->getFrameSet()) {
        QListViewItem *item = new QListViewItem( lFrameSList );
        item->setText( 0, QString( "*%1" ).arg( doc->getNumFrameSets()+1 ) );
        item->setText( 1, i18n( "Create a new frameset with this frame" ) );
        lFrameSList->setSelected( lFrameSList->firstChild(), TRUE );
    }

    connect( lFrameSList, SIGNAL( currentChanged( QListViewItem * ) ),
             this, SLOT( connectListSelected( QListViewItem * ) ) );
    grid3->addWidget( lFrameSList, 1, 0 );

    QHBox *row = new QHBox( tab3 );
    row->setSpacing( 5 );
    ( void )new QLabel( i18n( "Name of new frameset:" ), row );
    eFrameSetName = new QLineEdit( row );

    grid3->addWidget( row, 2, 0 );

    bool found=true;
    while (found) {
        numTxtFrameSets++;

        bool same = false;
        for ( unsigned int i = 0;!same && i < doc->getNumFrameSets(); ++i ) {
            if ( doc->getFrameSet( i )->getName() == i18n("Frameset %1" ).arg(numTxtFrameSets)){
                same = true;
                break;
            }
        }
        if(!same) found=false;
    }

    oldFrameName=i18n( "Frameset %1" ).arg( numTxtFrameSets );
    eFrameSetName->setText( oldFrameName );
    connectListSelected( lFrameSList->firstChild() );

    //kdDebug() << "setup tab 3 exit"<<endl;

    if ( frame && frame->getFrameSet() && frame->getFrameSet()->isFloating() )
        tab3->setEnabled( false );
}

void KWFrameDia::setupTab4(){ // TAB Geometry
    //kdDebug() << "setup tab 4 geometry"<<endl;

    tab4 = addPage( i18n( "Geometry" ) );
    grid4 = new QGridLayout( tab4, 4, 1, KDialog::marginHint(), KDialog::spacingHint() );

    floating = new QCheckBox (i18n("Frame is floating"), tab4);

    connect( floating, SIGNAL( toggled(bool) ), this, SLOT( slotFloatingToggled(bool) ) );
    int row = 0;
    grid4->addMultiCellWidget( floating, row, row, 0, 1 );

    /* ideally the following properties could be given to any floating frame:
       Position: (y)
        Top of frame
        Top of paragraph
        Above current line
        At insertion point
        Below current line
        Bottom of paragraph
        Bottom of frame
        Absolute
       Alignment: (x)
        Left
        Right
        Center
        Closest to binding
        Further from binding
        Absolute
    */


    grp1 = new QGroupBox( i18n("Position in %1").arg(doc->getUnitName()), tab4 );
    pGrid = new QGridLayout( grp1, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    lx = new QLabel( i18n( "Left:" ), grp1 );
    lx->resize( lx->sizeHint() );
    pGrid->addWidget( lx, 1, 0 );

    sx = new QLineEdit( grp1 );

    sx->setText( "0.00" );
    sx->setMaxLength( 16 );
    sx->setEchoMode( QLineEdit::Normal );
    sx->setFrame( true );
    sx->resize( sx->sizeHint() );
    pGrid->addWidget( sx, 2, 0 );

    ly = new QLabel( i18n( "Top:" ), grp1 );
    ly->resize( ly->sizeHint() );
    pGrid->addWidget( ly, 1, 1 );

    sy = new QLineEdit( grp1 );

    sy->setText( "0.00" );
    sy->setMaxLength( 16 );
    sy->setEchoMode( QLineEdit::Normal );
    sy->setFrame( true );
    sy->resize( sy->sizeHint() );
    pGrid->addWidget( sy, 2, 1 );

    lw = new QLabel( i18n( "Width:" ), grp1 );
    lw->resize( lw->sizeHint() );
    pGrid->addWidget( lw, 3, 0 );

    sw = new QLineEdit( grp1 );

    sw->setText( "0.00" );
    sw->setMaxLength( 16 );
    sw->setEchoMode( QLineEdit::Normal );
    sw->setFrame( true );
    sw->resize( sw->sizeHint() );
    pGrid->addWidget( sw, 4, 0 );

    lh = new QLabel( i18n( "Height:" ), grp1 );
    lh->resize( lh->sizeHint() );
    pGrid->addWidget( lh, 3, 1 );

    sh = new QLineEdit( grp1 );

    sh->setText( "0.00" );
    sh->setMaxLength( 16 );
    sh->setEchoMode( QLineEdit::Normal );
    sh->setFrame( true );
    sh->resize( sh->sizeHint() );
    pGrid->addWidget( sh, 4, 1 );

    pGrid->addRowSpacing( 0, KDialog::spacingHint() + 5 );

    grid4->addWidget( grp1, ++row, 0 );

    grp2 = new QGroupBox( i18n("Margins in %1").arg(doc->getUnitName()), tab4 );
    mGrid = new QGridLayout( grp2, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    lml = new QLabel( i18n( "Left:" ), grp2 );
    lml->resize( lml->sizeHint() );
    mGrid->addWidget( lml, 1, 0 );

    sml = new QLineEdit( grp2 );

    sml->setText( "0.00" );
    sml->setMaxLength( 5 );
    sml->setEchoMode( QLineEdit::Normal );
    sml->setFrame( true );
    sml->resize( sml->sizeHint() );
    mGrid->addWidget( sml, 2, 0 );

    lmr = new QLabel( i18n( "Right:" ), grp2 );
    lmr->resize( lmr->sizeHint() );
    mGrid->addWidget( lmr, 1, 1 );

    smr = new QLineEdit( grp2 );

    smr->setText( "0.00" );
    smr->setMaxLength( 5 );
    smr->setEchoMode( QLineEdit::Normal );
    smr->setFrame( true );
    smr->resize( smr->sizeHint() );
    mGrid->addWidget( smr, 2, 1 );

    lmt = new QLabel( i18n( "Top:" ), grp2 );
    lmt->resize( lmt->sizeHint() );
    mGrid->addWidget( lmt, 3, 0 );

    smt = new QLineEdit( grp2 );

    smt->setText( "0.00" );
    smt->setMaxLength( 5 );
    smt->setEchoMode( QLineEdit::Normal );
    smt->setFrame( true );
    smt->resize( smt->sizeHint() );
    mGrid->addWidget( smt, 4, 0 );

    lmb = new QLabel( i18n( "Bottom:" ), grp2 );
    lmb->resize( lmb->sizeHint() );
    mGrid->addWidget( lmb, 3, 1 );

    smb = new QLineEdit( grp2 );

    smb->setText( "0.00" );
    smb->setMaxLength( 5 );
    smb->setEchoMode( QLineEdit::Normal );
    smb->setFrame( true );
    smb->resize( smb->sizeHint() );
    mGrid->addWidget( smb, 4, 1 );

    mGrid->addRowSpacing( 0, KDialog::spacingHint() + 5 );

    grid4->addWidget( grp2, ++row, 0 );

    double l, r, t, b;
    doc->getFrameMargins( l, r, t, b );
    sml->setText( QString::number( QMAX(0.00,KWUnit::userValue( l, doc->getUnit() ) ) ));
    smr->setText( QString::number( QMAX(0.00,KWUnit::userValue( r, doc->getUnit() ) ) ));
    smt->setText( QString::number( QMAX(0.00,KWUnit::userValue( t, doc->getUnit() ) ) ));
    smb->setText( QString::number( QMAX(0.00,KWUnit::userValue( b, doc->getUnit() ) ) ));

    sx->setValidator( new QDoubleValidator( sx ) );
    sy->setValidator( new QDoubleValidator( sy ) );
    smb->setValidator( new QDoubleValidator( smb ) );
    sml->setValidator( new QDoubleValidator( sml ) );
    smr->setValidator( new QDoubleValidator( smr ) );
    smt->setValidator( new QDoubleValidator( smt ) );
    sh->setValidator( new QDoubleValidator( sh ) );
    sw->setValidator( new QDoubleValidator( sw ) );

    bool disable = false;
    if ( doc->isOnlyOneFrameSelected() )
    {
        KWFrame * theFrame = doc->getFirstSelectedFrame();

        oldX = KWUnit::userValue( theFrame->x(), doc->getUnit() );
        oldY = KWUnit::userValue( theFrame->y(), doc->getUnit() );
        oldW = KWUnit::userValue( theFrame->width(), doc->getUnit() );
        oldH = KWUnit::userValue( theFrame->height(), doc->getUnit() );

        sx->setText( QString::number( oldX ) );
        sy->setText( QString::number( oldY ) );
        sw->setText( QString::number( oldW ) );
        sh->setText( QString::number( oldH ) );



        KWFrameSet * fs = theFrame->getFrameSet();
        KWTableFrameSet *table = fs->getGroupManager();
        bool f = false;
        if(table)
        {
            f=table->isFloating();
            floating->setEnabled( false );
        }
        else
            f = theFrame->getFrameSet()->isFloating();
        floating->setChecked( f );

        slotFloatingToggled( f );

        // Can't change geometry of main WP frame
        if ( doc->processingType() == KWDocument::WP &&
             doc->getFrameSetNum( theFrame->getFrameSet() ) == 0 )
            disable = true;
    }
    else
        disable = true;

    if ( disable )
    {
        sx->setEnabled( false );
        sy->setEnabled( false );
        sw->setEnabled( false );
        sh->setEnabled( false );
        floating->setEnabled( false );
    }

    //kdDebug() << "setup tab 4 exit"<<endl;
}




/*================================================================*/
void KWFrameDia::uncheckAllRuns()
{
    rRunNo->setChecked( false );
    rRunBounding->setChecked( false );
    rRunContur->setChecked( false );
}

/*================================================================*/
void KWFrameDia::runNoClicked()
{
    uncheckAllRuns();
    rRunNo->setChecked( true );
}

/*================================================================*/
void KWFrameDia::runBoundingClicked()
{
    uncheckAllRuns();
    rRunBounding->setChecked( true );
}

/*================================================================*/
void KWFrameDia::runConturClicked()
{

    uncheckAllRuns();
    rRunContur->setChecked( true );
}

void KWFrameDia::setFrameBehaviourInputOn() {
    if(!rResizeFrame->isEnabled()) {
        if(frameBehaviour== AutoExtendFrame) {
            rResizeFrame->setChecked(true);
        } else if (frameBehaviour== AutoCreateNewFrame) {
            rAppendFrame->setChecked(true);
        } else {
            rNoShow->setChecked(true);
        }
        rResizeFrame->setEnabled(true);
        rAppendFrame->setEnabled(true);
        rNoShow->setEnabled(true);
    }
}

void KWFrameDia::setFrameBehaviourInputOff() {
    if(rResizeFrame->isEnabled()) {
        if(rResizeFrame->isChecked()) {
            frameBehaviour=AutoExtendFrame;
        } else if ( rAppendFrame->isChecked()) {
            frameBehaviour=AutoCreateNewFrame;
        } else {
            frameBehaviour=Ignore;
        }
        rNoShow->setChecked(true);
        rResizeFrame->setEnabled(false);
        rAppendFrame->setEnabled(false);
        rNoShow->setEnabled(false);
    }
}

void KWFrameDia::slotFloatingToggled(bool b)
{
    grp1->setEnabled( !b ); // Position doesn't make sense for a floating frame
    // grp2->setEnabled( !b ); do margins make sense for floating frames ?
}

/*================================================================*/
bool KWFrameDia::applyChanges()
{
    //kdDebug() << "KWFrameDia::applyChanges"<<endl;
    ASSERT(frame);
    if ( !frame )
        return false;

    if ( tab1 && frameType==FT_TEXT )
    {
        // FrameBehaviour
        if(rResizeFrame->isChecked())
        {
            frame->setFrameBehaviour(AutoExtendFrame);
        }
        else if ( rAppendFrame->isChecked())
        {
            frame->setFrameBehaviour(AutoCreateNewFrame);
        }
        else
        {
            frame->setFrameBehaviour(Ignore);
        }

        // NewFrameBehaviour
        if(reconnect->isChecked())
        {
            frame->setNewFrameBehaviour(Reconnect);
        }
        else if ( noFollowup->isChecked())
        {
            frame->setNewFrameBehaviour(NoFollowup);
        }
        else
        {
            frame->setNewFrameBehaviour(Copy);
        }

        QString str = lFrameSList->currentItem() ? lFrameSList->currentItem()->text( 0 ) : QString::null;
        QString name = QString::null;
        if ( str[ 0 ] == '*' ) {
            str.remove( 0, 1 );
            name = eFrameSetName->text();
            if ( name.isEmpty() )
                name = i18n( "Frameset %d" ).arg( doc->getNumFrameSets() + 1 );
            bool same = FALSE;
            for ( unsigned int i = 0; i < doc->getNumFrameSets(); ++i ) {
                if ( doc->getFrameSet( i )->getName() == name ) {
                    same = TRUE;
                    break;
                }
            }
            if ( same ) {
                KMessageBox::sorry( this,
                                    i18n( "A new frameset with the name '%1'\n"
                                          "can not be made because a frameset with that name\n"
                                          "already exists. Please enter another name or select\n"
                                          "an existing frameset from the list.").arg(name));
                eFrameSetName->setText(oldFrameName);
                return false;
            }
        }
        int _num = str.toInt() - 1;

        // delete frame from frameset
        if ( frame->getFrameSet() &&
             ! (static_cast<unsigned int>( _num ) < doc->getNumFrameSets() &&
                frame->getFrameSet() == doc->getFrameSet(_num))) {
            if ( frame->getFrameSet()->getNumFrames() > 1 )
                frame->getFrameSet()->delFrame( frame, FALSE );
            else {
                frame->getFrameSet()->delFrame( frame, FALSE );
                doc->delFrameSet( frame->getFrameSet() );
            }
        }

        if(frame->getFrameSet() == 0L) { // if there is no frameset (anymore)
            // attach frame to frameset
            if ( static_cast<unsigned int>( _num ) < doc->getNumFrameSets() ) {
                doc->getFrameSet( _num )->addFrame( frame );
            } else { // create a new frameset
                kdDebug() << "KWFrameDia::applyChanges creating a new frameset" << endl;
                KWTextFrameSet *_frameSet = new KWTextFrameSet( doc );
                _frameSet->setName( name );
                _frameSet->addFrame( frame );
                doc->addFrameSet( _frameSet );
                KWCreateFrameCommand *cmd=new KWCreateFrameCommand( i18n("Create text frame"), doc, frame) ;
                doc->addCommand(cmd);
            }
        }
    } // end of 'tab1 and text frame'

    if ( tab1 )
    {
        // The floating attribute applies to the whole frameset...
        KWFrameSet * fs = frame->getFrameSet();

        // Floating
        if ( floating->isChecked() && !fs->isFloating() )
        {
            // turn non-floating frame into floating frame
            fs->setFloating();
            doc->repaintAllViews();
        }
        else if ( !floating->isChecked() && fs->isFloating() )
        {
            // turn floating-frame into non-floating frame
            fs->setFixed();
            doc->repaintAllViews();
        }
    }
    if ( tab2 )
    {
        // Run around
        if ( rRunNo->isChecked() )
            frame->setRunAround( RA_NO );
        else if ( rRunBounding->isChecked() )
            frame->setRunAround( RA_BOUNDINGRECT );
        else if ( rRunContur->isChecked() )
            frame->setRunAround( RA_SKIP );

        frame->setRunAroundGap( KWUnit::fromUserValue( eRGap->text().toDouble(), doc->getUnit() ) );
    }

    if ( doc->isOnlyOneFrameSelected() && ( doc->processingType() == KWDocument::DTP ||
                                            ( doc->processingType() == KWDocument::WP &&
                                              doc->getFrameSetNum( doc->getFirstSelectedFrameSet() ) > 0 ) ) ) {
        if ( oldX != sx->text().toDouble() || oldY != sy->text().toDouble() || oldW != sw->text().toDouble() || oldH != sh->text().toDouble() ) {
            double px = KWUnit::fromUserValue( QMAX( sx->text().toDouble(), 0 ), doc->getUnit() );
            double py = KWUnit::fromUserValue( QMAX( sy->text().toDouble(), 0 ), doc->getUnit() );
            double pw = KWUnit::fromUserValue( QMAX( sw->text().toDouble(), 0 ), doc->getUnit() );
            double ph = KWUnit::fromUserValue( QMAX( sh->text().toDouble(), 0 ), doc->getUnit() );

            FrameIndex index;
            FrameResizeStruct tmpResize;
            tmpResize.sizeOfBegin = frame->normalize();

            frame->setRect( px, py, pw, ph );
            // TODO apply page limits
            // TODO undo-redo support

            tmpResize.sizeOfEnd = frame->normalize();
            KWTableFrameSet *table = frame->getFrameSet()->getGroupManager();
            if(table)
            {
                index.m_iFrameSetIndex=doc->getFrameSetNum(table);
                index.m_iFrameIndex=table->getFrameFromPtr(frame);
            }
            else
            {
                index.m_iFrameSetIndex=doc->getFrameSetNum(frame->getFrameSet());
                index.m_iFrameIndex=frame->getFrameSet()->getFrameFromPtr(frame);
            }

            KWFrameResizeCommand *cmd = new KWFrameResizeCommand( i18n("Resize Frame"), doc, index, tmpResize ) ;
            doc->addCommand(cmd);
            doc->frameChanged( frame );
            doc->setModified( true );
        }
    }

    double u1, u2, u3, u4;
    u1=KWUnit::fromUserValue( QMAX(sml->text().toDouble(),0), doc->getUnit() );
    u2=KWUnit::fromUserValue( QMAX(smr->text().toDouble(),0), doc->getUnit() );
    u3=KWUnit::fromUserValue( QMAX(smt->text().toDouble(),0), doc->getUnit() );
    u4=KWUnit::fromUserValue( QMAX(smb->text().toDouble(),0), doc->getUnit() );
    doc->setFrameMargins( u1, u2, u3, u4 );

    updateFrames();
    return true;
}

void KWFrameDia::updateFrames()
{
    QList<KWFrame> frames=doc->getSelectedFrames();

    doc->updateAllFrames();
    doc->layout();

    if(frames.count()==1)
    {
        KWFrame *theFrame = frames.first();
        if(theFrame->isSelected())
            theFrame->updateResizeHandles();
    }
}

void KWFrameDia::slotOk()
{
    if (applyChanges())
    {
        KDialogBase::slotOk();
    }
}

void KWFrameDia::connectListSelected( QListViewItem *item )
{
    if ( !item )
        return;

    QString str = item->text( 0 );
    if ( str[ 0 ] == '*' ) {
        str.remove( 0, 1 );
        eFrameSetName->setEnabled( TRUE );
    } else
        eFrameSetName->setEnabled( FALSE );
}

