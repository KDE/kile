/*************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILESTDTOOLS_H
#define KILESTDTOOLS_H

#include <QString>

#include "kiledebug.h"
#include "kiletool.h"
#include "tool_utils.h"

class KConfig;
class KActionCollection;

namespace KileTool
{
class View;
class Compile;
class Manager;

class Factory
{
    friend class Manager;

public:
    Factory(Manager *mngr, KConfig *config, KActionCollection *actionCollection);
    ~Factory();

    void resetToolConfigurations();
    void installStandardLivePreviewTools();

private:
    Manager            *m_manager;
    KConfig            *m_config;
    KActionCollection  *m_actionCollection;
    QString             m_standardToolConfigurationFileName;

    // only the 'Manager' is allowed to call this
    Base* create(const QString& tool, const QString& config, bool prepare = true);
};

class LaTeX : public Compile
{
    Q_OBJECT
    friend class KileTool::Factory;

protected:
    LaTeX(const QString& tool, Manager *mngr, bool prepare);
public:
    virtual ~LaTeX();

    void setupAsChildTool(KileTool::Base *child);

    LaTeXOutputHandler* latexOutputHandler();
    void setLaTeXOutputHandler(LaTeXOutputHandler *h);

Q_SIGNALS:
    void jumpToFirstError();

public Q_SLOTS:
    bool finish(int);

protected:
    LaTeXOutputHandler *m_latexOutputHandler;

    virtual bool determineSource();

    void checqCriticals();
    void checkAutoRun();
    void latexOutputParserResultInstalled();

    virtual bool updateBibs(bool checkOnlyBibDependencies);
    virtual bool updateIndex();
    virtual bool updateAsy();

    virtual void configureLaTeX(KileTool::Base *tool, const QString& source);
    virtual void configureBibTeX(KileTool::Base *tool, const QString& source);
    virtual void configureMakeIndex(KileTool::Base *tool, const QString& source);
    virtual void configureAsymptote(KileTool::Base *tool, const QString& source);

    /**
     * @brief Determine the tool name and configuration for the bibliography backend
     *
     * If a backend has been set by the user, that one is returned. Otherwise,
     * an automatic detection of the backend is attempted. If an automatic detection
     * is not possible and no backend has been previously auto-detected, the
     * default tool (BibTex) is returned.
     * @returns Tool name that can be provided to @ref KileTool::Manager::create
     **/
    ToolConfigPair determineBibliographyBackend(const QString& hint);

    //FIXME: this is a little 'hackish'
    static int m_reRun;
};

class PreviewLaTeX : public LaTeX
{
    Q_OBJECT
    friend class KileTool::Factory;

protected:
    PreviewLaTeX(const QString& tool, Manager *mngr, bool prepare);

public:
    void setPreviewInfo(const QString &filename, int selrow, int docrow);

public Q_SLOTS:
    bool finish(int);

private:
    QString m_filename;
    int m_selrow;
    int m_docrow;
};

class LivePreviewLaTeX : public LaTeX
{
    Q_OBJECT
    friend class KileTool::Factory;

protected:
    LivePreviewLaTeX(const QString& tool, Manager *mngr, bool prepare);

public:
// 			void setPreviewInfo(const QString &filename, int selrow, int docrow);

public Q_SLOTS:
// 			bool finish(int);

protected:
    virtual void configureLaTeX(KileTool::Base *tool, const QString& source);
    virtual void configureBibTeX(KileTool::Base *tool, const QString& source);
    virtual void configureMakeIndex(KileTool::Base *tool, const QString& source);
    virtual void configureAsymptote(KileTool::Base *tool, const QString& source);

private:
    QString m_filename;
    int m_selrow;
    int m_docrow;
};

class ForwardDVI : public View
{
    friend class KileTool::Factory;

protected:
    ForwardDVI(const QString & tool, Manager *mngr, bool prepare = true);

    bool determineTarget();
    bool checkPrereqs();
};

class ViewBib : public View
{
    friend class KileTool::Factory;

protected:
    ViewBib(const QString& tool, Manager *mngr, bool prepare = true);

    bool determineSource();
};

class ViewHTML : public View
{
    Q_OBJECT
    friend class KileTool::Factory;

protected:
    ViewHTML(const QString& tool, Manager *mngr, bool prepare = true);

    bool determineTarget();

Q_SIGNALS:
    void updateStatus(bool, bool);
};

class BibliographyCompile : public Compile
{
    friend class KileTool::Factory;

protected:
    BibliographyCompile(const QString& name, Manager* manager, bool prepare = true);
public:
    static const QString ToolClass;
};
}

#endif
