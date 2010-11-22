CWL File Specification 0.1:

Completion modes:
	Kile supports three completion modes, which use their own cwl files:
	- (La)TeX mode 
	- Dictionary mode
	- Abbreviation mode

Call of completion:
	When completion is called with a shortcut or in auto mode, Kile must separate words, so that some restriction had to be made about the already typed prefix. If the prefix is to be analyzed as valid, the completion box will open, otherwise not.
	- (La)TeX mode: all prefixes must start with a backslash, followed by letters, possibly with a trailing star 
	- Dictionary mode: letters, digits and underscore are valid characters
	- Abbreviation mode: letters, digits and underscore are valid characters
	Once the completion box is opened, there are no further restrictions. All characters are valid from this moment on.

Naming scheme of the cwl file:
	- If the commands in the file belong to a class the name is class-foo.cwl, if they belong to different classes the name is class-foo,bar.cwl.
	- In any other case it belongs to a package and should therefore have the same name than the package (package foo -> foo.cwl)

File syntax:
	- There are only two types of content, comments and text.
	- Comments start with a #, everything after # is ignored till the end of the line.
	- Everything else is text.

File content:
	- The files should have a small header including your name (email address is not needed), the date, and a link where the package or the class file can be downloaded.
	  For example the header of textcomp.cwl:
		# textcomp package
		# tbraun 04/27/2006
		# URL: http://www.ctan.org/tex-archive/help/Catalogue/entries/textcomp.html
	- All commands given must have a description what to insert in the empty brackets:
		-- \label{key} and not \label{}
		-- \includegraphics[options]{name}
		-- \footnotetext[number]{text} and so on
	- All commands means all commands. So please add all possible combinations, including mandatory options and all optional options in all possible combinations.
	  This list can be quite long but all users will appreciate the completeness.
	  For example (taken from class-beamer.cwl):
		  \begin{frame}
		  \begin{frame}<overlay specification>
		  \begin{frame}<overlay specification>[<default overlay specification>]
		  \begin{frame}<overlay specification>[<default overlay specification>][options]
		  \begin{frame}<overlay specification>[<default overlay specification>][options]{title}
		  \begin{frame}<overlay specification>[<default overlay specification>][options]{title}{subtitle}
		  \begin{frame}[<default overlay specification>]
		  \begin{frame}[<default overlay specification>][options]
		  \begin{frame}[<default overlay specification>][options]{title}
		  \begin{frame}[<default overlay specification>][options]{title}{subtitle}
		  \begin{frame}[<default overlay specification>][options]{title}{subtitle}
		  \begin{frame}[options]
		  \begin{frame}[options]{title}
		  \begin{frame}[options]{title}{subtitle}
		  \begin{frame}[options]{title}{subtitle}
		  \begin{frame}{title}
		  \begin{frame}{subtitle}
		  \end{frame}
	- Only include the \end{env} command once, but all combinations with \begin{env}{}[]<>...
	- If you declare an environment in which the \item command is valid you have to suffix the \begin{...} declaration with \item, like in latex-document.cwl  \begin{itemize}\item.
	- A starred version of a command must also be added
		  \circle{diameter}
		  \circle*{diameter}
		
Getting assistance:
	- You can see some examples under http://projects.kde.org/projects/extragear/office/kile/repository/revisions/master/show/src/data/complete/tex
	- If in doubt don't hesitate to mail to kile-devel@lists.sourceforge.net and ask, we will be glad to help you :)

The Kile development team
