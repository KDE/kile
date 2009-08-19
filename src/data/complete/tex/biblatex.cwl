# biblatex package, available from ctan
# commands for biblatex users
# tbraun, 19.08.2009

\ExecuteBibliographyOptions{key=value, ...}
\bibliography{bib ﬁles}
\printbibliography
\printbibliography[key=value, ...]
\bibbysection
\bibbysection[key=value, ...]
\bibbysegment
\bibbysegment[key=value, ...]
\bibbycategory
\bibbycategory[key=value, ...]
\printshorthands
\printshorthands[key=value, ...]
\begin{refsection}
\begin{refsection}[bib ﬁles]
\end{refsection}
\newrefsection[bib ﬁles]
\begin{refsegment}
\end{refsegment}
\newrefsegment
\DeclareBibliographyCategory{category}
\addtocategory{category}{key}
\defbibheading{name}{code}
\defbibnote{name}{text}
\defbibfilter{name}{code}
\segment{integer}
\type{type}
\keyword{keyword}
\category{category}

\cite[prenote][postnote]{key}
\cite[postnote]{key}
\cite{key}
\Cite[prenote][postnote]{key}
\Cite[postnote]{key}
\Cite{key}

\parencite[prenote][postnote]{key}
\parencite[postnote]{key}
\parencite{key}
\Parencite[prenote][postnote]{key}
\Parencite[postnote]{key}
\Parencite{key}

\footcite[prenote][postnote]{key}
\footcite[postnote]{key}
\footcite{key}
\Footcite[prenote][postnote]{key}
\Footcite[postnote]{key}
\Footcite{key}

\textcite[prenote][postnote]{key}
\textcite[postnote]{key}
\textcite{key}
\Textcite[prenote][postnote]{key}
\Textcite[postnote]{key}
\Textcite{key}

\cite*[prenote][postnote]{key}
\cite*[postnote]{key}
\cite*{key}

\parencite*[prenote][postnote]{key}
\parencite*[postnote]{key}
\parencite*{key}

\supercite{key}

\cites{key}{key}
\cites(post){key}{key}
\cites(pre)(post){key}{key}
\cites(pre)(post)[post]{key}[post]{key}
\cites(pre)(post)[pre][post]{key}[pre][post]{key}
\cites[post]{key}[post]{key}
\cites[pre][post]{key}[pre][post]{key}
\cites(pre)(post)[pre][post]{key}[pre][post]{key}

\Cites{key}{key}
\Cites(post){key}{key}
\Cites(pre)(post){key}{key}
\Cites(pre)(post)[post]{key}[post]{key}
\Cites(pre)(post)[pre][post]{key}[pre][post]{key}
\Cites[post]{key}[post]{key}
\Cites[pre][post]{key}[pre][post]{key}
\Cites(pre)(post)[pre][post]{key}[pre][post]{key}

\parencites{key}{key}
\parencites(post){key}{key}
\parencites(pre)(post){key}{key}
\parencites(pre)(post)[post]{key}[post]{key}
\parencites(pre)(post)[pre][post]{key}[pre][post]{key}
\parencites[post]{key}[post]{key}
\parencites[pre][post]{key}[pre][post]{key}
\parencites(pre)(post)[pre][post]{key}[pre][post]{key}

\Parencites{key}{key}
\Parencites(post){key}{key}
\Parencites(pre)(post){key}{key}
\Parencites(pre)(post)[post]{key}[post]{key}
\Parencites(pre)(post)[pre][post]{key}[pre][post]{key}
\Parencites[post]{key}[post]{key}
\Parencites[pre][post]{key}[pre][post]{key}
\Parencites(pre)(post)[pre][post]{key}[pre][post]{key}

\footcites{key}{key}
\footcites(post){key}{key}
\footcites(pre)(post){key}{key}
\footcites(pre)(post)[post]{key}[post]{key}
\footcites(pre)(post)[pre][post]{key}[pre][post]{key}
\footcites[post]{key}[post]{key}
\footcites[pre][post]{key}[pre][post]{key}
\footcites(pre)(post)[pre][post]{key}[pre][post]{key}

\Footcites{key}{key}
\Footcites(post){key}{key}
\Footcites(pre)(post){key}{key}
\Footcites(pre)(post)[post]{key}[post]{key}
\Footcites(pre)(post)[pre][post]{key}[pre][post]{key}
\Footcites[post]{key}[post]{key}
\Footcites[pre][post]{key}[pre][post]{key}
\Footcites(pre)(post)[pre][post]{key}[pre][post]{key}

\supercites{key}{key}
\supercites(post){key}{key}
\supercites(pre)(post){key}{key}
\supercites(pre)(post)[post]{key}[post]{key}
\supercites(pre)(post)[pre][post]{key}[pre][post]{key}
\supercites[post]{key}[post]{key}
\supercites[pre][post]{key}[pre][post]{key}
\supercites(pre)(post)[pre][post]{key}[pre][post]{key}

