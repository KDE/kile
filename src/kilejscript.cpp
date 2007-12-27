/**************************************************************************
*   Copyright (C) 2006, 2007 by Michel Ludwig (michel.ludwig@kdemail.net) *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kilejscript.h"

/*
 * KJS uses a garbage collection mechanism.
 */
#include <kjs/lookup.h>

#include <kconfig.h>
#include "kiledebug.h"
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <signal.h>
#include <sys/time.h>

#include <qevent.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <q3textstream.h>
#include <QList>

#include "kileconfig.h"
#include "kileedit.h"
#include "kileinfo.h"
#include "kileversion.h"
#include "kileviewmanager.h"
#include "editorkeysequencemanager.h"

#ifdef __GNUC__
#warning PORTING NOT FINISHED!
#endif

// Modified declaration from <khtml/ecma/kjs_proxy.h>
// Acknowledgements go to:
//  Copyright (C) 1999 Harri Porten (porten@kde.org)
//  Copyright (C) 2001 Peter Kelly (pmk@post.com)

class KJSCPUGuard {
public:
  KJSCPUGuard() {}
  void start(unsigned int msec=5000, unsigned int i_msec=0);
  void stop();
private:
  void (*oldAlarmHandler)(int);
  static void alarmHandler(int);
  itimerval oldtv;
};

// Modified implementation originating from <khtml/ecma/kjs_proxy.cpp>
// Acknowledgements go to:
// Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
// Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
// Copyright (C) 2001-2003 David Faure (faure@kde.org)
void KJSCPUGuard::start(unsigned int ms, unsigned int i_ms)
{
  oldAlarmHandler = signal(SIGVTALRM, alarmHandler);
  itimerval tv = {
      { i_ms / 1000, (i_ms % 1000) * 1000 },
      { ms / 1000, (ms % 1000) * 1000 }
  };
  setitimer(ITIMER_VIRTUAL, &tv, &oldtv);
}

void KJSCPUGuard::stop()
{
  setitimer(ITIMER_VIRTUAL, &oldtv, 0L);
  signal(SIGVTALRM, oldAlarmHandler);
}

void KJSCPUGuard::alarmHandler(int) {
#ifdef __GNUC__
#warning "Fix time limit functionality!"
#endif
//     KJS::ExecState::requestTerminate();
}


namespace KJS {
#ifdef __GNUC__
#warning "REMOVE ME once KJS headers get fixed"
#endif

// Taken from <kdelibs/kate/jscript/katejscript.cpp
// Acknowledgements go to:
// Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>
// Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>
// Copyright (C) 2006 Dominik Haumann <dhaumann kde org>
#define KJS_CHECK_THIS( ClassName, theObj ) \
  if (!theObj || !theObj->inherits(&ClassName::info)) { \
    KJS::UString errMsg = "Attempt at calling a function that expects a "; \
    errMsg += ClassName::info.className; \
    errMsg += " on a "; \
    errMsg += theObj->className(); \
    KJS::JSObject* err = KJS::Error::create(exec, KJS::TypeError, errMsg.ascii()); \
    exec->setException(err); \
    return err; \
  }


// Taken from <khtml/ecma/kjs_binding.cpp>
// Acknowledgements go to:
// Copyright (C) 1999-2003 Harri Porten (porten@kde.org)
// Copyright (C) 2001-2003 David Faure (faure@kde.org)
// Copyright (C) 2003 Apple Computer, Inc.
// 	UString::UString(const QString &d) {
// 		unsigned int len = d.length();
// 		UChar *dat = new UChar[len];
// 		memcpy(dat, d.unicode(), len * sizeof(UChar));
// 		rep = UString::Rep::create(dat, len);
// 	}
	
// 	QString UString::qstring() const {
// 		return QString((QChar*) data(), size());
// 	}
// 	
// 	QConstString UString::qconststring() const {
// 		return QConstString((QChar*) data(), size());
// 	}
}
namespace KileJScript {
	class KileJSObjectFunc; // forward declaration
	class KileTextDocumentJSObjectProtoFunc; // forward declaration

class KileJSObject : public KJS::JSObject {
 	friend class KileJSObjectFunc;
	public:
		KileJSObject(KJS::ExecState *exec, KileInfo *kileInfo);
		virtual ~KileJSObject();

// there are no non-functional properties
// 		KJS::Value get(KJS::ExecState *exec, const  KJS::Identifier &propertyName) const;

// 		KJS::Value getValueProperty(KJS::ExecState *exec, int token) const;

//		void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr = KJS::None);

// 		void putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr);

