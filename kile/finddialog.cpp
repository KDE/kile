
#include "finddialog.h"

#include <qvariant.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>

FindDialog::FindDialog(QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "FindDialog" );
    resize( 285, 189 ); 
    setCaption( i18n( "Find Text" ) );
    FindDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "FindDialogLayout"); 

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n("Find" ) );
    Layout1->addWidget( TextLabel1 );

    comboFind = new QComboBox( FALSE, this, "comboFind" );
    comboFind->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, comboFind->sizePolicy().hasHeightForWidth() ) );
    comboFind->setEditable( TRUE );
    Layout1->addWidget( comboFind );

    FindDialogLayout->addMultiCellLayout( Layout1, 0, 0, 0, 1 );

    Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2->addItem( spacer );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setText( i18n( "Find")  );
    PushButton1->setDefault( TRUE );
    Layout2->addWidget( PushButton1 );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setText( i18n( "Close")  );
    Layout2->addWidget( PushButton2 );

    FindDialogLayout->addMultiCellLayout( Layout2, 2, 2, 0, 1 );

    ButtonGroup2 = new QButtonGroup( this, "ButtonGroup2" );
    ButtonGroup2->setTitle( i18n( "Direction" ) );
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 6 );
    ButtonGroup2->layout()->setMargin( 11 );
    ButtonGroup2Layout = new QVBoxLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );

    radioForward = new QRadioButton( ButtonGroup2, "radioForward" );
    radioForward->setText( i18n( "Forward")  );
    radioForward->setChecked( TRUE );
    ButtonGroup2Layout->addWidget( radioForward );

    radioBackward = new QRadioButton( ButtonGroup2, "radioBackward" );
    radioBackward->setText( i18n( "Backward" ) );
    ButtonGroup2Layout->addWidget( radioBackward );

    FindDialogLayout->addWidget( ButtonGroup2, 1, 1 );

    ButtonGroup1 = new QButtonGroup( this, "ButtonGroup1" );
    ButtonGroup1->setTitle( i18n( "Options")  );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 6 );
    ButtonGroup1->layout()->setMargin( 11 );
    ButtonGroup1Layout = new QVBoxLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );

    checkWords = new QCheckBox( ButtonGroup1, "checkWords" );
    checkWords->setText( i18n( "Whole words only")  );
    ButtonGroup1Layout->addWidget( checkWords );

    checkCase = new QCheckBox( ButtonGroup1, "checkCase" );
    checkCase->setText( i18n( "Case sensitive" ) );
    ButtonGroup1Layout->addWidget( checkCase );

    checkBegin = new QCheckBox( ButtonGroup1, "checkBegin" );
    checkBegin->setText( i18n( "Start at Beginning" ) );
    ButtonGroup1Layout->addWidget( checkBegin );

    FindDialogLayout->addWidget( ButtonGroup1, 1, 0 );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( doFind() ) );
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( reject() ) );


    // tab order
    setTabOrder( comboFind, checkWords );
    setTabOrder( checkWords, checkCase );
    setTabOrder( checkCase, checkBegin );
    setTabOrder( checkBegin, radioForward );
    setTabOrder( radioForward, radioBackward );
    setTabOrder( radioBackward, PushButton1 );
    setTabOrder( PushButton1, PushButton2 );

    // buddies
    TextLabel1->setBuddy( comboFind );

 }

/*  
 *  Destroys the object and frees any allocated resources
 */
FindDialog::~FindDialog()
{

}

void FindDialog::doFind()
{
if ( !editor ) return;
if ( !editor->search( comboFind->currentText(), checkCase->isChecked(),	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked() ) )
   {
   checkBegin->setChecked( TRUE );
   }
else checkBegin->setChecked( FALSE );
editor->viewport()->repaint( FALSE );
}

void FindDialog::SetEditor(LatexEditor *ed)
{
editor=ed;
}


#include "finddialog.moc"
