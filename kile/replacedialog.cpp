
#include "replacedialog.h"

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
#include <kmessagebox.h>


ReplaceDialog::ReplaceDialog(QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ReplaceDialog" );
    resize( 286, 217 ); 
    setCaption(  i18n("Replace Text") );
    ReplaceDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "ReplaceDialogLayout"); 

    Layout3 = new QGridLayout( 0, 1, 1, 0, 6, "Layout3"); 

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( i18n( "Replace:" ) );

    Layout3->addWidget( TextLabel2, 1, 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Find:")  );

    Layout3->addWidget( TextLabel1, 0, 0 );

    comboFind = new QComboBox( FALSE, this, "comboFind" );
    comboFind->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, comboFind->sizePolicy().hasHeightForWidth() ) );
    comboFind->setEditable( TRUE );

    Layout3->addWidget( comboFind, 0, 1 );

    comboReplace = new QComboBox( FALSE, this, "comboReplace" );
    comboReplace->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, comboReplace->sizePolicy().hasHeightForWidth() ) );
    comboReplace->setEditable( TRUE );

    Layout3->addWidget( comboReplace, 1, 1 );

    ReplaceDialogLayout->addMultiCellLayout( Layout3, 0, 0, 0, 1 );

    Layout5 = new QHBoxLayout( 0, 0, 6, "Layout5"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout5->addItem( spacer );

    buttonReplace = new QPushButton( this, "buttonReplace" );
    buttonReplace->setText(  i18n("&Find")  );
    buttonReplace->setDefault( TRUE );
    Layout5->addWidget( buttonReplace );

    buttonReplaceAll = new QPushButton( this, "buttonReplaceAll" );
    buttonReplaceAll->setText( i18n("&Replace All")  );
    Layout5->addWidget( buttonReplaceAll );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setText( i18n( "&Close")  );
    Layout5->addWidget( PushButton2 );

    ReplaceDialogLayout->addMultiCellLayout( Layout5, 2, 2, 0, 1 );

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
    checkCase->setText( i18n("Case sensitive") );
    ButtonGroup1Layout->addWidget( checkCase );

    checkBegin = new QCheckBox( ButtonGroup1, "checkBegin" );
    checkBegin->setText( i18n( "Start at beginning")  );
    ButtonGroup1Layout->addWidget( checkBegin );

    ReplaceDialogLayout->addWidget( ButtonGroup1, 1, 0 );

    ButtonGroup2 = new QButtonGroup( this, "ButtonGroup2" );
    ButtonGroup2->setTitle( i18n( "Direction")  );
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 6 );
    ButtonGroup2->layout()->setMargin( 11 );
    ButtonGroup2Layout = new QVBoxLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );

    radioForward = new QRadioButton( ButtonGroup2, "radioForward" );
    radioForward->setText( i18n( "Forward" ) );
    radioForward->setChecked( TRUE );
    ButtonGroup2Layout->addWidget( radioForward );

    radioBackward = new QRadioButton( ButtonGroup2, "radioBackward" );
    radioBackward->setText( i18n( "Backward" ) );
    ButtonGroup2Layout->addWidget( radioBackward );

    ReplaceDialogLayout->addWidget( ButtonGroup2, 1, 1 );

    // signals and slots connections
    connect( buttonReplace, SIGNAL( clicked() ), this, SLOT( doReplace() ) );
    connect( buttonReplaceAll, SIGNAL( clicked() ), this, SLOT( doReplaceAll() ) );
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // tab order
    setTabOrder( comboFind, comboReplace );
    setTabOrder( comboReplace, checkWords );
    setTabOrder( checkWords, checkCase );
    setTabOrder( checkCase, checkBegin );
    setTabOrder( checkBegin, radioForward );
    setTabOrder( radioForward, radioBackward );
    setTabOrder( radioBackward, buttonReplace );
    setTabOrder( buttonReplace, buttonReplaceAll );
    setTabOrder( buttonReplaceAll, PushButton2 );

    // buddies
    TextLabel2->setBuddy( comboReplace );
    TextLabel1->setBuddy( comboFind );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ReplaceDialog::~ReplaceDialog()
{
}

void ReplaceDialog::doReplace()
{
reject();
if ( !editor ) 	return;
bool go=true;
while (go && editor->search( comboFind->currentText(), checkCase->isChecked(),
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked()) )
       {
       switch(  KMessageBox::questionYesNoCancel(this,i18n("Replace this occurrence?"),"Kile") )
         {
         case (KMessageBox::Yes):
         editor->replace(comboReplace->currentText() );
         checkBegin->setChecked( FALSE );
    	   break;
         case (KMessageBox::No):
         checkBegin->setChecked( FALSE );
    	   break;
         case (KMessageBox::Cancel):
         go=false;
    	   break;
         }
       }
if (go) checkBegin->setChecked( TRUE );
}

void ReplaceDialog::doReplaceAll()
{
if ( !editor ) return;
while ( editor->search( comboFind->currentText(), checkCase->isChecked(),
checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked()) )
    {
    editor->replace(comboReplace->currentText() );
    checkBegin->setChecked( FALSE );
    }
checkBegin->setChecked( TRUE );
}

void ReplaceDialog::SetEditor(LatexEditor *ed)
{
editor=ed;
}




#include "replacedialog.moc"
