//
//
// C++ Implementation: kilestdactions
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2003
//
// Copyright: See COPYING file that comes with parent distribution
//

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmainwindow.h>
#include <klocale.h>

#include "kileactions.h"

namespace KileStdActions
{

void setupStdTags(KileInfo *ki, KMainWindow *parent, QPtrList<KAction>* plist )
{
	(void) new KileAction::Tag("\\documentclass",0,parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(), "tag_documentclass",
  		"\\documentclass[10pt]{}",QString::null,21,0,i18n("\\documentclass[options]{class}\nclass : article,report,book,letter\nsize options : 10pt, 11pt, 12pt\npaper size options: a4paper, a5paper, b5paper, letterpaper, legalpaper, executivepaper\n"
		"other options: \nlandscape -- selects landscape format. Default is portrait. \ntitlepage, notitlepage -- selects if there should be a separate title page.\nleqno -- equation number on left side of equations. Default is right side.\n"
		"fleqn -- displayed formulas flush left. Default is centered.\nonecolumn, twocolumn -- one or two columns. Defaults to one column\noneside, twoside -- selects one- or twosided layout.\n" ));

	(void) new KileAction::Tag("\\usepackage{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_usepackage",
		"\\usepackage{} ",QString::null,12,0,i18n("Any options given in the \\documentclass command that are unknown by the selected document class\n"
		"are passed on to the packages loaded with \\usepackage."));

	(void) new KileAction::Tag(i18n("AMS Packages"),0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_amspackages","\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n",QString::null,0,3,i18n("The principal American Mathematical Society packages"));
	(void) new KileAction::Tag("\\begin{document}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_document","\\begin{document}\n\n\\end{document} ",QString::null,0,1,i18n("Text is allowed only between \\begin{document} and \\end{document}.\nThe 'preamble' (before \\begin{document}} ) may contain declarations only."));
	(void) new KileAction::Tag("\\maketitle",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_maketitle","\\maketitle",QString::null,10,0,i18n("This command generates a title on a separate title page\n- except in the article class, where the title normally goes at the top of the first page."));
	(void) new KileAction::Tag("\\tableofcontents",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_tableofcontents","\\tableofcontents",QString::null,16,0,i18n("Put this command where you want the table of contents to go"));
	(void) new KileAction::Tag("\\title{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_title","\\title{","}",7,0,i18n( "\\title{text}\nThe \\title command declares text to be the title.\nUse \\\\ to tell LaTeX where to start a new line in a long title."));
	(void) new KileAction::Tag("\\author{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_author","\\author{","}",8,0,i18n( "\\author{names}\nThe \\author command declares the author(s), where names is a list of authors separated by \\and commands."));

	(void) new KileAction::Tag("\\begin{center}","text_center",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_center", "\\begin{center}\n","\n\\end{center}", 0,1, i18n("Each line must be terminated with the string \\\\."));
	(void) new KileAction::Tag("\\begin{flushleft}","text_left",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_flushleft", "\\begin{flushleft}\n","\n\\end{flushleft}", 0,1, i18n("Each line must be terminated with the string \\\\.") );
	(void) new KileAction::Tag("\\begin{flushright}","text_right",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_flushright", "\\begin{flushright}\n","\n\\end{flushright}", 0,1, i18n("Each line must be terminated with the string \\\\.") );
	(void) new KileAction::Tag("\\begin{quote}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_quota","\\begin{quote}\n","\n\\end{quote} ",0,1,i18n("The text is justified at both margins.\nLeaving a blank line between text produces a new paragraph.") );
	(void) new KileAction::Tag("\\begin{quotation}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_quotation","\\begin{quotation}\n","\n\\end{quotation} ",0,1, i18n("The text is justified at both margins and there is paragraph indentation.\nLeaving a blank line between text produces a new paragraph.") );
	(void) new KileAction::Tag("\\begin{verse}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_verse", "\\begin{verse}\n","\n\\end{verse} ",0,1,i18n("The verse environment is designed for poetry.\nSeparate the lines of each stanza with \\\\, and use one or more blank lines to separate the stanzas.") );

	(void) new KileAction::Tag("\\begin{verbatim}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_verbatim","\\begin{verbatim}\n","\n\\end{verbatim} ",0,1,i18n("Environment that gets LaTeX to print exactly what you type in."));
	(void) new KileAction::Tag("\\begin{itemize}","itemize",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_itemize","\\begin{itemize}\n\\item \n\\end{itemize} ",QString::null,6,1,i18n("The itemize environment produces a 'bulleted' list.\nEach item of an itemized list begins with an \\item command."));
	(void) new KileAction::Tag("\\begin{enumerate}","enumerate",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_enumerate","\\begin{enumerate}\n\\item \n\\end{enumerate} ",QString::null,6,1,i18n("The enumerate environment produces a numbered list.\nEach item of an enumerated list begins with an \\item command."));
	(void) new KileAction::Tag("\\begin{description}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_description","\\begin{description}\n\\item[]\n\\end{description} ",QString::null,6,1,i18n("The description environment is used to make labelled lists.\nEach item of the list begins with an \\item[label] command.\nThe 'label' is bold face and flushed right."));
	(void) new KileAction::Tag("\\begin{list}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_list","\\begin{list}{}{}\n\\item \n\\end{list} ",QString::null,13,0,i18n("\\begin{list}{label}{spacing}\nThe {label} argument is a piece of text that is inserted in a box to form the label. \nThe {spacing} argument contains commands to change the spacing parameters for the list.\nEach item of the list begins with an \\item command."));


	(void) new KileAction::Tag("\\begin{table}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_table","\\begin{table}\n","\n\\caption{}\n\\end{table} ",0,1,
		i18n("\\begin{table}[placement]\nbody of the table\n\\caption{table title}\n\\end{table}\nTables are objects that are not part of the normal text, and are usually floated to a convenient place\n"
		"The optional argument [placement] determines where LaTeX will try to place your table\nh : Here - at the position in the text where the table environment appear\nt : Top - at the top of a text page\nb : Bottom - at the bottom of a text page\n"
		"p : Page of floats - on a separate float page, which is a page containing no text, only floats\nThe body of the table is made up of whatever text, LaTeX commands, etc., you wish.\nThe \\caption command allows you to title your table."));

	(void) new KileAction::Tag("\\begin{figure}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_figure" ,"\\begin{figure}\n","\n\\caption{}\n\\end{figure} ",0,1,
		i18n("\\begin{figure}[placement]\nbody of the figure\n\\caption{figure title}\n\\end{figure}\nFigures are objects that are not part of the normal text, and are usually floated to a convenient place\n"
		"The optional argument [placement] determines where LaTeX will try to place your figure\nh : Here - at the position in the text where the figure environment appear\nt : Top - at the top of a text page\n"
		"b : Bottom - at the bottom of a text page\np : Page of floats - on a separate float page, which is a page containing no text, only floats\nThe body of the figure is made up of whatever text, LaTeX commands, etc., you wish.\nThe \\caption command allows you to title your figure."));

	(void) new KileAction::Tag("\\begin{titlepage}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_titlepage" ,"\\begin{titlepage}\n","\n\\end{titlepage} ",0,1,
		i18n("\\begin{titlepage}\ntext\n\\end{titlepage}\nThe titlepage environment creates a title page, i.e. a page with no printed page number or heading."));

	plist->append(new KileAction::Tag(i18n("\\textit - Italics"),"text_italic",Qt::ALT+Qt::Key_I, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_textit","\\textit{","}",8,0,i18n("\\textit{italic text}")));
	plist->append(new KileAction::Tag(i18n("\\textsl - Slanted"),Qt::ALT+Qt::Key_A, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_textsl","\\textsl{","}",8,0,i18n("\\textsl{slanted text}")));
	plist->append(new KileAction::Tag(i18n("\\textbf - Boldface"),"text_bold",Qt::ALT+Qt::Key_B, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_textbf","\\textbf{","}",8,0,i18n("\\textbf{boldface text}")));
	plist->append(new KileAction::Tag(i18n("\\texttt - Typewriter"),Qt::ALT+Qt::Key_T, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_texttt","\\texttt{","}",8,0,i18n("\\texttt{typewriter text}")));
	plist->append(new KileAction::Tag(i18n("\\textsc - Small caps"),Qt::ALT+Qt::Key_C, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_textsc","\\textsc{","}",8,0,i18n("\\textsc{small caps text}")));
 	plist->append(new KileAction::Tag("\\item","item",Qt::ALT+Qt::Key_H, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_item","\\item ",QString::null,6,0, i18n("\\item[label] Hello!")));

	(void) new KileAction::Tag("\\begin{tabbing}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_tabbing" ,"\\begin{tabbing}\n","\n\\end{tabbing} ",0,1,i18n("The tabbing environment provides a way to align text in columns.\n\\begin{tabbing}\ntext \\= more text \\= still more text \\= last text \\\\\nsecond row \\>  \\> more \\\\\n\\end{tabbing}\nCommands :\n\\=  Sets a tab stop at the current position.\n\\>  Advances to the next tab stop.\n\\<  Allows you to put something to the left of the local margin without changing the margin. Can only be used at the start of the line.\n\\+  Moves the left margin of the next and all the following commands one tab stop to the right\n\\-  Moves the left margin of the next and all the following commands one tab stop to the left\n\\'  Moves everything that you have typed so far in the current column to the right of the previous column, flush against the current column's tab stop. \n\\`  Allows you to put text flush right against any tab stop, including tab stop 0\n\\kill  Sets tab stops without producing text.\n\\a  In a tabbing environment, the commands \\=, \\' and \\` do not produce accents as normal. Instead, the commands \\a=, \\a' and \\a` are used."));
	(void) new KileAction::Tag("\\begin{tabular}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_tabular" ,"\\begin{tabular}{","}\n\n\\end{tabular} ",16,0,i18n("\\begin{tabular}[pos]{cols}\ncolumn 1 entry & column 2 entry ... & column n entry \\\\\n...\n\\end{tabular}\npos : Specifies the vertical position; default is alignment on the center of the environment.\n     t - align on top row\n     b - align on bottom row\ncols : Specifies the column formatting.\n     l - A column of left-aligned items.\n     r - A column of right-aligned items.\n     c - A column of centered items.\n     | - A vertical line the full height and depth of the environment.\n     @{text} - this inserts text in every row.\nThe \\hline command draws a horizontal line the width of the table.\nThe \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j,\nThe \\vline command draws a vertical line extending the full height and depth of its row."));
	(void) new KileAction::Tag("\\multicolumn",0, parent, SLOT(insertTag(const KileAction::TagData&)),parent->actionCollection(),"tag_multicolumn","\\multicolumn{","}{}{} ",13,0,i18n("\\multicolumn{cols}{pos}{text}\ncol, specifies the number of columns to span.\npos specifies the formatting of the entry: c for centered, l for flushleft, r for flushright.\ntext specifies what text is to make up the entry."));
	(void) new KileAction::Tag("\\hline",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_hline" ,"\\hline ",QString::null,7,0,i18n("The \\hline command draws a horizontal line the width of the table."));
	(void) new KileAction::Tag("\\vline",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_vline" ,"\\vline ",QString::null,7,0,i18n("The \\vline command draws a vertical line extending the full height and depth of its row."));
	(void) new KileAction::Tag("\\cline",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_cline" ,"\\cline{-} ",QString::null,7,0,i18n("The \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j,"));

	(void) new KileAction::Tag("\\newpage",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_newpage","\\newpage ",QString::null,9,0,i18n("The \\newpage command ends the current page"));
	(void) new KileAction::Tag("\\linebreak",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_linebreak","\\linebreak ",QString::null,11,0,i18n("The \\linebreak command tells LaTeX to break the current line at the point of the command."));
	(void) new KileAction::Tag("\\pagebreak",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_pagebreak","\\pagebreak ",QString::null,11,0,i18n("The \\pagebreak command tells LaTeX to break the current page at the point of the command."));
	(void) new KileAction::Tag("\\bigskip",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bigskip","\\bigskip ",QString::null,9,0,i18n("The \\bigskip command adds a 'big' vertical space."));
	(void) new KileAction::Tag("\\medskip",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_medskip","\\medskip ",QString::null,9,0,i18n("The \\medskip command adds a 'medium' vertical space."));

  	(void) new KileAction::InputFigure(ki,"\\includegraphics{file.eps}",0, parent, SLOT(insertGraphic(const KileAction::TagData&)), parent->actionCollection(),"tag_includegraphics", parent, KileAction::KeepHistory | KileAction::ShowBrowseButton | KileAction::ShowFigureInput, "\\includegraphics[scale=1]{","}", 26,0, i18n( "This command is used to import image files (\\usepackage{graphicx} is required)\nExamples :\n\\includegraphics{file} ; \\includegraphics[width=10cm]{file} ; \\includegraphics*[scale=0.75]{file}"), i18n("Type or select a filename: "));
	(void) new KileAction::InputTag(ki,"\\include{file}","include",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_include",parent, KileAction::KeepHistory | KileAction::ShowBrowseButton, "\\include{%R","}",9,0, i18n("\\include{file}\nThe \\include command is used in conjunction with the \\includeonly command for selective inclusion of files."),i18n("Type or select a filename: "));
	(void) new KileAction::InputTag(ki,"\\input{file}","include",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_input", parent, KileAction::KeepHistory | KileAction::ShowBrowseButton, "\\input{%R","}",7,0,i18n("\\input{file}\nThe \\input command causes the indicated file to be read and processed, exactly as if its contents had been inserted in the current file at that point."),i18n("Type or select a filename: "));
	(void) new KileAction::Tag("\\bibliographystyle{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bibliographystyle", "\\bibliographystyle{","} ",19,0,i18n("The argument to \\bibliographystyle refers to a file style.bst, which defines how your citations will look\nThe standard styles distributed with BibTeX are:\nalpha : sorted alphabetically. Labels are formed from name of author and year of publication.\nplain  : sorted alphabetically. Labels are numeric.\nunsrt : like plain, but entries are in order of citation.\nabbrv  : like plain, but more compact labels."));
	(void) new KileAction::Tag("\\bibliography{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bibliography","\\bibliography{%S}\n",QString::null,0,1,i18n("The argument to \\bibliography refers to the bib file (without extension)\nwhich should contain your database in BibTeX format.\nKile inserts automatically the base name of the TeX file"));

	KileAction::Select *actionstructure_list = new KileAction::Select(i18n("Sectioning"), 0, parent->actionCollection(), "structure_list");
	QPtrList<KAction> alist;
	alist.append(new KileAction::InputTag(ki,"part","part",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_part", parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\part%A{%R}","\n", 0,1,i18n("\\part{title}\n\\part*{title} : do not include a number and do not make an entry in the table of contents\n"), i18n("Part"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"chapter","chapter",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_chapter" ,parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\chapter%A{%R}","\n", 0,1,i18n("\\chapter{title}\n\\chapter*{title} : do not include a number and do not make an entry in the table of contents\nOnly for 'report' and 'book' class document."), i18n("Chapter"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"section","section",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_section",parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\section%A{%R}","\n", 0,1,i18n("\\section{title}\n\\section*{title} : do not include a number and do not make an entry in the table of contents"), i18n("Section"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"subsection","subsection",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_subsection" ,parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\subsection%A{%R}","\n", 0,1,i18n("\\subsection{title}\n\\subsection*{title} : do not include a number and do not make an entry in the table of contents"), i18n("Subsection"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"subsubsection","subsubsection",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_subsubsection",parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\subsubsection%A{%R}","\n", 0,1,i18n("\\subsubsection{title}\n\\subsubsection*{title} : do not include a number and do not make an entry in the table of contents"), i18n("Subsubsection"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"paragraph",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_paragraph",parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\paragraph%A{%R}","\n", 0,1,i18n("\\paragraph{title}\n\\paragraph*{title} : do not include a number and do not make an entry in the table of contents"), i18n("Paragraph"),i18n("No numbering")));
	alist.append(new KileAction::InputTag(ki,"subparagraph",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_subparagraph",parent, KileAction::ShowAlternative | KileAction::KeepHistory, "\\part%A{%R}","\n", 0,1,i18n("\\subparagraph{title}\n\\subparagraph*{title} : do not include a number and do not make an entry in the table of contents"), i18n("Subparagraph"),i18n("No numbering")));
	actionstructure_list->setItems(alist);

	KileAction::Select *actionsize_list = new KileAction::Select(i18n("Size"), 0, parent->actionCollection(), "size_list");
	alist.clear();
	alist.append(new KileAction::Tag("tiny",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{tiny}","\\end{tiny}",12,0));
	alist.append(new KileAction::Tag("scriptsize",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{scriptsize}","\\end{scriptsize}",18,0));
	alist.append(new KileAction::Tag("footnotesize",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{footnotesize}","\\end{footnotesize}",20,0));
	alist.append(new KileAction::Tag("small",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{small}","\\end{small}",13,0));
	alist.append(new KileAction::Tag("normalsize",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{normalsize}","\\end{normalsize}",18,0));
	alist.append(new KileAction::Tag("large",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{large}","\\end{large}",13,0));
	alist.append(new KileAction::Tag("Large",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{Large}","\\end{Large}",13,0));
	alist.append(new KileAction::Tag("LARGE",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\begin{LARGE}","\\end{LARGE}",13,0));
	alist.append(new KileAction::Tag("huge",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(), "","\\begin{huge}","\\end{huge}",  12,0));
	alist.append(new KileAction::Tag("Huge",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(), "","\\begin{Huge}","\\end{Huge}",  12,0));
	actionsize_list->setItems(alist);

	KileAction::Select *actionother_list = new KileAction::Select(i18n("Other"), 0, parent->actionCollection(), "other_list");
	alist.clear();
	alist.append(new KileAction::Tag("label",   0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_label", "\\label{","} ",7,0,i18n("\\label{key}")));
	alist.append(new KileAction::Tag("index",   0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_index","\\index{","}",7,0,i18n( "\\index{word}")));
	alist.append(new KileAction::Tag("footnote",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_footnote", "\\footnote{","}",10,0,i18n( "\\footnote{text}")));
	alist.append(new KileAction::InputTag(ki,"ref",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_ref", parent, KileAction::FromLabelList, "\\ref{%R","}", 5,0, QString::null, i18n("Label") ));
	alist.append(new KileAction::InputTag(ki,"pageref",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_pageref", parent, KileAction::FromLabelList, "\\pageref{%R","}", 9,0, QString::null, i18n("Label") ));
	alist.append(new KileAction::InputTag(ki,"cite",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_cite", parent, KileAction::FromBibItemList, "\\cite{%R","}", 6,0, i18n("This command generates an in-text citation to the reference associated with the ref entry in the bib file\nYou can open the bib file with Kile to see all the available references"), i18n("Reference")));
	actionother_list->setItems(alist);

	(void) new KileAction::Tag(i18n("Underline"),"text_under",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_underline", "\\underline{","}",11);

	(void) new KileAction::Tag(i18n("New Line"),"newline",Qt::SHIFT+Qt::Key_Return , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_newline","\\\\\n",QString::null,0,1);
}

void setupBibTags(KMainWindow *parent)
{
	(void) new KileAction::Tag(i18n("Article in Journal"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_article","@Article{,\nauthor = {},\ntitle = {},\njournal = {},\nyear = {},\nOPTkey = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTpages = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,9,0,i18n("Bib fields - Article in Journal\nOPT.... : optional fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Article in Conference Proceedings"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_inproc","@InProceedings{,\nauthor = {},\ntitle = {},\nbooktitle = {},\nOPTcrossref = {},\nOPTkey = {},\nOPTpages = {},\nOPTyear = {},\nOPTeditor = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTseries = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTorganization = {},\nOPTpublisher = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,15,0,i18n("Bib fields - Article in Conference Proceedings\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Article in Collection"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_incol","@InCollection{,\nauthor = {},\ntitle = {},\nbooktitle = {},\nOPTcrossref = {},\nOPTkey = {},\nOPTpages = {},\nOPTpublisher = {},\nOPTyear = {},\nOPTeditor = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTseries = {},\nOPTtype = {},\nOPTchapter = {},\nOPTaddress = {},\nOPTedition = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,14,0,i18n("Bib fields - Article in a Collection\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Chapter or Pages in Book"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_inbook","@InBook{,\nALTauthor = {},\nALTeditor = {},\ntitle = {},\nchapter = {},\npublisher = {},\nyear = {},\nOPTkey = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTseries = {},\nOPTtype = {},\nOPTaddress = {},\nOPTedition = {},\nOPTmonth = {},\nOPTpages = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,8,0,i18n("Bib fields - Chapter or Pages in a Book\nALT.... : you have the choice between these two fields\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Conference Proceedings"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_proceedings","@Proceedings{,\ntitle = {},\nyear = {},\nOPTkey = {},\nOPTeditor = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTseries = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTorganization = {},\nOPTpublisher = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,13,0,i18n("Bib fields - Conference Proceedings\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Book"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_book","@Book{,\nALTauthor = {},\nALTeditor = {},\ntitle = {},\npublisher = {},\nyear = {},\nOPTkey = {},\nOPTvolume = {},\nOPTnumber = {},\nOPTseries = {},\nOPTaddress = {},\nOPTedition = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,6,0,i18n("Bib fields - Book\nALT.... : you have the choice between these two fields\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Booklet"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_booklet","@Booklet{,\ntitle = {},\nOPTkey = {},\nOPTauthor = {},\nOPThowpublished = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTyear = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,9,0,i18n("Bib fields - Booklet\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("PhD. Thesis"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_phdthesis","@PhdThesis{,\nauthor = {},\ntitle = {},\nschool = {},\nyear = {},\nOPTkey = {},\nOPTtype = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,11,0,i18n("Bib fields - PhD. Thesis\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Master's Thesis"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_masterthesis","@MastersThesis{,\nauthor = {},\ntitle = {},\nschool = {},\nyear = {},\nOPTkey = {},\nOPTtype = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,15,0,i18n("Bib fields - Master's Thesis\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Technical Report"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_techreport","@TechReport{,\nauthor = {},\ntitle = {},\ninstitution = {},\nyear = {},\nOPTkey = {},\nOPTtype = {},\nOPTnumber = {},\nOPTaddress = {},\nOPTmonth = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,12,0,i18n("Bib fields - Technical Report\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Technical Manual"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_manual","@Manual{,\ntitle = {},\nOPTkey = {},\nOPTauthor = {},\nOPTorganization = {},\nOPTaddress = {},\nOPTedition = {},\nOPTmonth = {},\nOPTyear = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,8,0,i18n("Bib fields - Technical Manual\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Unpublished"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_unpublished","@Unpublished{,\nauthor = {},\ntitle = {},\nnote = {},\nOPTkey = {},\nOPTmonth = {},\nOPTyear = {},\nOPTannote = {}\n}\n",QString::null,13,0,i18n("Bib fields - Unpublished\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
	(void) new KileAction::Tag(i18n("Miscellaneous"),0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bib_misc","@Misc{,\nOPTkey = {},\nOPTauthor = {},\nOPTtitle = {},\nOPThowpublished = {},\nOPTmonth = {},\nOPTyear = {},\nOPTnote = {},\nOPTannote = {}\n}\n",QString::null,6,0,i18n("Bib fields - Miscellaneous\nOPT.... : optionnal fields (use the 'Clean' command to remove them)"));
}

void setupMathTags(KMainWindow *parent, QPtrList<KAction>* plist)
{
	(void) new KileAction::Tag("\\mathrm{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathrm","\\mathrm{","}",8);
	(void) new KileAction::Tag("\\mathit{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathit" ,"\\mathit{","}",8);
	(void) new KileAction::Tag("\\mathbf{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathbf" ,"\\mathbf{","}",8);
	(void) new KileAction::Tag("\\mathsf{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathsf" ,"\\mathsf{","}",8);
	(void) new KileAction::Tag("\\mathtt{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathtt" ,"\\mathtt{","}",8);
	(void) new KileAction::Tag("\\mathcal{}", 0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathcal" ,"\\mathcal{","}",9);
	(void) new KileAction::Tag("\\mathbb{}",  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathbb" ,"\\mathbb{","}",8);
	(void) new KileAction::Tag("\\mathfrak{}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathfrak" ,"\\mathfrak{","}",10);

	(void) new KileAction::Tag("\\acute{}","acute",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_acute", "\\acute{","}",7);
	(void) new KileAction::Tag("\\grave{}","grave",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_grave", "\\grave{","}", 7);
	(void) new KileAction::Tag("\\tilde{}","tilde",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_tilde", "\\tilde{","}", 7);
	(void) new KileAction::Tag("\\bar{}","bar",    0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_bar", "\\bar{","}", 5);
	(void) new KileAction::Tag("\\vec{}","vec",    0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_vec", "\\vec{","}", 5);
	(void) new KileAction::Tag("\\hat{}","hat",    0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_hat", "\\hat{","}", 5);
	(void) new KileAction::Tag("\\check{}","check",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_check", "\\check{","}", 7);
	(void) new KileAction::Tag("\\breve{}","breve",0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_breve", "\\breve{","}", 7);
	(void) new KileAction::Tag("\\dot{}","dot",    0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_dot", "\\dot{","}", 5);
	(void) new KileAction::Tag("\\ddot{}","ddot",  0 , parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_ddot", "\\ddot{","}", 6);

	(void) new KileAction::Tag(i18n("small"),  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_space_small", "\\,", QString::null, 2);
	(void) new KileAction::Tag(i18n("medium"), 0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_space_medium", "\\:", QString::null,2);
	(void) new KileAction::Tag(i18n("large"),  0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_space_large", "\\;", QString::null,2);
	(void) new KileAction::Tag("\\quad", 0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_quad", "\\quad ", QString::null, 6);
	(void) new KileAction::Tag("\\qquad",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_qquad", "\\qquad ", QString::null, 7);

	plist->append(new KileAction::Tag("$...$","mathmode",Qt::ALT+Qt::Key_M, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_mathmode","$","$",1));
	plist->append(new KileAction::Tag("$$...$$",Qt::ALT+Qt::Key_E, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_equation", "$$","$$", 2));
  	(void) new KileAction::Tag("\\begin{equation}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_equation","\\begin{equation}\n","\n\\end{equation} ",0,1);
  	(void) new KileAction::Tag("\\begin{eqnarray}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_eqnarray","\\begin{eqnarray}\n","\n\\end{eqnarray} ",0,1);
	plist->append(new KileAction::Tag("subscript  _{}","indice",Qt::ALT+Qt::Key_D, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_subscript","_{","}",2));
	plist->append(new KileAction::Tag("superscript  ^{}","puissance",Qt::ALT+Qt::Key_U, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_superscript","^{","}",2));
	plist->append(new KileAction::Tag("\\frac{}{}","smallfrac",Qt::ALT+Qt::Key_F, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_frac", "\\frac{","}{}",6));
	plist->append(new KileAction::Tag("\\dfrac{}{}","dfrac",Qt::ALT+Qt::Key_Q, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_dfrac", "\\dfrac{","}{}", 7));
	plist->append(new KileAction::Tag("\\sqrt{}","racine",Qt::ALT+Qt::Key_S, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_sqrt", "\\sqrt{","}", 6));
	plist->append(new KileAction::Tag("\\left",Qt::ALT+Qt::Key_L, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_left", "\\left", QString::null, 5));
	plist->append(new KileAction::Tag("\\right",Qt::ALT+Qt::Key_R, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_right", "\\right", QString::null, 6));
	(void) new KileAction::Tag("\\begin{array}",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"tag_env_array", "\\begin{array}{}\n", "\n\\end{array}", 14, 0,
		i18n("\\begin{array}{col1col2...coln}\ncolumn 1 entry & column 2 entry ... & column n entry \\\\ \n...\n\\end{array}\nEach column, coln, is specified by a single letter that tells how items in that column should be formatted.\n"
		"     c -- for centered \n     l -- for flush left \n     r -- for flush right\n"));

	QPtrList<KAction> alist;
  	KileAction::Select *actionleft_list = new KileAction::Select(i18n("Left Delimiter"), 0, parent->actionCollection(), "left_list");
 	alist.clear();
  	alist.append(new KileAction::Tag("left (",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left( ",QString::null,7,0));
	alist.append(new KileAction::Tag("left [",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left[ ",QString::null,7,0));
  	alist.append(new KileAction::Tag("left {",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left\\lbrace ",QString::null,14,0));
  	alist.append(new KileAction::Tag("left <",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left\\langle ",QString::null,14,0));
  	alist.append(new KileAction::Tag("left )",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left) ",QString::null,7,0));
  	alist.append(new KileAction::Tag("left ]",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left] ",QString::null,7,0));
  	alist.append(new KileAction::Tag("left }",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left\\rbrace ",QString::null,14,0));
  	alist.append(new KileAction::Tag("left >",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left\\rangle ",QString::null,14,0));
  	alist.append(new KileAction::Tag("left .",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\left. ",QString::null,7,0));

  	actionleft_list->setItems(alist);

  	KileAction::Select *actionright_list = new KileAction::Select(i18n("Right Delimiter"), 0, parent->actionCollection(), "right_list");
  	alist.clear();
  	alist.append(new KileAction::Tag("right (",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right( ",QString::null,8,0));
  	alist.append(new KileAction::Tag("right [",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right[ ",QString::null,8,0));
  	alist.append(new KileAction::Tag("right {",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right\\lbrace ",QString::null,14,0));
  	alist.append(new KileAction::Tag("right <",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right\\langle ",QString::null,14,0));
  	alist.append(new KileAction::Tag("right )",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right) ",QString::null,8,0));
  	alist.append(new KileAction::Tag("right ]",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right] ",QString::null,8,0));
  	alist.append(new KileAction::Tag("right }",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right\\rbrace ",QString::null,14,0));
  	alist.append(new KileAction::Tag("right >",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right\\rangle ",QString::null,14,0));
  	alist.append(new KileAction::Tag("right.",0, parent, SLOT(insertTag(const KileAction::TagData&)), parent->actionCollection(),"","\\right. ",QString::null,8,0));

  	actionright_list->setItems(alist);
}

}
