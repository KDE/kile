# biblatex package, available from ctan
# commands for biblatex style authors
# tbraun, 19.08.2009

\RequireBibliographyStyle{style}
\InitializeBibliographyStyle{code}
\DeclareBibliographyDriver{type}{code}
\DeclareBibliographyAlias{alias}{type}

\DeclareBibliographyOption{key}{code}
\DeclareBibliographyOption{key}[value]{code}

\DeclareEntryOption{key}{code}
\DeclareEntryOption{key}[value]{code}

\RequireCitationStyle{style}
\InitializeCitationStyle{code}
\OnManualCitation{code}

\DeclareCiteCommand{command}{precode}{loopcode}{sepcode}{postcode}
\DeclareCiteCommand{command}[wrapper]{precode}{loopcode}{sepcode}{postcode}

\DeclareCiteCommand*{command}{precode}{loopcode}{sepcode}{postcode}
\DeclareCiteCommand*{command}[wrapper]{precode}{loopcode}{sepcode}{postcode}

\DeclareMultiCiteCommand{command}{cite}{delimiter}
\DeclareMultiCiteCommand{command}[wrapper]{cite}{delimiter}

\DeclareAutoCiteCommand{name}{cite}{multicite}
\DeclareAutoCiteCommand{name}[position]{cite}{multicite}

\printfield{ﬁeld}
\printfield[format]{ﬁeld}

\printlist{literal list}
\printlist[format]{literal list}
\printlist[format][start – stop]{literal list}

\printnames{name list}
\printnames[format]{name list}
\printnames[format][start – stop]{name list}

\printtext{text}
\printtext[format]{text}

\printfile{ﬁle}
\printfile[format]{ﬁle}

\indexfield{ﬁeld}
\indexfield[format]{ﬁeld}

\indexlist{literal list}
\indexlist[format]{literal list}
\indexlist[format][start – stop]{literal list}

\indexnames{literal list}
\indexnames[format]{literal list}
\indexnames[format][start – stop]{literal list}

\entrydata{key}{code}
\entryset{precode}{postcode}

\DeclareFieldFormat{format}{code}
\DeclareFieldFormat[entry type]{format}{code}

\DeclareListFormat{format}{code}
\DeclareListFormat[entry type]{format}{code}

\DeclareNameFormat{format}{code}
\DeclareNameFormat[entry type]{format}{code}

\DeclareIndexFieldFormat{format}{code}
\DeclareIndexFieldFormat[entry type]{format}{code}

\DeclareIndexListFormat{format}{code}
\DeclareIndexListFormat[entry type]{format}{code}

\DeclareIndexNameFormat{format}{code}
\DeclareIndexNameFormat[entry type]{format}{code}

\DeclareFieldAlias{alias}{format}
\DeclareFieldAlias[entry type]{alias}{format}
\DeclareFieldAlias{alias}[format entry type]{format}
\DeclareFieldAlias[entry type]{alias}[format entry type]{format}

\DeclareListAlias{alias}{format}
\DeclareListAlias[entry type]{alias}{format}
\DeclareListAlias{alias}[format entry type]{format}
\DeclareListAlias[entry type]{alias}[format entry type]{format}

\DeclareNameAlias{alias}{format}
\DeclareNameAlias[entry type]{alias}{format}
\DeclareNameAlias{alias}[format entry type]{format}
\DeclareNameAlias[entry type]{alias}[format entry type]{format}

\DeclareIndexFieldAlias{alias}{format}
\DeclareIndexFieldAlias[entry type]{alias}{format}
\DeclareIndexFieldAlias{alias}[format entry type]{format}
\DeclareIndexFieldAlias[entry type]{alias}[format entry type]{format}

\DeclareIndexListAlias{alias}{format}
\DeclareIndexListAlias[entry type]{alias}{format}
\DeclareIndexListAlias{alias}[format entry type]{format}
\DeclareIndexListAlias[entry type]{alias}[format entry type]{format}

