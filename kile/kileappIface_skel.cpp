/****************************************************************************
**
** DCOP Skeleton created by dcopidl2cpp from kileappIface.kidl
**
** WARNING! All changes made in this file will be lost!
**
*****************************************************************************/

#include "kileappIface.h"

#include <kdatastream.h>
#include <kurl.h>


static const char* const KileAppDCOPIface_ftable[3][3] = {
    { "Kate::View*", "load(KURL,QString)", "load(KURL f, QString encoding)" },
    { "void", "setLine(QString)", "setLine(QString line)" },
    { 0, 0, 0 }
};

bool KileAppDCOPIface::process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData)
{
    if ( fun == KileAppDCOPIface_ftable[0][1] ) { // void load(QString)
	QString arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = KileAppDCOPIface_ftable[0][0]; 
	load(KURL(arg0),QString::null);
    } else if ( fun == KileAppDCOPIface_ftable[1][1] ) { // void setLine(QString)
	QString arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = KileAppDCOPIface_ftable[1][0]; 
	setLine(arg0 );
    } else {
	return DCOPObject::process( fun, data, replyType, replyData );
    }
    return TRUE;
}

QCStringList KileAppDCOPIface::interfaces()
{
    QCStringList ifaces = DCOPObject::interfaces();
    ifaces += "KileAppDCOPIface";
    return ifaces;
}

QCStringList KileAppDCOPIface::functions()
{
    QCStringList funcs = DCOPObject::functions();
    for ( int i = 0; KileAppDCOPIface_ftable[i][2]; i++ ) {
	QCString func = KileAppDCOPIface_ftable[i][0];
	func += ' ';
	func += KileAppDCOPIface_ftable[i][2];
	funcs << func;
    }
    return funcs;
}


