/**************************************************************************
 *   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)  *
 *             (C) 2022 by Michel Ludwig (michel.ludwig@kdemail.net)      *
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilestdactions.h"

#include <KActionMenu>
#include <KConfig>
#include <KLocalizedString>
#include <KMainWindow>

#include "kileactions.h"
#include "editorextension.h"
#include "utilities.h"

namespace KileStdActions
{

void setupStdTags(KileInfo *ki, const QObject* receiver, KActionCollection *actionCollection, QWidget *parentWidget)
{
    (void) new KileAction::Tag(i18n("Document Class Selection - \\documentclass{}"), i18n("Document Class"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_documentclass",
                               "\\documentclass[10pt]{","}", 21, 0, i18n("\\documentclass[options]{class}\nclass : article,report,book,letter\nsize options : 10pt, 11pt, 12pt\npaper size options: a4paper, a5paper, b5paper, letterpaper, legalpaper, executivepaper\n"
                                       "other options: \nlandscape -- selects landscape format; default is portrait. \ntitlepage, notitlepage -- selects if there should be a separate title page.\nleqno -- display equation number on left side of equations; default is right side.\n"
                                       "fleqn -- display formulae flush left; default is centered.\nonecolumn, twocolumn -- one or two columns; defaults to one column\noneside, twoside -- selects one- or two-sided layout.\n" ));

    (void) new KileAction::Tag(i18n("Package Import - \\usepackage{}"), i18n("Package Import"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_usepackage",
                               "\\usepackage{","}", 12, 0, i18n("Any options given in the \\documentclass command that are unknown by the selected document class\n"
                                       "are passed on to the packages loaded with \\usepackage."));

    (void) new KileAction::Tag(i18n("AMS Packages"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_amspackages","\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n",QString(),0,3,i18n("The principal American Mathematical Society packages"));
    (void) new KileAction::Tag(i18n("Start Document Body - \\begin{document}"), i18n("Start Document Body"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_document","\\begin{document}\n", "\n\\end{document}", 0,1,i18n("Text is allowed only between \\begin{document} and \\end{document}.\nThe 'preamble' (before \\begin{document} ) may contain declarations only."));
    (void) new KileAction::Tag(i18n("Generate Title - \\maketitle"), i18n("Generate Title"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_maketitle","\\maketitle",QString(),10,0,i18n("This command generates a title on a separate title page\n- except in the article class, where the title normally goes at the top of the first page."));
    (void) new KileAction::Tag(i18n("Table of Contents - \\tableofcontents"), i18n("Table of Contents"), "view-table-of-contents-ltr", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_tableofcontents","\\tableofcontents",QString(),16,0,i18n("Put this command where you want the table of contents to go"));
    (void) new KileAction::Tag(i18n("Title Definition - \\title{}"), i18n("Title Definition"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_title","\\title{","}",7,0,i18n( "\\title{text}\nThe \\title command declares text to be the title.\nUse \\\\ to tell LaTeX where to start a new line in a long title."));
    (void) new KileAction::Tag(i18n("Author Definition - \\author{}"), i18n("Author Definition"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_author","\\author{","}",8,0,i18n( "\\author{names}\nThe \\author command declares the author(s), where names is a list of authors separated by \\and commands."));

    (void) new KileAction::Tag(i18n("Center - \\begin{center}"), i18n("Center"), "format-justify-center", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_center", "\\begin{center}\n","%E\n\\end{center}", 0,1, i18n("Each line must be terminated with the string \\\\."));
    (void) new KileAction::Tag(i18n("Align Left - \\begin{flushleft}"), i18n("Align Left"), "format-justify-left", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_flushleft", "\\begin{flushleft}\n","%E\n\\end{flushleft}", 0,1, i18n("Each line must be terminated with the string \\\\.") );
    (void) new KileAction::Tag(i18n("Align Right - \\begin{flushright}"), i18n("Align Right"), "format-justify-right", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_flushright", "\\begin{flushright}\n","%E\n\\end{flushright}", 0,1, i18n("Each line must be terminated with the string \\\\.") );
    (void) new KileAction::Tag(i18n("Quote - \\begin{quote}"), i18n("Quote"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_quote","\\begin{quote}\n","%E\n\\end{quote} ",0,1,i18n("The text is justified at both margins.\nLeaving a blank line between text produces a new paragraph.") );
    (void) new KileAction::Tag(i18n("Quotation - \\begin{quotation}"), i18n("Quotation"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_quotation","\\begin{quotation}\n","%E\n\\end{quotation} ",0,1, i18n("The text is justified at both margins and there is paragraph indentation.\nLeaving a blank line between text produces a new paragraph.") );
    (void) new KileAction::Tag(i18n("Verse - \\begin{verse}"), i18n("Verse"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_verse", "\\begin{verse}\n","%E\n\\end{verse} ",0,1,i18n("The verse environment is designed for poetry.\nSeparate the lines of each stanza with \\\\, and use one or more blank lines to separate the stanzas.") );

    (void) new KileAction::Tag(i18n("Verbatim - \\begin{verbatim}"), i18n("Verbatim"), "verbatim", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_verbatim","\\begin{verbatim}\n","%E\n\\end{verbatim} ",0,1,i18n("Environment that gets LaTeX to print exactly what you type in."));
    (void) new KileAction::Tag(i18n("Bulleted List - \\begin{itemize}"), i18n("Bulleted List"), "itemize", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_itemize","\\begin{itemize}\n%E\\item \n", "\\end{itemize}\n", 6,1,i18n("The itemize environment produces a 'bulleted' list.\nEach item of an itemized list begins with an \\item command."));
    (void) new KileAction::Tag(i18n("Enumeration - \\begin{enumerate}"), i18n("Enumeration"), "enumerate", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_enumerate","\\begin{enumerate}\n%E\\item \n","\\end{enumerate}\n", 6,1,i18n("The enumerate environment produces a numbered list.\nEach item of an enumerated list begins with an \\item command."));
    (void) new KileAction::Tag(i18n("Description - \\begin{description}"), i18n("Description"), "description", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_description","\\begin{description}\n%E\\item[] \n", "\\end{description}",6,1,i18n("The description environment is used to make labeled lists.\nEach item of the list begins with an \\item[label] command.\nThe 'label' is bold face and flushed right."));

    (void) new KileAction::Tag(i18n("Table - \\begin{table}"), i18n("Table"), "table-env", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_table","\\begin{table}\n","%E\n\\caption{}\n\\end{table} ",0,1,
                               i18n("\\begin{table}[placement]\nbody of the table\n\\caption{table title}\n\\end{table}\nTables are objects that are not part of the normal text, and are usually floated to a convenient place.\n"
                                    "The optional argument [placement] determines where LaTeX will try to place your table\nh : Here - at the position in the text where the table environment appears\nt : Top - at the top of a text page\nb : Bottom - at the bottom of a text page\n"
                                    "p : Page of floats - on a separate float page, which is a page containing no text, only floats.\nThe body of the table is made up of whatever text or LaTeX commands, etc., you wish.\nThe \\caption command allows you to title your table."));

    (void) new KileAction::Tag(i18n("Figure - \\begin{figure}"), i18n("Figure"), "figure-env", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_figure","\\begin{figure}\n","%E\n\\caption{}\n\\end{figure} ",0,1,
                               i18n("\\begin{figure}[placement]\nbody of the figure\n\\caption{figure title}\n\\end{figure}\nFigures are objects that are not part of the normal text, and are usually floated to a convenient place.\n"
                                    "The optional argument [placement] determines where LaTeX will try to place your figure\nh : Here - at the position in the text where the figure environment appears\nt : Top - at the top of a text page\n"
                                    "b : Bottom - at the bottom of a text page\np : Page of floats - on a separate float page, which is a page containing no text, only floats.\nThe body of the figure is made up of whatever text or LaTeX commands, etc., you wish.\nThe \\caption command allows you to title your figure."));

    (void) new KileAction::Tag(i18n("Title Page - \\begin{titlepage}"), i18n("Title Page"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_titlepage","\\begin{titlepage}\n","%E\n\\end{titlepage} ",0,1,
                               i18n("\\begin{titlepage}\ntext\n\\end{titlepage}\nThe titlepage environment creates a title page, i.e. a page with no printed page number or heading."));

    new KileAction::Tag(i18n("Italics - \\textit{}"), i18n("Italics"), "format-text-italic", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_I), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_textit","\\textit{","}",8,0,i18n("\\textit{italic text}"));
    new KileAction::Tag(i18n("Slanted - \\textsl{}"), i18n("Slanted"), "slanted", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_A), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_textsl","\\textsl{","}",8,0,i18n("\\textsl{slanted text}"));
    new KileAction::Tag(i18n("Boldface - \\textbf{}"), i18n("Boldface"), "format-text-bold", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_B), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_textbf","\\textbf{","}",8,0,i18n("\\textbf{boldface text}"));
    new KileAction::Tag(i18n("Typewriter - \\texttt{}"), i18n("Typewriter"), "typewriter", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_T), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_texttt","\\texttt{","}",8,0,i18n("\\texttt{typewriter text}"));
    new KileAction::Tag(i18n("Small Caps - \\textsc{}"), i18n("Small Caps"), QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_C), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_textsc","\\textsc{","}",8,0,i18n("\\textsc{small caps text}"));
    new KileAction::Tag("\\item", QString(), "item", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_H), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_item","\\item ",QString(),6,0, i18n("\\item[label] Hello!"));

    (void) new KileAction::Tag(i18n("Tabbing - \\begin{tabbing}"), i18n("Tabbing"), "tabbing", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_tabbing","\\begin{tabbing}\n","%E\n\\end{tabbing} ",0,1,i18n("The tabbing environment provides a way to align text in columns.\n\\begin{tabbing}\ntext \\= more text \\= still more text \\= last text \\\\\nsecond row \\&gt;  \\&gt; more \\\\\n\\end{tabbing}\nCommands :\n\\=  Sets a tab stop at the current position.\n\\>  Advances to the next tab stop.\n\\&lt;  Allows you to put something to the left of the local margin without changing the margin. Can only be used at the start of the line.\n\\+  Moves the left margin of the next and all the following commands one tab stop to the right\n\\-  Moves the left margin of the next and all the following commands one tab stop to the left\n\\' Moves everything that you have typed so far in the current column to the right of the previous column, flush against the current column's tab stop. \n\\`  Allows you to put text flush right against any tab stop, including tab stop 0\n\\kill  Sets tab stops without producing text.\n\\a  In a tabbing environment, the commands \\=, \\' and \\` do not produce accents as normal. Instead, the commands \\a=, \\a' and \\a` are used."));
    (void) new KileAction::Tag(i18n("Tabular - \\begin{tabular}"), i18n("Tabular"), "tabular", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_tabular","\\begin{tabular}{","}\n%E\n\\end{tabular} ",16,0,i18n("\\begin{tabular}[pos]{cols}\ncolumn 1 entry & column 2 entry ... & column n entry \\\\\n...\n\\end{tabular}\npos : Specifies the vertical position; default is alignment on the center of the environment.\n     t - align on top row\n     b - align on bottom row\ncols : Specifies the column formatting.\n     l - A column of left-aligned items.\n     r - A column of right-aligned items.\n     c - A column of centered items.\n     | - A vertical line the full height and depth of the environment.\n     @{text} - this inserts text in every row.\nThe \\hline command draws a horizontal line the width of the table.\nThe \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j.\nThe \\vline command draws a vertical line extending the full height and depth of its row."));
    (void) new KileAction::Tag(i18n("Multicolumn Cells - \\multicolumn"), i18n("Multicolumn Cells"), "multicolumn", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)),actionCollection,"tag_multicolumn","\\multicolumn{","}{}{} ",13,0,i18n("\\multicolumn{cols}{pos}{text}\ncol, specifies the number of columns to span.\npos specifies the formatting of the entry: c for centered, l for flushleft, r for flushright.\ntext specifies what text is to make up the entry."));
    (void) new KileAction::Tag(i18n("Horizontal Line - \\hline"), i18n("Horizontal Line"), "hline", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_hline","\\hline ",QString(),7,0,i18n("The \\hline command draws a horizontal line the width of the table."));
    (void) new KileAction::Tag(i18n("Vertical Line - \\vline"), i18n("Vertical Line"), "vline", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_vline","\\vline ",QString(),7,0,i18n("The \\vline command draws a vertical line extending the full height and depth of its row."));
    (void) new KileAction::Tag(i18n("Horizontal Line Across Columns - \\cline{m-n}"), i18n("Horizontal Line Across Columns"), "cline", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_cline","\\cline{-} ",QString(),7,0,i18n("The \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j,"));

    (void) new KileAction::Tag(i18n("New Page - \\newpage"), i18n("New Page"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_newpage","\\newpage ",QString(),9,0,i18n("The \\newpage command ends the current page"));
    (void) new KileAction::Tag(i18n("Line Break - \\linebreak"), i18n("Line Break"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_linebreak","\\linebreak ",QString(),11,0,i18n("The \\linebreak command tells LaTeX to break the current line at the point of the command."));
    (void) new KileAction::Tag(i18n("Page Break - \\pagebreak"), i18n("Page Break"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_pagebreak","\\pagebreak ",QString(),11,0,i18n("The \\pagebreak command tells LaTeX to break the current page at the point of the command."));
    (void) new KileAction::Tag(i18n("\"Big\" Vertical Space - \\bigskip"), i18n("\"Big\" Vertical Space"), "bigskip", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bigskip","\\bigskip ",QString(),9,0,i18n("The \\bigskip command adds a 'big' vertical space."));
    (void) new KileAction::Tag(i18n("\"Medium\" Vertical Space - \\medskip"), i18n("\"Medium\" Vertical Space"), "medskip", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_medskip","\\medskip ",QString(),9,0,i18n("The \\medskip command adds a 'medium' vertical space."));

    // includegraphics (dani)
    (void) new KileAction::Tag(i18n("Image Insertion - \\includegraphics{file}"), i18n("Image Insertion"), "insert-image", QKeySequence("Alt+I, G"),receiver,SLOT(includeGraphics()), actionCollection,"tag_includegraphics",0L);
    // two new shortcuts (dani)
    (void) new KileAction::InputTag(ki, i18n("Customizable File Inclusion - \\include{file}"), i18n("Customizable File Inclusion"), "include-file", QKeySequence("Alt+I, F"), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_include", parentWidget, KileAction::KeepHistory | KileAction::ShowBrowseButton | KileAction::AddProjectFile, "\\include{%R","}",9,0, i18n("\\include{file}\nThe \\include command is used in conjunction with the \\includeonly command for selective inclusion of files."),i18n("Type or select a filename: "));
    (void) new KileAction::InputTag(ki, i18n("File Inclusion - \\input{file}"), i18n("File Inclusion"), "input-file", QKeySequence("Alt+I, P"), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_input", parentWidget, KileAction::KeepHistory | KileAction::ShowBrowseButton | KileAction::AddProjectFile, "\\input{%R","}",7,0,i18n("\\input{file}\nThe \\input command causes the indicated file to be read and processed, exactly as if its contents had been inserted in the current file at that point."),i18n("Type or select a filename: "));

    ToolbarSelectAction *actionstructure_list = new ToolbarSelectAction(i18n("Sectioning"), actionCollection,false);
    actionCollection->addAction("structure_list", actionstructure_list);
    actionstructure_list->addAction(new KileAction::InputTag(ki,"&part", QString(), "part", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_part", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\part%A{%R}","\n", 0,1,i18n("\\part{title}\n\\part*{title} : do not include a number and do not make an entry in the table of contents\n"), i18n("&Part"),i18n("No &numbering")));
    actionstructure_list->addSeparator();
    actionstructure_list->addAction(new KileAction::InputTag(ki,"&chapter", QString(), "chapter", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_chapter", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\chapter%A{%R}","\n", 0,1,i18n("\\chapter{title}\n\\chapter*{title} : do not include a number and do not make an entry in the table of contents\nOnly for 'report' and 'book' class document."), i18n("C&hapter"),i18n("No &numbering")));
    actionstructure_list->addAction(new KileAction::InputTag(ki,"&section", QString(), "section", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_section", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\section%A{%R}","\n", 0,1,i18n("\\section{title}\n\\section*{title} : do not include a number and do not make an entry in the table of contents"), i18n("&Section"),i18n("No &numbering")));
    actionstructure_list->addAction(new KileAction::InputTag(ki,"s&ubsection", QString(), "subsection", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_subsection", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\subsection%A{%R}","\n", 0,1,i18n("\\subsection{title}\n\\subsection*{title} : do not include a number and do not make an entry in the table of contents"), i18n("&Subsection"),i18n("No &numbering")));
    actionstructure_list->addAction(new KileAction::InputTag(ki,"su&bsubsection", QString(), "subsubsection", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_subsubsection", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\subsubsection%A{%R}","\n", 0,1,i18n("\\subsubsection{title}\n\\subsubsection*{title} : do not include a number and do not make an entry in the table of contents"), i18n("&Subsubsection"),i18n("No &numbering")));
    actionstructure_list->addSeparator();
    actionstructure_list->addAction(new KileAction::InputTag(ki,"p&aragraph", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_paragraph", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\paragraph%A{%R}","\n", 0,1,i18n("\\paragraph{title}\n\\paragraph*{title} : do not include a number and do not make an entry in the table of contents"), i18n("&Paragraph"),i18n("No &numbering")));
    actionstructure_list->addAction(new KileAction::InputTag(ki,"subpa&ragraph", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_subparagraph", parentWidget, KileAction::ShowAlternative|KileAction::ShowLabel, "\\subparagraph%A{%R}","\n", 0,1,i18n("\\subparagraph{title}\n\\subparagraph*{title} : do not include a number and do not make an entry in the table of contents"), i18n("&Subparagraph"),i18n("No &numbering")));

    ToolbarSelectAction *actionsize_list = new ToolbarSelectAction(i18n("Size"), actionCollection,false);
    actionCollection->addAction("size_list", actionsize_list);
    actionsize_list->addAction(new KileAction::Tag(i18n("tiny"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_tiny", "\\begin{tiny}", "\\end{tiny}", 12, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("scriptsize"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_scriptsize", "\\begin{scriptsize}", "\\end{scriptsize}", 18, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("footnotesize"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_footnotesize", "\\begin{footnotesize}", "\\end{footnotesize}", 20, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("small"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_small", "\\begin{small}", "\\end{small}", 13, 0));
    actionsize_list->addSeparator();
    actionsize_list->addAction(new KileAction::Tag(i18n("normalsize"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_normalsize", "\\begin{normalsize}", "\\end{normalsize}", 18, 0));
    actionsize_list->addSeparator();
    actionsize_list->addAction(new KileAction::Tag(i18n("large"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_large", "\\begin{large}", "\\end{large}", 13, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("Large"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_Large", "\\begin{Large}", "\\end{Large}", 13, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("LARGE"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_LARGE", "\\begin{LARGE}", "\\end{LARGE}", 13, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("huge"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_huge", "\\begin{huge}", "\\end{huge}", 12, 0));
    actionsize_list->addAction(new KileAction::Tag(i18n("Huge"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_Huge", "\\begin{Huge}", "\\end{Huge}", 12, 0));

    ToolbarSelectAction *actionother_list = new ToolbarSelectAction(i18n("Other"), actionCollection,false);
    actionCollection->addAction("other_list", actionother_list);
    actionother_list->addAction(new KileAction::Tag("label", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_label", "\\label{","} ",7,0,i18n("\\label{key}")));
    actionother_list->addAction(new KileAction::InputTag(ki,"ref", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_ref", parentWidget, KileAction::FromLabelList, "\\ref{%R", "}", 5,0, QString(), i18n("Label") ));
    actionother_list->addAction(new KileAction::InputTag(ki,"pageref", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_pageref", parentWidget, KileAction::FromLabelList, "\\pageref{%R", "}", 9,0, QString(), i18n("Label") ));
    actionother_list->addSeparator();
    actionother_list->addAction(new KileAction::Tag("index", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_index","\\index{","}",7,0,i18n( "\\index{word}")));
    actionother_list->addAction(new KileAction::Tag("footnote", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_footnote", "\\footnote{","}",10,0,i18n( "\\footnote{text}")));
    actionother_list->addSeparator();
    actionother_list->addAction(new KileAction::InputTag(ki,"cite", QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_cite", parentWidget, KileAction::FromBibItemList, "\\cite{%R", "}", 6,0, i18n("This command generates an in-text citation to the reference associated with the ref entry in the bib file\nYou can open the bib file with Kile to see all the available references"), i18n("Reference")));

    (void) new KileAction::Tag(i18n("Underline - \\underline{}"), i18n("Underline"), "format-text-underline", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_underline", "\\underline{","}",11);

    QAction *action = actionCollection->addAction("tag_tabulator", ki->editorExtension(), SLOT(insertIntelligentTabulator()));
    action->setText(i18n("Smart Tabulator"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Ampersand));

    // new tags (dani 29.01.2005)
    KActionCollection* ac = actionCollection;

    // environments
    (void) new KileAction::Tag(i18n("Abstract - \\begin{abstract}"), i18n("Abstract"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_env_abstract","\\begin{abstract}\n","%E\n\\end{abstract} ",0,1, i18n("\\begin{abstract}\ntext\n\\end{abstract}\nThe abstract environment creates a title page, i.e. a page with no printed page number or heading."));

    (void) new KileAction::Tag(i18n("Tabular* - \\begin{tabular*}"), i18n("Tabular*"), "tabular-star", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_env_tabular*","\\begin{tabular*}{}{","}\n%E\n\\end{tabular*}\n",17,0, i18n("\\begin{tabular*}{width}[pos]{cols}\ncolumn 1 entry & column 2 entry ... & column n entry \\\\\n...\n\\end{tabular*}\nThis is an extended version of the tabular environment with an extra parameter for the width. There must be rubber space between columns that can stretch to fill out the specified width."));

    (void) new KileAction::Tag(i18n("Minipage - \\begin{minipage}"), i18n("Minipage"), "minipage", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)),ac,"tag_env_minipage","\\begin{minipage}[","]{}\n%E\n\\end{minipage} ",17,0, i18n("The minipage environment is similar to a \\parbox command. It takes the same optional position argument and mandatory width argument. You may use other paragraph-making environments inside a minipage."));

    // lists
    (void) new KileAction::Tag(i18n("Table of Figures - \\listoffigures"), i18n("Table of Figures"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), ac,"tag_listoffigures","\\listoffigures",QString(),14,0, i18n("Put this command where you want the list of figures to go."));

    (void) new KileAction::Tag(i18n("Table of Tables - \\listoftables"), i18n("Table of Tables"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_listoftables","\\listoftables",QString(),14,0, i18n("Put this command where you want the list of tables to go."));

    (void) new KileAction::Tag(i18n("Generate Index - \\makeindex"), i18n("Generate Index"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_makeindex","\\makeindex",QString(),10,0, i18n("Put this command when you want to generate the raw index."));

    (void) new KileAction::Tag(i18n("Print Index - \\printindex"), i18n("Print Index"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_printindex","\\printindex",QString(),11,0, i18n("Put this command when you want to print the formatted index."));

    (void) new KileAction::Tag(i18n("Glossary - \\makeglossary"), i18n("Glossary"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_makeglossary","\\makeglossary",QString(),13,0, i18n("Put this command when you want to print a glossary."));

    (void) new KileAction::Tag(i18n("Bibliography - \\begin{thebibliography}"), i18n("Bibliography"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), ac,"tag_env_thebibliography","\\begin{thebibliography}{","}\n\n\\end{thebibliography} ",24,0, i18n("\\begin{thebibliography}{widest-label}\n\\bibitem[label]{cite_key}\n...\n\\end{thebibliography}\n\nwidest-label : Text that, when printed, is approximately as wide as the widest item label produces by the \\bibitem commands\n\\bibitem : Specify a bibliography item"));

    // verbatim code
    (void) new KileAction::Tag(i18n("Verbatim (show spaces) - \\begin{verbatim*}"), i18n("Verbatim (show spaces)"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)),ac,"tag_env_verbatim*","\\begin{verbatim*}\n","%E\n\\end{verbatim*}\n",0,1, i18n("Environment that gets LaTeX to print exactly what you type in. In this variant, spaces are printed in a special manner."));

    (void) new KileAction::Tag(i18n("Embedded Code - \\verb||"), i18n("Embedded Code"), "verb", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)),ac,"tag_verb","\\verb|","|",6,0, i18n("Macro form of the verbatim environment."));

    (void) new KileAction::Tag(i18n("Embedded Code (show spaces) - \\verb*||"), i18n("Embedded Code (show spaces)"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)),ac,"tag_verb*","\\verb*|","|",7,0, i18n("Macro form of the verbatim* environment."));

    // horizontal/vertical space
    (void) new KileAction::Tag(i18n("\"Small\" Vertical Space - \\smallskip"), i18n("\"Small\" Vertical Space"), "smallskip", QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), ac,"tag_smallskip","\\smallskip ",QString(),10,0, i18n("The \\smallskip command adds a 'small' vertical space."));

    (void) new KileAction::Tag(i18n("\\enskip"), QString(), "enskip", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_enskip", "\\enskip ", QString(), 8);

    (void) new KileAction::Tag(i18n("Horizontal Variable Space - \\hfill"), i18n("Horizontal Variable Space"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_hfill","\\hfill",QString(),6,0, i18n("The \\hfill fill command produces a \"rubber length\" which can stretch or shrink horizontally. It will be filled with spaces."));

    (void) new KileAction::Tag(i18n("Horizontal Dots - \\dotfill"), i18n("Horizontal Dots"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_dotfill","\\dotfill",QString(),8,0, i18n("The \\dotfill command produces a \"rubber length\" that produces dots instead of just spaces."));

    (void) new KileAction::Tag(i18n("Horizontal Rule - \\hrulefill"), i18n("Horizontal Rule"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_hrulefill","\\hrulefill",QString(),10,0, i18n("The \\hrulefill fill command produces a \"rubber length\" which can stretch or shrink horizontally. It will be filled with a horizontal rule."));

    (void) new KileAction::Tag(i18n("Vertical Variable Space - \\vfill"), i18n("Vertical Variable Space"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_vfill","\\vfill",QString(),6,0, i18n("The \\vfill fill command produces a \"rubber length\" which can stretch or shrink vertically."));

    (void) new KileAction::Tag(i18n("Horizontal Space - \\hspace{}"), i18n("Horizontal Space"), "hspace", QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_hspace","\\hspace{","}",8,0, i18n("The \\hspace command adds horizontal space. The length of the space can be expressed in any terms that LaTeX understands, i.e., points, inches, etc. You can add negative as well as positive space with an \\hspace command. Adding negative space is like backspacing."));

    (void) new KileAction::Tag(i18n("Horizontal Space (forced) - \\hspace*{}"), i18n("Horizontal Space (forced)"), "hspace-star", QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_hspace*","\\hspace*{","}",9,0, i18n("The \\hspace* command adds horizontal space like the \\hspace command. LaTeX removes horizontal space that comes at the end of a line. If you do not want LaTeX to remove this space, include the optional * argument. Then the space is never removed."));

    (void) new KileAction::Tag(i18n("Vertical Space - \\vspace{}"), i18n("Vertical Space"), "vspace", QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_vspace","\\vspace{","}",8,0, i18n("The \\vspace command adds vertical space. The length of the space can be expressed in any terms that LaTeX understands, i.e., points, inches, etc. You can add negative as well as positive space with an \\vspace command."));

    (void) new KileAction::Tag(i18n("Vertical Space (forced) - \\vspace*{}"), i18n("Vertical Space (forced)"), "vspace-star", QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)),ac,"tag_vspace*","\\vspace*{","}",9,0, i18n("The \\vspace* command adds vertical space like the \\vspace command. LaTeX removes vertical space that comes at the end of a page. If you do not want LaTeX to remove this space, include the optional * argument. Then the space is never removed."));

    // fonts
    new KileAction::Tag(i18n("Emphasized - \\emph{}"), i18n("Emphasized"), "emph", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_emph","\\emph{","}",6,0,i18n("\\emph{emphasized text}"));
    new KileAction::Tag(i18n("Strong - \\strong{}"), i18n("Strong"), "strong", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_strong","\\strong{","}",8,0,i18n("\\strong{text}"));

    (void) new KileAction::Tag(i18n("Roman - \\rmfamily"), i18n("Roman"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_rmfamily", "\\rmfamily", QString(), 9);
    (void) new KileAction::Tag(i18n("Sans Serif - \\sffamily"), i18n("Sans Serif"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_sffamily", "\\sffamily", QString(), 9);
    (void) new KileAction::Tag(i18n("Monospace - \\ttfamily"), i18n("Monospace"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_ttfamily", "\\ttfamily", QString(), 9);

    (void) new KileAction::Tag(i18n("Medium - \\mdseries"), i18n("Medium"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_mdseries", "\\mdseries", QString(), 9);
    (void) new KileAction::Tag(i18n("Bold - \\bfseries"), i18n("Bold"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_bfseries", "\\bfseries", QString(), 9);

    (void) new KileAction::Tag(i18n("Upright - \\upshape"), i18n("Upright"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_upshape", "\\upshape", QString(), 8);
    (void) new KileAction::Tag(i18n("Italic - \\itshape"), i18n("Italic"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_itshape", "\\itshape", QString(), 8);
    (void) new KileAction::Tag(i18n("Slanted - \\slshape"), i18n("Slanted"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_slshape", "\\slshape", QString(), 8);
    (void) new KileAction::Tag(i18n("Smallcaps - \\scshape"), i18n("Smallcaps"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_scshape", "\\scshape", QString(), 8);

}

void setupBibTags(const QObject *receiver, KActionCollection *actionCollection, KActionMenu * menu)
{
    KILE_DEBUG_MAIN << "void setupBibTags(const QObject *receiver, KActionCollection *actionCollection)";

    QString filename;

    if(KileConfig::bibliographyType().isEmpty() || KileConfig::bibliographyType() == QString("bibtex") ) {

        menu->addAction(new KileAction::Tag(i18n("Bibliography Style Selection - \\bibliographystyle{}"), i18n("Bibliography Style Selection"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliographystyle", "\\bibliographystyle{","} ",19,0,i18n("The argument to \\bibliographystyle refers to a file style.bst, which defines how your citations will look\nThe standard styles distributed with BibTeX are:\nalpha : sorted alphabetically. Labels are formed from name of author and year of publication.\nplain  : sorted alphabetically. Labels are numeric.\nunsrt : like plain, but entries are in order of citation.\nabbrv  : like plain, but more compact labels.")));
        menu->addAction(new KileAction::Tag(i18n("Bibliography Generation - \\bibliography{}"), i18n("Bibliography Generation"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliography","\\bibliography{%S", "}\n",14, 0,i18n("The argument to \\bibliography refers to the bib file (without extension)\nwhich should contain your database in BibTeX format.\nKile inserts automatically the base name of the TeX file")));
        menu->addSeparator();

        filename = KileUtilities::locate(QStandardPaths::AppDataLocation, "bibtexentries.rc");
    }
    else if(KileConfig::bibliographyType() == QString("biblatex")) {

        menu->addAction(new KileAction::Tag(i18n("Load Biblatex Package - \\usepackage{biblatex}"), i18n("Load Biblatex Package"), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliographyPackage", "\\usepackage{biblatex}\n",QString(),21,0,i18n("This includes the package biblatex")));
        menu->addAction(new KileAction::Tag(i18n("Bibliography Generation - \\bibliography{}"), i18n("Bibliography Generation"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliography","\\bibliography{%S", "}\n",14, 0,i18n("The argument to \\bibliography refers to the bib file (without extension)\nwhich should contain your database in BibTeX format.\nKile inserts automatically the base name of the TeX file")));
        menu->addAction(new KileAction::Tag(i18n("Print Bibliography"), QString(), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_printbibliography", "\\printbibliography",QString(),18,0,i18n("Prints the complete bibliography")));
        menu->addAction(new KileAction::Tag(i18n("Print Bibliography by Section"), QString(), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliographyBySection", "\\bibbysection[","]",14,0,i18n("Print the bibliography for each section")));
        menu->addAction(new KileAction::Tag(i18n("Print List of Shorthands"), QString(), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bibliographyShortHands", "\\printshorthands",QString(),16,0,QString()));
        menu->addSeparator();
        /* use this to insert more
        		menu->addAction(new KileAction::Tag(i18n(""), QKeySequence(), receiver,SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_", "\\",QString(),,0,i18n("")));
        Load Biblatex-Package - \usepackage{biblatex}
        Bibliography File - \bibliography{}
        Print Bibliography - \printbibliography
        Print Bibliography by Section - \bibbysection[]
        Print List of Shorthands - \printshorthands
        */
        filename = KileUtilities::locate(QStandardPaths::AppDataLocation, "biblatexentries.rc");
    }
    else {
        filename.clear();
    }

    if(filename.isEmpty()) {
        KILE_DEBUG_MAIN << "found no filename" << Qt::endl;
        return;
    }

    KConfig *bibCfg = new KConfig(filename, KConfig::SimpleConfig);

    if(bibCfg == Q_NULLPTR )
        return;

    const QStringList groupList = bibCfg->groupList();

    // check if a non-deleted group exists
    // groupList.count() == 0 is not enough due to bug 384039
    {
        bool allDeleted = false;
        for(auto group : groupList) {
            if(bibCfg->hasGroup(group)) {
                allDeleted = false;
                break;
            }
        }
        if(allDeleted) {
            return;
        }
    }

    QString name, tag, internalName, keys, key;
    QStringList keyList, optKeyList, altKeyList;
    QString altText, optText, compText;

    for(auto group : groupList) {
        if(!bibCfg->hasGroup(group)) { // 'group' might have been deleted
            continue;                // work around bug 384039
        }
        altKeyList.clear();
        keyList.clear();
        optKeyList.clear();

        KConfigGroup grp = bibCfg->group(group);

// 		KILE_DEBUG_MAIN << "group " <<  grp.name();

        tag = grp.name();
        name = grp.readEntry(QString("name"));
        internalName = grp.readEntry(QString("internalName"));
        keyList = grp.readEntry(QString("key")).split(',', Qt::SkipEmptyParts);
        altKeyList = grp.readEntry(QString("altkey")).split(',', Qt::SkipEmptyParts);
        optKeyList = grp.readEntry(QString("optkey")).split(',', Qt::SkipEmptyParts);

// 		KILE_DEBUG_MAIN << "length(keys)=" << keyList.count() << ", length(altkeys)=" << altKeyList.count() << ", length(optkeys)=" << optKeyList.count();
// 		KILE_DEBUG_MAIN << "tag=" << tag << ", name=" << name << ", internalName=" << internalName;

        keys = QString("@%1{,\n").arg(tag);
        int length = keys.length() - 2;

        // do some trimming
        name = name.trimmed();
        internalName = "tag_bib_" + internalName.trimmed();
        tag = tag.trimmed();

        for(QList<QString>::iterator it = keyList.begin(); it != keyList.end(); ++it) {
            key = (*it).trimmed();
            key = QString(" %1 = {},\n").arg(key);
            keys.append(key);
// 			KILE_DEBUG_MAIN << "key" << key ;
        }

        for(QList<QString>::iterator it = altKeyList.begin(); it != altKeyList.end(); ++it) {
            key = (*it).trimmed();
            key = QString(" ALT%1 = {},\n").arg(key);
            keys.append(key);
// 			KILE_DEBUG_MAIN << "altkey" << key ;
        }

        for(QList<QString>::iterator it = optKeyList.begin(); it != optKeyList.end(); ++it) {
            key = (*it).trimmed();
            key = QString(" OPT%1 = {},\n").arg(key);
            keys.append(key);
// 			KILE_DEBUG_MAIN << "optKey" << key;
        }
        keys.append("}\n");

        altText = i18n("ALT.... : you have the choice between these two fields\n");
        optText = i18n("OPT.... : optional fields (use the 'Clean' command to remove them)");
        compText = i18n("Bib fields - %1\n",name);

        if( altKeyList.count() > 1 ) {
            compText.append(altText);
        }
        if( optKeyList.count() > 1 ) {
            compText.append(optText);
        }

        menu->addAction(new KileAction::Tag(name, QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,internalName,keys,QString(),length,0,compText));
    }
}