		virtual const KJS::ClassInfo *classInfo() const;

		enum {
			CurrentTextDocument,
			GetInputValue
		};

		static const KJS::ClassInfo info;

		protected:
			KileInfo *m_kileInfo;
};

class KileTextDocumentJSObject : public KJS::JSObject {
 	friend class KileTextDocumentJSObjectProtoFunc;
	public:
		KileTextDocumentJSObject(KJS::ExecState *exec, KTextEditor::View* view, KileInfo *kileInfo);

		virtual ~KileTextDocumentJSObject();

// there are no non-functional properties
// 		KJS::Value get(KJS::ExecState *exec, const  KJS::Identifier &propertyName) const;

// 		KJS::Value getValueProperty(KJS::ExecState *exec, int token) const;

//		void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr = KJS::None);

// 		void putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr);

		virtual const KJS::ClassInfo *classInfo() const;

		enum {
			InsertBullet,
			InsertText,
			NextBullet,
			PreviousBullet,
			CursorLeft,
			CursorRight,
			Up,
			Down,
			CursorLine,
			CursorColumn,
			SetCursorLine,
			SetCursorColumn,
			Backspace
		};


		static const KJS::ClassInfo info;

		protected:
			KTextEditor::View* view;
			KileInfo *m_kileInfo;
};
}

#include "kilejscript.lut.h"


/*
 * The number of supplied arguments is checked automatically.
 */


// Only functional properties:
/*
@begin KileJSObjectProtoTable 2
  currentTextDocument                            KileJSObject::CurrentTextDocument                         DontDelete|Function 0
  getInputValue                                  KileJSObject::GetInputValue                       DontDelete|Function 2
@end
*/

