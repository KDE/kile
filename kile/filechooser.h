#ifndef FILECHOOSER_H
#define FILECHOOSER_H

#include <qdialog.h>

class QLineEdit;
class QPushButton;

class FileChooser : public QDialog
{
    Q_OBJECT

 public:
    FileChooser( QWidget *parent = 0, const char *name = 0, const QString &caption = QString::null);

    QString fileName() const;
    QString filter,dir;

public slots:
    void setDir( const QString &di );
    void setFilter( const QString &fil );

private slots:
    void chooseFile();

private:
    QLineEdit *lineEdit;
    QPushButton *button;
	  QPushButton *buttonOk;
	  QPushButton *buttonCancel;
};

#endif