\autocite[prenote][postnote]{key}
\autocite[postnote]{key}
\autocite{key}

\Autocite[prenote][postnote]{key}
\Autocite[postnote]{key}
\Autocite{key}

\autocite*[prenote][postnote]{key}
\autocite*[postnote]{key}
\autocite*{key}

\Autocite*[prenote][postnote]{key}
\Autocite*[postnote]{key}
\Autocite*{key}

\autocites{key}{key}
\autocites(post){key}{key}
\autocites(pre)(post){key}{key}
\autocites(pre)(post)[post]{key}[post]{key}
\autocites(pre)(post)[pre][post]{key}[pre][post]{key}
\autocites[post]{key}[post]{key}
\autocites[pre][post]{key}[pre][post]{key}
\autocites(pre)(post)[pre][post]{key}[pre][post]{key}

\Autocites{key}{key}
\Autocites(post){key}{key}
\Autocites(pre)(post){key}{key}
\Autocites(pre)(post)[post]{key}[post]{key}
\Autocites(pre)(post)[pre][post]{key}[pre][post]{key}
\Autocites[post]{key}[post]{key}
\Autocites[pre][post]{key}[pre][post]{key}
\Autocites(pre)(post)[pre][post]{key}[pre][post]{key}

\citeauthor[prenote][postnote]{key}
\citeauthor[postnote]{key}
\citeauthor{key}

\Citeauthor[prenote][postnote]{key}
\Citeauthor[postnote]{key}
\Citeauthor{key}

\citetitle[prenote][postnote]{key}
\citetitle[postnote]{key}
\citetitle{key}

\citetitle*[prenote][postnote]{key}
\citetitle*[postnote]{key}
\citetitle*{key}

\citeyear[prenote][postnote]{key}
\citeyear[postnote]{key}
\citeyear{key}

\citeurl[prenote][postnote]{key}
\citeurl[postnote]{key}
\citeurl{key}

\nocite{key}

\fullcite[prenote][postnote]{key}
\fullcite[postnote]{key}
\fullcite{key}

\footfullcite[prenote][postnote]{key}
\footfullcite[postnote]{key}
\footfullcite{key}

\citename[prenote][postnote]{key}[format]{name list}
\citename[postnote]{key}[format]{name list}
\citename[postnote]{key}{name list}
\citename{key}[format]{name list}
\citename{key}{name list}

\citelist[prenote][postnote]{key}[format]{literallist}
\citelist[postnote]{key}[format]{literal list}
\citelist[postnote]{key}{literal list}
\citelist{key}[format]{literal list}
\citelist{key}{literal list}

\citefield[prenote][postnote]{key}[format]{field}
\citefield[postnote]{key}[format]{field}
\citefield[postnote]{key}{field}
\citefield{key}[format]{field}
\citefield{key}{field}

\citereset
\citereset*
\mancite

\pno
\ppno
\nopp
\psq
\psqq
\RN{integer}
\Rn{integer}

\DefineBibliographyStrings{language}{deﬁnitions}
\DefineBibliographyExtras{language}{code}
\UndefineBibliographyExtras{language}{code}
\DefineHyphenationExceptions{language}{text}
\NewBibliographyString{key}

\bibsetup
\bibfont
\citesetup
\newblockpunct
\newunitpunct
\finentrypunct
\bibleftparen
\bibrightparen
\bibleftbracket
\bibrightbracket
\bibnamedash
\labelnamepunct
\subtitlepunct
\bibpagespunct
\multinamedelim
\finalnamedelim
\revsdnamedelim
\andothersdelim
\multilistdelim
\finallistdelim
\andmoredelim
\multicitedelim
\supercitedelim
\compcitedelim
\nameyeardelim
\labelalphaothers
\prenotedelim
\postnotedelim
\mkbibnamelast{text}
\mkbibnamefirst{text}
\mkbibnameprefix{text}
\mkbibnameaffix{text}

\bibrangedash
\bibdatelong
\bibdateshort
\biburldatelong
\biburldateshort
\finalandcomma
\mkbibordinal{integer}
\mkbibmascord{integer}
\mkbibfemord{integer}
\bibhang
\biblabelsep
\bibitemsep
\bibitemextrasep
\bibparsep
abbrvpenalty # counter
lownamepenalty
highnamepenalty

\bibellipsis
\hyphenate
\hyphen
\nbhyphen
\noligature
\mknumalph{integer}
\mkbibacro{text}
\autocap{character}

# spanish
smartand # counter
\forceE
\forceY

# koma-script classes
\ifkomabibtotoc{true}{false}
\ifkomabibtotocnumbered{true}{false}

# memoir class
\ifmemoirbibintoc{true}{false}