// Non-functional properties go here:
/*
@begin KileJSObjectTable 0
@end
*/
using namespace KileJScript;
namespace KileJScript {
KJS_DEFINE_PROTOTYPE(KileJSObjectProto)
KJS_IMPLEMENT_PROTOFUNC(KileJSObjectFunc)
KJS_IMPLEMENT_PROTOTYPE("KileJSObject", KileJSObjectProto, KileJSObjectFunc)

const KJS::ClassInfo KileJSObject::info = {"KileJSObject", 0, /* &KileJSObjectTable */ NULL, 0};

KileJSObject::KileJSObject(KJS::ExecState *exec, KileInfo *kileInfo) : KJS::JSObject(KileJSObjectProto::self(exec)), m_kileInfo(kileInfo) {
}

KileJSObject::~KileJSObject() {
}

const KJS::ClassInfo* KileJSObject::classInfo() const {
	return &info;
}

/*
KJS::Value KileJSObject::get(KJS::ExecState *exec, const  KJS::Identifier &propertyName) const {
 	return KJS::lookupGetValue<KileJSObject, KJS::ObjectImp>(exec, propertyName, &KileJSObjectTable, this);
}
*/

/*
KJS::Value KileJSObject::getValueProperty(KJS::ExecState *exec, int token) const
{
	if (!m_kileInfo) {
		return KJS::jsUndefined();
	}
	switch (token) {

	}
	return KJS::jsUndefined();
}
*/

KJS::JSValue* KileJSObjectFunc::callAsFunction(KJS::ExecState *exec, KJS::JSObject *thisObj, const KJS::List &args) {
	KJS_CHECK_THIS(KileJSObject, thisObj);
// 	KileJSObject *obj = static_cast<KileJSObject *>(thisObj.imp())->doc;
	KileInfo* kileInfo = static_cast<KileJSObject*>(thisObj)->m_kileInfo;
	if(!kileInfo) {
		return KJS::jsUndefined();
	}
	QString caption, label, value;
	KileTextDocumentJSObject *kileViewJSObject;
	KTextEditor::View *kateView;
	switch (id) {
		case KileJSObject::CurrentTextDocument:
			kateView = kileInfo->viewManager()->currentTextView();
			if(kateView == 0L) {
				return KJS::jsNull();
			}
			else {
#ifdef __GNUC__
#warning Check for garbage collection at line 281!
#endif
				kileViewJSObject = new KileTextDocumentJSObject(exec, kateView, kileInfo);
				return new KJS::JSObject(kileViewJSObject);
			}
		break;
		case KileJSObject::GetInputValue:
			caption = args[0]->toString(exec).qstring();
			label = args[1]->toString(exec).qstring();
			if(caption.isEmpty()) {
				caption = i18n("Enter Value");
			}
			if(label.isEmpty()) {
				label = i18n("Please enter a value");
			}
			value = KInputDialog::getText(caption, label, QString(), 0, 0L);
			return KJS::jsString(KJS::UString(value));
		break;
	}
	return KJS::jsUndefined();
}


// Only functional properties:
/* 
@begin KileTextDocumentJSObjectProtoTable 13
#
  insertText                                 KileTextDocumentJSObject::InsertText                      DontDelete|Function 1
  insertBullet                               KileTextDocumentJSObject::InsertBullet                    DontDelete|Function 0
  nextBullet                                 KileTextDocumentJSObject::NextBullet                      DontDelete|Function 0
  previousBullet                             KileTextDocumentJSObject::PreviousBullet                  DontDelete|Function 0
  cursorLeft                                 KileTextDocumentJSObject::CursorLeft                      DontDelete|Function 0
  cursorRight                                KileTextDocumentJSObject::CursorRight                     DontDelete|Function 0
  up                                         KileTextDocumentJSObject::Up                              DontDelete|Function 0
  down                                       KileTextDocumentJSObject::Down                            DontDelete|Function 0
  cursorLine                                 KileTextDocumentJSObject::CursorLine                      DontDelete|Function 0
  cursorColumn                               KileTextDocumentJSObject::CursorColumn                    DontDelete|Function 0
  setCursorLine                              KileTextDocumentJSObject::SetCursorLine                   DontDelete|Function 1
  setCursorColumn                            KileTextDocumentJSObject::SetCursorColumn                 DontDelete|Function 1
  backspace                                  KileTextDocumentJSObject::Backspace                       DontDelete|Function 0
#
@end
*/

// Non-functional properties go here:
/*
@begin KileTextDocumentJSObjectTable 0
@end
*/


KJS_DEFINE_PROTOTYPE(KileTextDocumentJSObjectProto)
KJS_IMPLEMENT_PROTOFUNC(KileTextDocumentJSObjectProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("KileTextDocumentJSObject", KileTextDocumentJSObjectProto, KileTextDocumentJSObjectProtoFunc)

KileTextDocumentJSObject::KileTextDocumentJSObject(KJS::ExecState *exec, KTextEditor::View* view, KileInfo *kileInfo) : KJS::JSObject(KileTextDocumentJSObjectProto::self(exec)), view(view), m_kileInfo(kileInfo) {
}

KileTextDocumentJSObject::~KileTextDocumentJSObject() {
}

/*
KJS::Value KileTextDocumentJSObject::get(KJS::ExecState *exec, const  KJS::Identifier &propertyName) const {
 	return KJS::lookupGetValue<KileTextDocumentJSObject, KJS::ObjectImp>(exec, propertyName, &KileTextDocumentJSObjectTable, this);
}
*/

/*
KJS::Value KileTextDocumentJSObject::getValueProperty(KJS::ExecState *exec, int token) const {
	return KJS::jsUndefined();
}
*/

KJS::JSValue* KileTextDocumentJSObjectProtoFunc::callAsFunction(KJS::ExecState *exec, KJS::JSObject *thisObj, const KJS::List &args) {
	KJS_CHECK_THIS(KileTextDocumentJSObject, thisObj);

// 	KileJSObject *obj = static_cast<KileJSObject *>(thisObj.imp())->doc;
	KileInfo* kileInfo = static_cast<KileTextDocumentJSObject *>(thisObj)->m_kileInfo;
	KTextEditor::View* view = (static_cast<KileTextDocumentJSObject *>(thisObj))->view;
	uint col, line;
#ifdef __GNUC__
#warning Things left to be ported at line 359!
#endif
//FIXME: port for KDE4
/*
	switch (id) {
		case KileTextDocumentJSObject::InsertText:
			view->insertText(args[0]->toString(exec).qstring());
		break;
		case KileTextDocumentJSObject::InsertBullet:
			kileInfo->editorExtension()->insertBullet(view);
		break;
		case KileTextDocumentJSObject::NextBullet:
			kileInfo->editorExtension()->nextBullet(view);
		break;
		case KileTextDocumentJSObject::PreviousBullet:
			kileInfo->editorExtension()->prevBullet(view);
		break;
		case KileTextDocumentJSObject::CursorRight:
			view->cursorRight();
		break;
		case KileTextDocumentJSObject::CursorLeft:
			view->cursorLeft();
		break;
		case KileTextDocumentJSObject::Up:
			view->up();
		break;
		case KileTextDocumentJSObject::Down:
			view->down();
		break;
		case KileTextDocumentJSObject::CursorLine:
			view->cursorPositionReal(&line, &col);
			return KJS::jsNumber(line);
		break;
		case KileTextDocumentJSObject::CursorColumn:
			view->cursorPositionReal(&line, &col);
			return KJS::jsNumber(col);
		break;
		case KileTextDocumentJSObject::SetCursorLine:
			view->cursorPositionReal(&line, &col);
			view->setCursorPositionReal(args[0]->toUInt32(exec), col);
		break;
		case KileTextDocumentJSObject::SetCursorColumn:
			view->cursorPositionReal(&line, &col);
			view->setCursorPositionReal(line, args[0]->toUInt32(exec));
		break;
		case KileTextDocumentJSObject::Backspace:
			view->backspace();
		break;
	}
*/
	return KJS::jsUndefined();
}


const KJS::ClassInfo* KileTextDocumentJSObject::classInfo() const {
	return &info;
}

const KJS::ClassInfo KileTextDocumentJSObject::info = {"KileTextDocumentJSObject", 0, /* &KileTextDocumentJSObjectTable */ NULL, 0};

} /* namespace */

