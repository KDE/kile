#include "filechooser.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kfiledialog.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <klocale.h>

static const char *fileselect_xpm[]={
"22 22 83 2",
"Qt c None",
".b c #000000",
".c c #030303",
".a c #040404",
".V c #060606",
".p c #090909",
".f c #0b0b0b",
".W c #0d0d0d",
".# c #161616",
".q c #171717",
".u c #1e1e1e",
"#. c #1f1f1f",
".r c #222222",
".e c #242424",
".t c #262626",
".s c #272727",
".U c #282828",
".M c #292929",
".G c #2e2e2e",
"#q c #313100",
"#k c #353500",
"#f c #363600",
".v c #373737",
".9 c #383800",
".d c #3c3c3c",
"#p c #3d3d00",
".6 c #404000",
".k c #424242",
"#o c #464600",
"#j c #474700",
".g c #474747",
"#n c #494900",
"#e c #4a4a00",
".2 c #4c4c00",
"#m c #4d4d00",
".5 c #525200",
"#l c #555500",
"#i c #575700",
"#d c #5d5d00",
".1 c #5f5f00",
".8 c #606000",
".4 c #646400",
"#h c #676700",
"#c c #6b6b00",
".7 c #6e6e00",
".0 c #6f6f00",
".3 c #707000",
"#b c #717100",
"#a c #747400",
".Z c #757500",
".Y c #777700",
".X c #787800",
".T c #c2c243",
".L c #c6c665",
".F c #d1d16a",
".S c #d7d74a",
".R c #e4e44f",
".O c #e4e450",
".P c #e4e451",
".Q c #e4e452",
"## c #e5e553",
".K c #e9e976",
"#g c #f2f27b",
".E c #f4f47c",
".z c #f4f47d",
".o c #f4f47e",
".J c #f4f47f",
".j c #f4f480",
".A c #f4f481",
".y c #f5f583",
".N c #f5f584",
".B c #f5f585",
".I c #f5f586",
".n c #f5f587",
".C c #f5f589",
".D c #f5f58a",
".x c #f5f58b",
".i c #f5f58c",
".H c #f6f68e",
".m c #f6f693",
".w c #f6f694",
".h c #f6f699",
".l c #f7f79c",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQt.#.a.bQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt.c.bQt.b.bQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQt.b.bQtQtQt.b.bQtQt.bQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.b.bQt",
"QtQt.d.e.fQtQtQtQtQtQtQtQtQtQtQtQtQt.b.b.bQt",
"Qt.g.h.i.j.bQtQtQtQtQtQtQtQtQtQtQt.b.b.b.bQt",
".k.l.m.n.o.b.p.q.r.s.t.u.f.bQtQtQtQtQtQtQtQt",
".v.w.x.y.o.z.A.B.C.x.D.n.E.F.bQtQtQtQtQtQtQt",
".G.H.I.j.z.z.J.A.y.y.y.j.K.L.bQtQtQtQtQtQtQt",
".M.x.N.J.E.O.O.P.Q.Q.Q.R.S.T.bQtQtQtQtQtQtQt",
".U.x.y.o.E.R.R.b.b.b.b.b.b.b.b.b.V.W.c.b.b.b",
".U.x.y.o.R.R.b.X.X.X.X.X.Y.Z.Z.Y.Y.0.1.2.bQt",
".U.x.y.o.R.b.X.X.X.X.X.X.X.Y.Z.3.4.5.6.bQtQt",
".s.D.y.Q.b.X.X.X.X.X.X.X.X.Z.7.8.2.9.bQtQtQt",
"#..I##.b#a#a#a#a#a#a#a#a#b#c#d#e#f.bQtQtQtQt",
".W#g.b#h.4.4.4.4.4.4.4.4.8#i#j#k.bQtQtQtQtQt",
".b.b#l#m#e#e#e#e#e#e#e#n#o#p#q.bQtQtQtQtQtQt",
"Qt.b.b.b.b.b.b.b.b.b.b.b.b.b.bQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt"};


FileChooser::FileChooser( QWidget *parent, const char *name)
    : QDialog( parent, name,true )
{
   setCaption(name);

   QGridLayout *gbox = new QGridLayout( this, 3, 2,5,5,"" );
   gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );


    lineEdit = new QLineEdit( this, "filechooser_lineedit" );
    lineEdit->setText("");
    lineEdit->setFixedWidth(250);
    gbox->addMultiCellWidget( lineEdit,0,0,0,1,1 );


    button = new QPushButton("", this, "filechooser_button" );
    button->setPixmap(QPixmap(fileselect_xpm));
    button->setFixedWidth(30);

    gbox->addWidget( button,0,2 );

    connect( button, SIGNAL( clicked() ), this, SLOT( chooseFile() ) );

   buttonOk= new QPushButton(this,"NoName");
   buttonOk->setMinimumSize(0,0);
   buttonOk->setText(i18n("Ok"));
   buttonOk->setDefault(true);

   buttonCancel= new QPushButton(this,"NoName");
   buttonCancel->setMinimumSize(0,0);
   buttonCancel->setText(i18n("Cancel"));

   connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
   connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

   gbox->addWidget(buttonOk , 1, 0,Qt::AlignLeft );
   gbox->addWidget(buttonCancel , 1, 1,Qt::AlignRight);

    setFocusProxy( lineEdit );

}

void FileChooser::setDir( const QString &di )
{
    dir=di;
}

void FileChooser::setFilter( const QString &fil )
{
    filter=fil;
}
QString FileChooser::fileName() const
{
    return lineEdit->text();
}

void FileChooser::chooseFile()
{

  QString fn;
	fn = KFileDialog::getOpenFileName( dir,filter, this,i18n("Select a File") );
  if ( !fn.isEmpty() )
     {
     	lineEdit->setText( fn );
     }
}


#include "filechooser.moc"