void setupMathTags(const QObject *receiver, KActionCollection *actionCollection)
{
    (void) new KileAction::Tag(i18n("\\mathrm{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathrm","\\mathrm{","}",8);
    (void) new KileAction::Tag(i18n("\\mathit{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathit","\\mathit{","}",8);
    (void) new KileAction::Tag(i18n("\\mathbf{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathbf","\\mathbf{","}",8);
    (void) new KileAction::Tag(i18n("\\mathsf{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathsf","\\mathsf{","}",8);
    (void) new KileAction::Tag(i18n("\\mathtt{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathtt","\\mathtt{","}",8);
    (void) new KileAction::Tag(i18n("\\mathcal{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathcal","\\mathcal{","}",9);
    (void) new KileAction::Tag(i18n("\\mathbb{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathbb","\\mathbb{","}",8);
    (void) new KileAction::Tag(i18n("\\mathfrak{}"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathfrak","\\mathfrak{","}",10);

    (void) new KileAction::Tag(i18n("\\acute{}"), QString(), "acute", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_acute", "\\acute{","}",7);
    (void) new KileAction::Tag(i18n("\\grave{}"), QString(), "grave", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_grave", "\\grave{","}", 7);
    (void) new KileAction::Tag(i18n("\\tilde{}"), QString(), "tilde", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_tilde", "\\tilde{","}", 7);
    (void) new KileAction::Tag(i18n("\\bar{}"), QString(), "bar",    QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_bar", "\\bar{","}", 5);
    (void) new KileAction::Tag(i18n("\\vec{}"), QString(), "vec",    QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_vec", "\\vec{","}", 5);
    (void) new KileAction::Tag(i18n("\\hat{}"), QString(), "hat",    QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_hat", "\\hat{","}", 5);
    (void) new KileAction::Tag(i18n("\\check{}"), QString(), "check", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_check", "\\check{","}", 7);
    (void) new KileAction::Tag(i18n("\\breve{}"), QString(), "breve", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_breve", "\\breve{","}", 7);
    (void) new KileAction::Tag(i18n("\\dot{}"), QString(), "dot",    QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_dot", "\\dot{","}", 5);
    (void) new KileAction::Tag(i18n("\\ddot{}"), QString(), "ddot",  QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_ddot", "\\ddot{","}", 6);

    (void) new KileAction::Tag(i18n("Small Space"), QString(), "thinspace", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_space_small", "\\,", QString(), 2);
    (void) new KileAction::Tag(i18n("Medium Space"), QString(), "medspace", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_space_medium", "\\:", QString(),2);
    (void) new KileAction::Tag(i18n("Large Space"), QString(), "bigspace", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_space_large", "\\;", QString(),2);
    (void) new KileAction::Tag(i18n("\\quad"), QString(), "quad", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_quad", "\\quad ", QString(), 6);
    (void) new KileAction::Tag(i18n("\\qquad"), QString(), "qquad", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_qquad", "\\qquad ", QString(), 7);

    (void) new KileAction::Tag(i18n("Math Mode - $...$"), i18n("Math Mode"), "mathmode", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_M), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_mathmode","$","$",1);
    (void) new KileAction::Tag(i18n("Displaymath Mode - \\[...\\]"), i18n("Displaymath Mode"), "displaymathmode", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_E), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_equation", "\\[","\\]", 2);
    (void) new KileAction::Tag(i18n("Equation - \\begin{equation}"), i18n("Equation"), "equation", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_equation","\\begin{equation}\n","%E\n\\end{equation} ",0,1);
    (void) new KileAction::Tag(i18n("Subscript - _{}"), i18n("Subscript"), "format-text-subscript", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_D), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_subscript","_{","}",2);
    (void) new KileAction::Tag(i18n("Superscript - ^{}"), i18n("Superscript"), "format-text-superscript", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_U), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_superscript","^{","}",2);
    (void) new KileAction::Tag(i18n("Normal Fraction - \\frac{}{}"), i18n("Normal Fraction"), "smallfrac", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_F), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_frac", "\\frac{","}{}",6);
    (void) new KileAction::Tag(i18n("Displaystyle Fraction - \\dfrac{}{}"), i18n("Displaystyle Fraction"), "dfrac", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_Q), receiver, SLOT(insertAmsTag(KileAction::TagData)), actionCollection,"tag_dfrac", "\\dfrac{","}{}", 7);
    (void) new KileAction::Tag(i18n("Textstyle Fraction - \\tfrac{}{}"), i18n("Textstyle Fraction"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), actionCollection, "tag_tfrac", "\\tfrac{", "}{}", 7);
    (void) new KileAction::Tag(i18n("Square Root - \\sqrt{}"), i18n("Square Root"), "sqrt", QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_S), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_sqrt", "\\sqrt{","}", 6);
    (void) new KileAction::Tag(i18n("\\left"), QString(), QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_L), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_left", "\\left", QString(), 5);
    (void) new KileAction::Tag(i18n("\\right"), QString(), QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_R), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_right", "\\right", QString(), 6);
    (void) new KileAction::Tag(i18n("Array - \\begin{array}"), i18n("Array"), "array", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection,"tag_env_array", "\\begin{array}{}\n", "%E\n\\end{array}", 14, 0,
                               i18n("\\begin{array}{col1col2...coln}\ncolumn 1 entry & column 2 entry ... & column n entry \\\\ \n...\n\\end{array}\nEach column, coln, is specified by a single letter that tells how items in that column should be formatted.\n"
                                    "     c -- for centered \n     l -- for flush left \n     r -- for flush right\n"));

    ToolbarSelectAction *actionleft_list = new ToolbarSelectAction(i18n("Left Delimiter"), actionCollection,false);
    actionCollection->addAction("left_list", actionleft_list);
    actionleft_list->addAction(new KileAction::Tag(i18n("left ("), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_(", "\\left( ", QString(), 7, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left ["), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_[", "\\left[ ", QString(), 7, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left {"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_{", "\\left\\lbrace ", QString(), 14, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left &lt;"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_<", "\\left\\langle ", QString(), 14, 0));
    actionleft_list->addSeparator();
    actionleft_list->addAction(new KileAction::Tag(i18n("left )"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_)", "\\left) ", QString(), 7, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left ]"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_]", "\\left] ", QString(), 7, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left }"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_}", "\\left\\rbrace ", QString(), 14, 0));
    actionleft_list->addAction(new KileAction::Tag(i18n("left &gt;"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_>", "\\left\\rangle ", QString(), 14, 0));
    actionleft_list->addSeparator();
    actionleft_list->addAction(new KileAction::Tag(i18n("left ."), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_left_.", "\\left. ", QString(), 7, 0));

    ToolbarSelectAction *actionright_list = new ToolbarSelectAction(i18n("Right Delimiter"), actionCollection,false);
    actionCollection->addAction("right_list", actionright_list);
    actionright_list->addAction(new KileAction::Tag(i18n("right )"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_)", "\\right) ", QString(), 8, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right ]"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_]", "\\right] ", QString(), 8, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right }"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_}", "\\right\\rbrace ", QString(), 14, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right &gt;"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_>", "\\right\\rangle ", QString(), 14, 0));
    actionright_list->addSeparator();
    actionright_list->addAction(new KileAction::Tag(i18n("right ("), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_(", "\\right( ", QString(), 8, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right ["), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_[", "\\right[ ", QString(), 8, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right {"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_{", "\\right\\lbrace ", QString(), 14, 0));
    actionright_list->addAction(new KileAction::Tag(i18n("right &lt;"), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_<", "\\right\\langle ", QString(), 14, 0));
    actionright_list->addSeparator();
    actionright_list->addAction(new KileAction::Tag(i18n("right ."), QString(), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), actionCollection, "tag_right_.", "\\right. ", QString(), 8, 0));

    // new math tags (dani 29.01.2005)
    KActionCollection* ac = actionCollection;

    (void) new KileAction::Tag(i18n("Normal Binomial - \\binom{}{}"), i18n("Normal Binomial"), "binom", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_binom", "\\binom{","}{}", 7);

    (void) new KileAction::Tag(i18n("Displaystyle Binomial - \\dbinom{}{}"), i18n("Displaystyle Binomial"), QKeySequence(), receiver,SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_dbinom", "\\dbinom{","}{}", 8);

    (void) new KileAction::Tag(i18n("Textstyle Binomial - \\tbinom{}{}"), i18n("Textstyle Binomial"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_tbinom", "\\tbinom{","}{}", 8);

    (void) new KileAction::Tag(i18n("N-th Root - \\sqrt[]{}"), i18n("N-th Root"), "nroot", QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_nroot", "\\sqrt[]{","}", 6);

    (void) new KileAction::Tag(i18n("Left-Right () - \\left(..\\right)"), i18n("Left-Right ()"), "lr", QKeySequence(Qt::ALT+Qt::Key_ParenLeft), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_leftright", "\\left(  \\right)", QString(), 7);

    (void) new KileAction::Tag(i18n("Extendable Left Arrow - \\xleftarrow{}"), i18n("Extendable Left Arrow"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_xleftarrow", "\\xleftarrow{","}", 12);

    (void) new KileAction::Tag(i18n("Extendable Right Arrow - \\xrightarrow{}"), i18n("Extendable Right Arrow"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_xrightarrow", "\\xrightarrow{","}", 13);

    (void) new KileAction::Tag(i18n("Boxed Formula - \\boxed{}"), i18n("Boxed Formula"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_boxed", "\\boxed{","}", 7);

    (void) new KileAction::Tag(i18n("bigl - \\bigl"), i18n("bigl"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_bigl", "\\bigl",QString(), 5);
    (void) new KileAction::Tag(i18n("Bigl - \\Bigl"), i18n("Bigl"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_Bigl", "\\Bigl",QString(), 5);
    (void) new KileAction::Tag(i18n("biggl - \\biggl"), i18n("biggl"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_biggl", "\\biggl",QString(), 6);
    (void) new KileAction::Tag(i18n("Biggl - \\Biggl"), i18n("Biggl"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_Biggl", "\\Biggl",QString(), 6);

    (void) new KileAction::Tag(i18n("bigr - \\bigr"), i18n("bigr"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_bigr", "\\bigr",QString(), 5);
    (void) new KileAction::Tag(i18n("Bigr - \\Bigr"), i18n("Bigr"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_Bigr", "\\Bigr",QString(), 5);
    (void) new KileAction::Tag(i18n("biggr - \\biggr"), i18n("biggr"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_biggr", "\\biggr",QString(), 6);
    (void) new KileAction::Tag(i18n("Biggr - \\Biggr"), i18n("Biggr"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_Biggr", "\\Biggr",QString(), 6);

    // text in mathmode
    (void) new KileAction::Tag(i18n("Text in Mathmode - \\text{}"), i18n("Text in Mathmode"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_text", "\\text{","}", 6);

    (void) new KileAction::Tag(i18n("Intertext - \\intertext{}"), i18n("Intertext"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)), ac,"tag_intertext", "\\intertext{","}\n", 11);

    // math environments
    (void) new KileAction::Tag(i18n("Displaymath - \\begin{displaymath}"), i18n("Displaymath"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_env_displaymath","\\begin{displaymath}\n","%E\n\\end{displaymath}\n",0,1);

    (void) new KileAction::Tag(i18n("Equation (not numbered) - \\begin{equation*}"), i18n("Equation (not numbered)"), QKeySequence(), receiver, SLOT(insertTag(KileAction::TagData)), ac,"tag_env_equation*","\\begin{equation*}\n","%E\n\\end{equation*}\n",0,1);

    // AMS environments
    (void) new KileAction::Tag(i18n("Multline - \\begin{multline}"), i18n("Multline"), "multline", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_multline","\\begin{multline}\n","%E\n\\end{multline}\n", 0,1);
    (void) new KileAction::Tag(i18n("Multline* - \\begin{multline*}"), i18n("Multline*"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_multline*","\\begin{multline*}\n","%E\n\\end{multline*}\n", 0,1);

    (void) new KileAction::Tag(i18n("Split - \\begin{split}"), i18n("Split"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_split","\\begin{split}\n","%E\n\\end{split}\n", 0,1);

    (void) new KileAction::Tag(i18n("Gather - \\begin{gather}"), i18n("Gather"), "gather", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_gather","\\begin{gather}\n","%E\n\\end{gather}\n", 0,1);
    (void) new KileAction::Tag(i18n("Gather* - \\begin{gather*}"), i18n("Gather*"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_gather*","\\begin{gather*}\n","%E\n\\end{gather*}\n", 0,1);

    (void) new KileAction::Tag(i18n("Align - \\begin{align}"), i18n("Align"), "align", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_align","\\begin{align}\n","%E\n\\end{align}\n", 0,1);
    (void) new KileAction::Tag(i18n("Align* - \\begin{align*}"), i18n("Align*"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_align*","\\begin{align*}\n","%E\n\\end{align*}\n", 0,1);

    (void) new KileAction::Tag(i18n("Flalign - \\begin{flalign}"), i18n("Flalign"), "flalign", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_flalign","\\begin{flalign}\n","%E\n\\end{flalign}\n", 0,1);
    (void) new KileAction::Tag(i18n("Flalign* - \\begin{flalign*}"), i18n("Flalign*"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_flalign*","\\begin{flalign*}\n","%E\n\\end{flalign*}\n", 0,1);

    (void) new KileAction::Tag(i18n("Alignat - \\begin{alignat}"), i18n("Alignat"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_alignat","\\begin{alignat}{","}\n%E\n\\end{alignat}\n", 16,0);
    (void) new KileAction::Tag(i18n("Alignat* - \\begin{alignat*}"), i18n("Alignat*"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_alignat*","\\begin{alignat*}{","}\n%E\n\\end{alignat*}\n", 17,0);

    (void) new KileAction::Tag(i18n("Aligned - \\begin{aligned}"), i18n("Aligned"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_aligned","\\begin{aligned}\n","%E\n\\end{aligned}\n", 0,1);
    (void) new KileAction::Tag(i18n("Gathered - \\begin{gathered}"), i18n("Gathered"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_gathered","\\begin{gathered}\n","%E\n\\end{gathered}\n", 0,1);
    (void) new KileAction::Tag(i18n("Alignedat - \\begin{alignedat}"), i18n("Alignedat"), QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_alignedat","\\begin{alignedat}\n","%E\n\\end{alignedat}\n", 0,1);

    (void) new KileAction::Tag(i18n("Cases - \\begin{cases}"), i18n("Cases"), "cases", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_cases","\\begin{cases}\n","%E\n\\end{cases}\n", 0,1);

    (void) new KileAction::Tag(i18n("matrix - \\begin{matrix}"), i18n("matrix"), "matrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_matrix","\\begin{matrix}\n","%E\n\\end{matrix}\n", 0,1);
    (void) new KileAction::Tag(i18n("pmatrix - \\begin{pmatrix}"), i18n("pmatrix"), "pmatrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_pmatrix","\\begin{pmatrix}\n","%E\n\\end{pmatrix}\n", 0,1);
    (void) new KileAction::Tag(i18n("vmatrix - \\begin{vmatrix}"), i18n("vmatrix"), "vmatrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_vmatrix","\\begin{vmatrix}\n","%E\n\\end{vmatrix}\n", 0,1);
    (void) new KileAction::Tag(i18n("Vmatrix - \\begin{Vmatrix}"), i18n("Vmatrix"), "VVmatrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_VVmatrix","\\begin{Vmatrix}\n","%E\n\\end{Vmatrix}\n", 0,1);
    (void) new KileAction::Tag(i18n("bmatrix - \\begin{bmatrix}"), i18n("bmatrix"), "bmatrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_bmatrix","\\begin{bmatrix}\n","%E\n\\end{bmatrix}\n", 0,1);
    (void) new KileAction::Tag(i18n("Bmatrix - \\begin{Bmatrix}"), i18n("Bmatrix"), "BBmatrix", QKeySequence(), receiver, SLOT(insertAmsTag(KileAction::TagData)),ac,"tag_env_BBmatrix","\\begin{Bmatrix}\n","%E\n\\end{Bmatrix}\n", 0,1);

}

}