 namespace KileJScript {

////////////////////////////// JScript //////////////////////////////

/* The IDs of the scripts are used to maintain correct bindings with KAction objects, i.e. for example, we
 * want to make sure the action script_execution_0 always refers to same script (the script with id 0 !), even
 * after reloading all the scripts.
 */

	JScript::JScript(unsigned int id, const QString& file) : m_id(id), m_file(file), m_action(NULL) {
		m_name = KGlobal::dirs()->relativeLocation("appdata", file);
		if(m_name.startsWith("scripts")) {
			m_name = m_name.mid(8); // remove "scripts" + path separator
		}
		if(m_name.endsWith(".js")) { // remove the extension
			m_name = m_name.left(m_name.length() - 3);
		}
	}
	
	QString JScript::getName() const {
		return m_name;
	}

	QString JScript::getCode() const {
		QFile qFile(m_file);
		if(qFile.open(QIODevice::ReadOnly)) {
			Q3TextStream inputStream(&qFile);
// 			inputStream.setEncoding(QTextStream::UnicodeUTF8);
			QString code = inputStream.read();
			qFile.close();
			return code;
		}
		else {
			return QString();
		}
	}

	QString JScript::getFileName() const {
		return m_file;
	}

	unsigned int JScript::getID() const {
		return m_id;
	}

	void JScript::setID(unsigned int id) {
		m_id = id;
	}

	void JScript::setActionObject(KAction* action) {
		m_action = action;
	}

	const KAction* JScript::getActionObject() const {
		return m_action;
	}

	KAction* JScript::getActionObject() {
		return m_action;
	}

	void JScript::setKeySequence(const QString& str) {
		m_keySequence = str;
	}

	QString JScript::getKeySequence() const {
		return m_keySequence;
	}

////////////////////////////// JScriptEnvironment //////////////////////////////

	JScriptEnvironment::JScriptEnvironment(KileInfo *kileInfo) : m_interpreter(new KJS::Interpreter()),
								     m_kileJSObject(new KileJSObject(m_interpreter->globalExec(), kileInfo)), m_kileInfo(kileInfo) {
		// no garbage collection because of an external reference
		m_interpreter->globalObject()->put(m_interpreter->globalExec(), KJS::Identifier("kile"), m_kileJSObject, KJS::DontDelete|KJS::Internal);
	}
	
	
	JScriptEnvironment::~JScriptEnvironment() {
		//kileJSObject->imp() will be deleted during the next garbage cleanup
		delete m_kileJSObject;
		m_interpreter->deref(); // m_interpreter will be deleted by KJS' garbage collection
	}
	
