/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QObject>

class Kile;
class KileInfo;

namespace KileConfiguration {

/**
 * This class handles the configuration management in Kile.
 **/
class Manager : public QObject {
    friend class ::Kile;

    Q_OBJECT

public:
    /**
     * Constructs a new Manager object.
     **/
    explicit Manager(KileInfo *info, QObject *parent = Q_NULLPTR, const char *name = Q_NULLPTR);
    virtual ~Manager();

Q_SIGNALS:
    /**
    * This signal is emitted when the configuration has changed. Classes that read and write to the global KConfig object
    * should connect to this signal so they can update their settings.
    **/
    void configChanged();

protected:
    KileInfo *m_kileInfo;

    void emitConfigChanged();

};

}

#endif


