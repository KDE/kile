
#include "gotolinedialog.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>

GotoLineDialog::GotoLineDialog(QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "GotoLineDialog" );
    resize( 243, 85 );
    setCaption(  i18n("Goto Line")  );
    GotoLineDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "GotoLineDialogLayout");

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Line:")  );

    GotoLineDialogLayout->addWidget( TextLabel1, 0, 0 );

    spinLine = new QSpinBox( this, "spinLine" );
    spinLine->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, spinLine->sizePolicy().hasHeightForWidth() ) );

    GotoLineDialogLayout->addWidget( spinLine, 0, 1 );

    Layout1 = new QHBoxLayout( GotoLineDialogLayout, 6, "Layout1");
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setText( i18n( "&Goto")  );
    PushButton2->setDefault( TRUE );
    Layout1->addWidget( PushButton2 );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setText( i18n("&Close")  );
    Layout1->addWidget( PushButton1 );

    GotoLineDialogLayout->addMultiCellLayout( Layout1, 2, 2, 0, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GotoLineDialogLayout->addItem( spacer_2, 1, 1 );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( gotoLine() ) );

    // buddies
    TextLabel1->setBuddy( spinLine );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
GotoLineDialog::~GotoLineDialog()
{

}
void GotoLineDialog::gotoLine()
{
    if ( editor )
    {
    editor->gotoLine( spinLine->value() - 1 );
    editor->setFocus();
    }
    accept();
}
void GotoLineDialog::SetEditor(LatexEditor *ed)
{
editor=ed;
}

#include "gotolinedialog.moc"
