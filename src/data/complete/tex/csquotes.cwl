# csquotes, http://www.ctan.org/tex-archive/macros/latex/contrib/csquotes
# tbraun 10/8/2012

\enquote{text}
\enquote*{text}
\foreignquote{lang}{text}
\foreignquote*{lang}{text}
\hyphenquote{lang}{text}
\hyphenquote*{lang}{text}

\textquote[cite][punct]{text}<tpunct>
\textquote[cite]{text}<tpunct>
\textquote{text}<tpunct>
\textquote[cite][punct]{text}
\textquote[cite]{text}
\textquote{text}

\textquote*[cite][punct]{text}<tpunct>
\textquote*[cite]{text}<tpunct>
\textquote*{text}<tpunct>
\textquote*[cite][punct]{text}
\textquote*[cite]{text}
\textquote*{text}

\foreigntextquote{lang}[cite][punct]{text}<tpunct>
\foreigntextquote{lang}[cite]{text}<tpunct>
\foreigntextquote{lang}{text}<tpunct>
\foreigntextquote{lang}[cite][punct]{text}
\foreigntextquote{lang}[cite]{text}
\foreigntextquote{lang}{text}

\foreigntextquote*{lang}[cite][punct]{text}<tpunct>
\foreigntextquote*{lang}[cite]{text}<tpunct>
\foreigntextquote*{lang}{text}<tpunct>
\foreigntextquote*{lang}[cite][punct]{text}
\foreigntextquote*{lang}[cite]{text}
\foreigntextquote*{lang}{text}

\hyphentextquote{lang}[cite][punct]{text}<tpunct>
\hyphentextquote{lang}[cite]{text}<tpunct>
\hyphentextquote{lang}{text}<tpunct>
\hyphentextquote{lang}[cite][punct]{text}
\hyphentextquote{lang}[cite]{text}
\hyphentextquote{lang}{text}

\hyphentextquote*{lang}[cite][punct]{text}<tpunct>
\hyphentextquote*{lang}[cite]{text}<tpunct>
\hyphentextquote*{lang}{text}<tpunct>
\hyphentextquote*{lang}[cite][punct]{text}
\hyphentextquote*{lang}[cite]{text}
\hyphentextquote*{lang}{text}

\blockquote[cite][punct]{text}<tpunct>
\blockquote[cite]{text}<tpunct>
\blockquote{text}<tpunct>
\blockquote[cite][punct]{text}
\blockquote[cite]{text}
\blockquote{text}

\foreignblockquote{lang}[cite][punct]{text}<tpunct>
\foreignblockquote{lang}[cite]{text}<tpunct>
\foreignblockquote{lang}{text}<tpunct>
\foreignblockquote{lang}[cite][punct]{text}
\foreignblockquote{lang}[cite]{text}
\foreignblockquote{lang}{text}

\hyphenblockquote{lang}[cite][punct]{text}<tpunct>
\hyphenblockquote{lang}[cite]{text}<tpunct>
\hyphenblockquote{lang}{text}<tpunct>
\hyphenblockquote{lang}[cite][punct]{text}
\hyphenblockquote{lang}[cite]{text}
\hyphenblockquote{lang}{text}

\hybridblockquote{lang}[cite][punct]{text}<tpunct>
\hybridblockquote{lang}[cite]{text}<tpunct>
\hybridblockquote{lang}{text}<tpunct>
\hybridblockquote{lang}[cite][punct]{text}
\hybridblockquote{lang}[cite]{text}
\hybridblockquote{lang}{text}

\setquotestyle[variant]{style}
\setquotestyle{alias}
\setquotestyle*

\MakeOuterQuote{char}
\MakeInnerQuote{char}
\MakeAutoQuote{char1}{char2}
\MakeAutoQuote*{char1}{char2}
\MakeForeignQuote{lang}{char1}{char2}
\MakeForeignQuote*{lang}{char1}{char2}
\MakeHyphenQuote{lang}{char1}{char2}
\MakeHyphenQuote*{lang}{char1}{char2}
\MakeBlockQuote{char1}{delim}{char2}
\MakeForeignBlockQuote{lang}{char1}{delim}{char2}
\MakeHyphenBlockQuote{lang}{char1}{delim}{char2}
\MakeHybridBlockQuote{lang}{char1}{delim}{char2}

