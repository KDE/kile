//
// C++ Interface: kileeventfilter
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KILEEVENTFILTER_H
#define KILEEVENTFILTER_H

#include <qobject.h>
#include <qregexp.h>

class QEvent;

namespace Kate {class View; }

/**
 * This class is capable of intercepting key-strokes from the editor. It can complete a \begin{env}
 * with a \end{env} when enter is pressed.
 **/
class KileEventFilter : public QObject
{
	Q_OBJECT

public:
	KileEventFilter(KileDocument::EditorExtension *edit);

public slots:
	void readConfig();

protected:
	bool eventFilter(QObject *o, QEvent *e);

private:
	bool    m_bCompleteEnvironment;
	KileDocument::EditorExtension *m_edit;

};

#endif