\DeclareIndexNameAlias{alias}{format}
\DeclareIndexNameAlias[entry type]{alias}{format}
\DeclareIndexNameAlias{alias}[format entry type]{format}
\DeclareIndexNameAlias[entry type]{alias}[format entry type]{format}

\thefield{ﬁeld}
\strfield{ﬁeld}
\thelist{literal list}
\thename{name list}

\savefield{ﬁeld}{macro}
\savefield*{ﬁeld}{macro}

\savename{name list}{macro}
\savename*{name list}{macro}

\savefieldcs{ﬁeld}{csname}
\savefieldcs*{ﬁeld}{csname}

\savelistcs{literal list}{csname}
\savelistcs*{literal list}{csname}

\savenamecs{name list}{csname}
\savenamecs*{name list}{csname}

\restorefield{ﬁeld}{macro}
\restorelist{literal list}{macro}
\restorename{name list}{macro}
\clearfield{ﬁeld}
\clearlist{literal list}
\clearname{name list}

# tests
\iffieldundef{ﬁeld}{true}{false}
\iflistundef{literal list}{true}{false}
\ifnameundef{name list}{true}{false}
\iffieldsequal{ﬁeld 1}{ﬁeld 2}{true}{false}
\iflistsequal{literal list 1}{literal list 2}{true}{false}
\ifnamesequal{name list 1}{name list 2}{true}{false}
\iffieldequals{ﬁeld}{macro}{true}{false}
\iflistequals{literal list}{macro}{true}{false}
\ifnameequals{name list}{macro}{true}{false}
\iffieldequalcs{ﬁeld}{csname}{true}{false}
\iflistequalcs{literal list}{csname}{true}{false}
\ifnameequalcs{name list}{csname}{true}{false}
\iffieldequalstr{ﬁeld}{string}{true}{false}
\iffieldxref{ﬁeld}{true}{false}
\iflistxref{literal list}{true}{false}
\ifnamexref{name list}{true}{false}
\ifcurrentfield{ﬁeld}{true}{false}
\ifcurrentlist{literal list}{true}{false}
\ifcurrentname{name list}{true}{false}
\ifuseprefix{true}{false}
\ifuseauthor{true}{false}
\ifuseeditor{true}{false}
\ifsingletitle{true}{false}
\ifandothers{list}{true}{false}
\ifmorenames{true}{false}
\ifmoreitems{true}{false}
\iffirstinits{true}{false}
\ifciteseen{true}{false}
\ifciteibid{true}{false}
\ifopcit{true}{false}
\ifloccit{true}{false}
\iffirstonpage{true}{false}
\ifsamepage{instance 1}{instance 2}{true}{false}
\ifinteger{string}{true}{false}
\ifnumeral{string}{true}{false}
\ifnumerals{string}{true}{false}
\ifbibstring{string}{true}{false}
\iffieldbibstring{ﬁeld}{true}{false}
\ifcapital{true}{false}
\ifcitation{true}{false}
\ifbibliography{true}{false}
\iffootnote{true}{false}
uniquename # counter
\ifthenelse{tests}{true}{false}

# misc

\newbibmacro{name}{deﬁnition}
\newbibmacro{name}[arguments]{deﬁnition}
\newbibmacro{name}[arguments][optional]{deﬁnition}

\newbibmacro*{name}{deﬁnition}
\newbibmacro*{name}[arguments]{deﬁnition}
\newbibmacro*{name}[arguments][optional]{deﬁnition}

\renewbibmacro{name}{deﬁnition}
\renewbibmacro{name}[arguments]{deﬁnition}
\renewbibmacro{name}[arguments][optional]{deﬁnition}

\renewbibmacro*{name}{deﬁnition}
\renewbibmacro*{name}[arguments]{deﬁnition}
\renewbibmacro*{name}[arguments][optional]{deﬁnition}

\usebibmacro{name}
\savecommand{command}
\restorecommand{command}
\savebibmacro{name}
\restorebibmacro{name}

\savefieldformat{format}
\savefieldformat[entry type]{format}