\EnableQuotes
\DisableQuotes
\VerbatimQuotes
\DeleteQuotes

# advanced commands
\textcquote[prenote][postnote]{key}[punct]{text}<tpunct>
\textcquote*[prenote][postnote]{key}[punct]{text}<tpunct>
\foreigntextcquote{lang}[prenote][postnote]{key}[punct]{text}<tpunct>
\foreigntextcquote*{lang}[prenote][postnote]{key}[punct]{text}<tpunct>
\hyphentextcquote{lang}[prenote][postnote]{key}[punct]{text}<tpunct>
\hyphentextcquote*{lang}[prenote][postnote]{key}[punct]{text}<tpunct>

\blockcquote[prenote][postnote]{key}[punct]{text}<tpunct>
\foreignblockcquote{lang}[prenote][postnote]{key}[punct]{text}<tpunct>
\hyphenblockcquote{lang}[prenote][postnote]{key}[punct]{text}<tpunct>
\hybridblockcquote{lang}[prenote][postnote]{key}[punct]{text}<tpunct>

\begin{displayquote}[cite][punct]
\begin{displayquote}[cite]
\begin{displayquote}
\end{displayquote}

\begin{foreigndisplayquote}{lang}[cite][punct]
\begin{foreigndisplayquote}{lang}[cite]
\begin{foreigndisplayquote}{lang}
\end{foreigndisplayquote}
\begin{hyphendisplayquote}{lang}[cite][punct]
\begin{hyphendisplayquote}{lang}[cite]
\begin{hyphendisplayquote}{lang}
\end{hyphendisplayquote}

\begin{displaycquote}[prenote][postnote]{key}[punct]
\begin{displaycquote}{key}[punct]
\begin{displaycquote}{key}
\end{displaycquote}

\begin{foreigndisplaycquote}{lang}[prenote][postnote]{key}[punct]
\begin{foreigndisplaycquote}{lang}{key}[punct]
\begin{foreigndisplaycquote}{lang}{key}
\end{foreigndisplaycquote}
\begin{hyphendisplaycquote}{lang}[prenote][postnote]{key}[punct]
\begin{hyphendisplaycquote}{lang}{key}[punct]
\begin{hyphendisplaycquote}{lang}{key}
\end{hyphendisplaycquote}

# auxiliary commands
\textelp{}
\textelp{text}
\textelp*{text}

\textins{}
\textins*{}

\DeclareQuoteStyle[variant]{style}[outer init][inner init]{opening outer mark}[middle outer mark]{closing outer mark}[kern]{opening inner mark}[middle inner mark]{closing inner mark}
\DeclareQuoteAlias[variant]{style}{alias}
\DeclareQuoteAlias{ﬁrst-levelalias}{second-levelalias}
\DeclareQuoteOption{style}
\ExecuteQuoteOptions{key=value}
\DeclarePlainStyle{opening outer mark}{closing outer mark}{opening inner mark}{closing inner mark}
\SetBlockThreshold{integer}
\SetBlockEnvironment{environment}
\SetCiteCommand{command}
\mkcitation{cite}
\mkccitation{citecode}
\mktextquote{open}{text}{close}{punct}{tpunct}{cite}
\mkblockquote{text}{punct}{tpunct}{cite}
\mkbegdispquote{punct}{cite}
\mkenddispquote{punct}{cite}
\ifpunctmark{character}{true}{false}
\ifpunct{true}{false}
\ifterm{true}{false}
\iftextpunctmark{text}{character}{true}{false}
\iftextpunct{text}{true}{false}
\iftextterm{text}{true}{false}
\ifblockquote{true}{false}
\ifblank{string}{true}{false}
\unspace
\DeclareAutoPunct{characters}