	void JScriptEnvironment::execute(const QString& s) {
		bool useGuard = KileConfig::timeLimitEnabled();
		uint timeLimit = (uint)KileConfig::timeLimit();
		KJSCPUGuard guard;
		if(useGuard) {
			guard.start(timeLimit*1000);
		}
		KJS::Completion completion = m_interpreter->evaluate(QString(), 0, s);
		if(useGuard) {
			guard.stop();
		}
#ifdef __GNUC__
#warning Things left to be ported at line 498!
#endif
//FIXME: port for KDE4
/*
		if(completion.complType() == Throw) {
			KJS::JSValue value = completion.value();
			if(value.type() == ObjectType) {
				KJS::JSObject o = KJS::JSObject::dynamicCast(value);
				if(o.isValid()) {
					JSValue lineValue = o.get(m_interpreter->globalExec(), "line");
					if(lineValue.type() == NumberType) {
						KMessageBox::sorry(0L, i18n("The following exception has occurred at line %1 during execution of the script:\n%2").arg(lineValue.toInt32(m_interpreter->globalExec())).arg(value.toString(m_interpreter->globalExec()).qstring()), i18n("Exception"));
						return;
					}
				}
			}
			KMessageBox::sorry(0L, i18n("The following exception has occurred during execution of the script:\n%1").arg(value.toString(m_interpreter->globalExec()).qstring()), i18n("Exception"));
		}
*/
	}

////////////////////////////// Manager //////////////////////////////

	Manager::Manager(KileInfo *kileInfo, KConfig *config, KActionCollection *actionCollection, QObject *parent, const char *name)  : QObject(parent, name), m_jScriptDirWatch(NULL), m_kileInfo(kileInfo), m_config(config), m_actionCollection(actionCollection) {
		// create a local scripts directory if it doesn't exist yet
		m_localJScriptDir = KStandardDirs::locateLocal("appdata", "scripts/", true);
		m_jScriptDirWatch = new KDirWatch(this);
		m_jScriptDirWatch->setObjectName("KileJScript::Manager::JScriptDirWatch");
		connect(m_jScriptDirWatch, SIGNAL(dirty(const QString&)), this, SLOT(scanJScriptDirectories()));
		connect(m_jScriptDirWatch, SIGNAL(created(const QString&)), this, SLOT(scanJScriptDirectories()));
		connect(m_jScriptDirWatch, SIGNAL(deleted(const QString&)), this, SLOT(scanJScriptDirectories()));
 		m_jScriptDirWatch->startScan();
	}

	Manager::~Manager() {
		delete m_jScriptDirWatch;

		//still need to delete the scripts
		for(QList<JScript*>::iterator it = m_jScriptList.begin(); it != m_jScriptList.end(); ++it) {
			delete *it;
		}
	}

	void Manager::executeJScript(const JScript *script) {
		JScriptEnvironment env(m_kileInfo);
// demonstrate repainting bug:
/*KMessageBox::information(0L, "works!!!");
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();*/
		QString code = script->getCode();
		QRegExp endOfLineExp("(\r\n)|\n|\r");
		int i = code.indexOf(endOfLineExp);
		QString firstLine = (i >= 0 ? code.left(i) : code);
		QRegExp requiredVersionTagExp("(kile-version:\\s*)(\\d+\\.\\d+(.\\d+)?)");
		if(requiredVersionTagExp.search(firstLine) != -1) {
			QString requiredKileVersion = requiredVersionTagExp.cap(2);
			if(compareVersionStrings(requiredKileVersion, kileFullVersion) > 0) {
				KMessageBox::sorry(0L, i18n("Version %1 of Kile is at least required to execute the script \"%2\". The execution has been aborted.").arg(requiredKileVersion).arg(script->getName()), i18n("Version Error"));
				return;
			}
		}
		env.execute(code);
	}

	void Manager::executeJScript(unsigned int id) {
		QMap<unsigned int, JScript*>::iterator i = m_idScriptMap.find(id);
		if(i != m_idScriptMap.end()) {
			executeJScript(*i);
		}
	}