\restorefieldformat{format}
\restorefieldformat[entry type]{format}

\savelistformat{format}
\savelistformat[entry type]{format}

\restorelistformat{format}
\restorelistformat[entry type]{format}

\savenameformat{format}
\savenameformat[entry type]{format}

\restorenameformat{format}
\restorenameformat[entry type]{format}

\usedriver{code}{type}
\bibhypertarget{name}{text}
\bibhyperlink{name}{text}
\bibhyperlink{name}{text}

\bibhyperref{text}
\bibhyperref[entrykey]{text}

\ifhyperref{true}{false}
\docsvfield{ﬁeld}
\MakeCapital{text}
\MakeSentenceCase{text}
\MakeSentenceCase*{text}

\mkpageprefix{text}
\mkpageprefix[pagination]{text}

\mkpagetotal{text}
\mkpagetotal[pagination]{text}

\DeclareNumChars{characters}
\DeclareNumChars*{characters}
\DeclareRangeChars{characters}
\DeclareRangeChars*{characters}
\DeclareRangeCommands{commands}
\DeclareRangeCommands*{commands}
\NumCheckSetup{code}
\DeclareCaseLangs{languages}
\DeclareCaseLangs*{languages}
\BibliographyWarning{message}
\pagetrackertrue
\pagetrackerfalse
\citetrackertrue
\citetrackerfalse

# punctuation and spacing

\newblock
\newunit
\finentry
\setunit{punctuation}
\setunit*{punctuation}
\setpunctfont{command}
\resetpunctfont
\ifpunct{true}{false}
\ifterm{true}{false}
\ifpunctmark{character}{true}{false}
\adddot
\addcomma
\addsemicolon
\addcolon
\addperiod
\addexclam
\addquestion
\isdot
\nopunct
\unspace
\addspace
\addnbspace
\addthinspace
\addnbthinspace
\addlowpenspace
\addhighpenspace
\addlpthinspace
\addhpthinspace
\addabbrvspace
\adddotspace
\addslash
\DeclareAutoPunctuation{characters}
\DeclareCapitalPunctuation{characters}
\DeclarePunctuationPairs{identiﬁer}{characters}
\DeclareQuotePunctuation{characters}
\bibsentence
\midsentence

# bibliography strings
\bibstring{key}
\bibstring[wrapper]{key}

\bibcpstring{key}
\bibcpstring[wrapper]{key}

\biblcstring{key}
\biblcstring[wrapper]{key}
\bibxstring{key}

# localization
\DeclareBibliographyStrings{deﬁnitions}
\InheritBibliographyStrings{language}
\DeclareBibliographyExtras{code}
\UndeclareBibliographyExtras{code}
\InheritBibliographyExtras{language}
\DeclareHyphenationExceptions{text}
\DeclareLanguageMapping{language}{ﬁle}
\NewBibliographyString{key}

# formatting

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
\prenotedelim
\postnotedelim

\mkbibnamelast{text}
\mkbibnamefirst{text}
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

# misc
\mkbibemph{text}
\mkbibquote{text}
\mkbibparens{text}
\mkbibbrackets{text}
\mkbibfootnote{text}
\mkbibsuperscript{text}
\mkbibmonth{integer}

\bibdate
\biburldate
shorthandwidth
labelnumberwidth
labelalphawidth
bibhyperref
bibhyperlink
bibhypertarget
\shorthandwidth
\labelnumberwidth
\labelalphawidth
maxlabelyear
maxextraalpha
refsection
refsegment
maxnames
minnames
maxitems
minitems
instcount
citetotal
citecount
multicitetotal
multicitecount
listtotal
listcount
liststart
liststop
\currentfield
\currentlist
\currentname
\AtBeginBibliography{code}
\AtBeginShorthands{code}
\AtEveryBibitem{code}
\AtEveryLositem{code}
\AtEveryCite{code}
\AtEveryCitekey{code}
\AtNextCite{code}
\AtNextCitekey{code}

\AtDataInput{code}
\AtDataInput[type]{code}

