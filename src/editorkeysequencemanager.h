/**************************************************************************
*   Copyright (C) 2006-2019 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef EDITORKEYSEQUENCEMANAGER_H
#define EDITORKEYSEQUENCEMANAGER_H

#include <QEvent>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <KTextEditor/View>

namespace KileScript {
class Script;
class Manager;
}

class KileInfo;

namespace KileEditorKeySequence {
/**
 * This class represents an action that can be assigned to an editor key sequence.
 **/
class Action {
public:
    Action();
    virtual ~Action();

    /**
     * The main method, which implements the "action" itself.
     **/
    virtual void execute() = 0;

    /**
     * Returns a textual representation of the action.
     **/
    virtual QString getDescription() const;
};

/**
 * This class represents the execution of a script in Kile.
 **/
class ExecuteScriptAction : public Action {
public:
    ExecuteScriptAction(KileScript::Script *script, KileScript::Manager *scriptManager);
    virtual ~ExecuteScriptAction();

    virtual void execute() override;
    virtual QString getDescription() const override;

protected:
    KileScript::Script *m_script;
    KileScript::Manager *m_scriptManager;
};

// forward declaration
class Recorder;

/**
 * This manager class is responsible for handling the key sequences that get assigned
 * to actions. Currently, every key sequence can only trigger one single action.
 *
 * Whenever a watched key sequence is typed, the manager triggers the corresponding
 * action. The only characters that are allowed in key sequences are those that make
 * the cursor advance by one position, i.e. for example tabs are not allowed in key
 * sequences.
 **/
class Manager : public QObject {
    Q_OBJECT

    friend class Recorder;

public:
    /**
     * Constructs a new manager object.
     **/
    explicit Manager(KileInfo* kileInfo, QObject *parent = 0, const char *name = 0);
    virtual ~Manager();

    /**
     * Adds a new consequence and the corresponding action.
     * @param seq the key sequence
     * @param action the action for the sequence
     **/
    void addAction(const QString& seq, Action *action);

    /**
     * Convenience method. Adds a key sequence-to-action map to this
     * manager, removing any existing mappings.
     * @warning This method overrides any exising mappings !
     **/
    void addActionMap(const QMap<QString, Action*>& map);

    /**
     * Removes all the mappings.
     **/
    void clear();

    /**
     * Returns a list of all the key sequences that are currently being
     * watched.
     **/
    const QStringList& getWatchedKeySequences();

    /**
     * Returns the key sequence that corresponds to an action.
     * @param a the action that is considered
     **/
    QString getKeySequence(const Action* a);

    /**
     * Returns the action that corresponds to a key sequence.
     **/
    Action* getAction(const QString& seq);

    /**
     * Remove a key sequence, i.e. the key sequence is no longer watched.
     * @param seq the key sequence that should be removed
     **/
    void removeKeySequence(const QString& seq);

    /**
     * Convenience method. Removes every key sequence contained in the list.
     * @see removeKeySequence(const QString& seq)
     **/
    void removeKeySequence(const QStringList& l);

    /**
     * @warning not implemented yet !
     **/
    void setEditorKeySequence(const QString& seq, Action *action);

    /**
     * Checks whether the sequence "seq" is already assigned to an action.
     * This method also checks whether a longer sequence that starts with
     * "seq" is assigned to an action.
     * @param seq the sequence that should be checked
     * @return "true" if and only the sequence "seq" or another sequence
     *                that starts with "seq" is assigned to an action
     **/
    bool isSequenceAssigned(const QString& seq) const;

    /**
     * Performs a few checks on a key sequence.
     * @returns in the first component: 0 if the sequence is free; 1
     *          if the sequence is assigned; 2 if there is a longer,
     *          currently stored sequence that starts with "seq"; 3
     *          if "seq" starts with a shorter sequence that is currently
     *          stored
     *
     *          in the second component: a string that corresponds to one
     *          of the previous cases (in the case 0: QString())
     **/
    QPair<int, QString> checkSequence(const QString& seq, const QString& skip = QString());

Q_SIGNALS:
    /**
     * Emitted whenever the set of watched key sequences changes.
     **/
    void watchedKeySequencesChanged();

protected Q_SLOTS:
    /**
     * Signalises to the manager that a (watched) sequence has been typed.
     * @param seq the sequence that has been typed
     **/
    void keySequenceTyped(const QString& seq);

protected:
    KileInfo *m_kileInfo;
    QMap<QString, Action*> m_actionMap;
    QStringList m_watchedKeySequencesList;
};

/**
 * This class keeps track of the characters that are typed. It is used in
 * conjunction with a KTextEditor view and a KileEditorKeySequence::Manager.
 **/
class Recorder : public QObject {
    Q_OBJECT
public:
    Recorder(KTextEditor::View *view, Manager *manager);
    virtual ~Recorder();

Q_SIGNALS:
    /**
     * Emitted whenever a key sequence that is currently watched has
     * been typed.
     **/
    void detectedTypedKeySequence(const QString& seq);


public Q_SLOTS:
    /**
     * Reloads the key sequences that this recorders watches.
     **/
    void reloadWatchedKeySequences();

protected:
    Manager *m_manager;
    QString m_typedSequence;
    int m_maxSequenceLength;
    int m_oldCol, m_oldLine;
    KTextEditor::View* m_view;
    QStringList m_watchedKeySequencesList;

    virtual bool eventFilter(QObject *o, QEvent *e) override;

    /**
     * Checks whether a key sequence is currently watched.
     * @param s the key sequence that should be checked
     **/
    bool seekForKeySequence(const QString& s);
};
}

#endif