	const JScript* Manager::getScript(unsigned int id) {
		QMap<unsigned int, JScript*>::iterator i = m_idScriptMap.find(id);
		return ((i != m_idScriptMap.end()) ? (*i) : NULL);
	}

	void Manager::scanJScriptDirectories() {
		if(!KileConfig::scriptingEnabled()) {
			return;
		}
		deleteJScripts();
		populateDirWatch();

		KConfigGroup configGroup = m_config->group("Scripts");
		QList<unsigned int> idList = configGroup.readEntry("IDs", QList<unsigned int>());
		unsigned int maxID = 0;
		QMap<QString, unsigned int> pathIDMap;
		QMap<unsigned int, bool> takenIDMap;
		for(QList<unsigned int>::iterator i = idList.begin(); i != idList.end(); ++i) {
			QString fileName = configGroup.readPathEntry("Script" + QString::number(*i), QString());
			if(!fileName.isEmpty()) {
				unsigned int id = *i;
				pathIDMap[fileName] = id;
				takenIDMap[id] = true;
				maxID = qMax(maxID, id);
			}
		}

		QStringList scriptFileNamesList = KGlobal::dirs()->findAllResources("appdata", "scripts/*.js", KStandardDirs::Recursive | KStandardDirs::NoDuplicates);
		for(QStringList::iterator i = scriptFileNamesList.begin(); i != scriptFileNamesList.end(); ++i) {
			registerScript(*i, pathIDMap, takenIDMap, maxID);
		}
		//rewrite the IDs that are currently in use
		writeIDs();
		m_actionCollection->readSettings();
		emit jScriptsChanged();
	}

	void Manager::deleteJScripts() {
		QList<JScript*> scriptList = m_jScriptList;
		m_jScriptList.clear(); // pretend that there are no scripts left
		QStringList keySequenceList;
		for(QList<JScript*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
			keySequenceList.push_back((*it)->getKeySequence());
		}
		m_idScriptMap.clear();
		m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequenceList);
		for(QList<JScript*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
			KAction *action = (*it)->getActionObject();
			if(action) {
				foreach (QWidget *w, action->associatedWidgets()) {
					w->removeAction(action);
				}
				delete action;
			}
			delete *it;
		}
		emit jScriptsChanged();
	}

	QList<JScript*> Manager::getJScripts() {
		return m_jScriptList;
	}

	void Manager::registerScript(const QString& fileName, QMap<QString, unsigned int>& pathIDMap, QMap<unsigned int, bool>& takenIDMap, unsigned int &maxID) {
		unsigned int id;
		QMap<QString, unsigned int>::iterator it = pathIDMap.find(fileName);
		if(it != pathIDMap.end()) {
			id = *it;
		}
		else {
			id = findFreeID(takenIDMap, maxID);
			pathIDMap[fileName] = id;
			takenIDMap[id] = true;
			maxID = qMax(maxID, id);
		}
		JScript* script = new JScript(id, fileName);
		m_jScriptList.push_back(script);
		m_idScriptMap[id] = script;
		// start with setting up the key sequence
		KConfigGroup configGroup = m_config->group("Scripts");
		QString editorKeySequence = configGroup.readEntry("Script" + QString::number(id) + "KeySequence");
		if(!editorKeySequence.isEmpty()) {
			script->setKeySequence(editorKeySequence);
			m_kileInfo->editorKeySequenceManager()->addAction(editorKeySequence, new KileEditorKeySequence::ExecuteJScriptAction(script, this));
		}
		// now set up a regular action object
		ScriptExecutionAction *action = new ScriptExecutionAction(id, this, m_actionCollection);
		script->setActionObject(action);
	}

	void Manager::writeConfig() {
		// don't delete the key sequence settings if scripting has been disabled
		if(!KileConfig::scriptingEnabled()) {
			return;
		}
		m_config->deleteGroup("Scripts");
		writeIDs();
		// write the key sequences
		KConfigGroup configGroup = m_config->group("Scripts");
		for(QList<JScript*>::iterator i = m_jScriptList.begin(); i != m_jScriptList.end(); ++i) {
			configGroup.writeEntry("Script" + QString::number((*i)->getID()) + "KeySequence", (*i)->getKeySequence());
		}
	}

	void Manager::setEditorKeySequence(JScript* script, const QString& keySequence) {
		if(keySequence.isEmpty()) {
			return;
		}
		if(script) {
			QString oldSequence = script->getKeySequence();
			if(oldSequence == keySequence) {
				return;
			}
			m_kileInfo->editorKeySequenceManager()->removeKeySequence(oldSequence);
			script->setKeySequence(keySequence);
			m_kileInfo->editorKeySequenceManager()->addAction(keySequence, new KileEditorKeySequence::ExecuteJScriptAction(script, this));
			writeConfig();
		}
	}

	void Manager::removeEditorKeySequence(JScript* script) {
		if(script) {
			QString keySequence = script->getKeySequence();
			if(keySequence.isEmpty()) {
				return;
			}
			script->setKeySequence(QString());
			m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequence);
			writeConfig();
		}
	}

	void Manager::populateDirWatch() {
		QStringList jScriptDirectories = KGlobal::dirs()->findDirs("appdata", "scripts");
		for(QStringList::iterator i = jScriptDirectories.begin(); i != jScriptDirectories.end(); ++i) {
			// FIXME: future KDE versions could support the recursive
			//        watching of directories out of the box.
			addDirectoryToDirWatch(*i);
		}
		//we do not remove the directories that were once added as this apparently causes some strange
		//bugs (on KDE 3.5.x)
	}

	QString Manager::getLocalJScriptDirectory() const {
		return m_localJScriptDir;
	}

	void Manager::readConfig() {
		deleteJScripts();
		scanJScriptDirectories();
	}

	unsigned int Manager::findFreeID(const QMap<unsigned int, bool>& takenIDMap, unsigned int maxID) {
		if(takenIDMap.size() == 0) {
			return 0;
		}
		// maxID should have a real meaning now 
		for(unsigned int i = 0; i < maxID; ++i) {
			if(takenIDMap.find(i) == takenIDMap.end()) {
				return i;
			}
		}
		return (maxID + 1);
	}

	void Manager::writeIDs() {
		KConfigGroup configGroup = m_config->group("Scripts");
		//delete old entries
		QList<unsigned int> idList = configGroup.readEntry("IDs", QList<unsigned int>());
		for(QList<unsigned int>::iterator i = idList.begin(); i != idList.end(); ++i) {
			configGroup.deleteEntry("Script" + QString::number(*i));
		}
		//write new ones
		idList.clear();
		for(QMap<unsigned int, JScript*>::iterator i = m_idScriptMap.begin(); i != m_idScriptMap.end(); ++i) {
			unsigned int id = i.key();
			idList.push_back(id);
			configGroup.writePathEntry("Script" + QString::number(id), (*i)->getFileName());
		}
		configGroup.writeEntry("IDs", idList);
	}

	void Manager::addDirectoryToDirWatch(const QString& dir) {
		//FIXME: no recursive watching and no watching of files as it isn't implemented
		//       yet
		//FIXME: check for KDE4
		if(!m_jScriptDirWatch->contains(dir)) {
			m_jScriptDirWatch->addDir(dir,  KDirWatch::WatchDirOnly);
		}
		QDir qDir(dir);
		QStringList list = qDir.entryList(QDir::Dirs);
		for(QStringList::iterator i = list.begin(); i != list.end(); ++i) {
			QString subdir = *i;
			if(subdir != "." && subdir != "..") {
				addDirectoryToDirWatch(qDir.filePath(subdir));
			}
		}
	}
////////////////////////////// ScriptExecutionAction //////////////////////////////

	ScriptExecutionAction::ScriptExecutionAction(unsigned int id, KileJScript::Manager *manager, QObject* parent) : KAction(parent), m_manager(manager), m_id(id) {
		const KileJScript::JScript *script = m_manager->getScript(m_id);
		Q_ASSERT(script);
		setText(i18n("Execution of %1").arg(script->getName())); 
		connect(this, SIGNAL(activated()), this, SLOT(executeScript()));
	}

	ScriptExecutionAction::~ScriptExecutionAction()	{
	}

	void ScriptExecutionAction::executeScript() {
		m_manager->executeJScript(m_id);
	}

};

#include "kilejscript.moc"
