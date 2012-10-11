# etoolbox, http://www.ctan.org/tex-archive/macros/latex/contrib/etoolbox
# tbraun, 10/8/2012

\newrobustcmd{command}[arguments][optargdefault]{replacement text}
\newrobustcmd{command}[arguments]{replacement text}
\newrobustcmd{command}{replacement text}

\newrobustcmd*{command}[arguments][optargdefault]{replacement text}
\newrobustcmd*{command}[arguments]{replacement text}
\newrobustcmd*{command}{replacement text}

\renewrobustcmd{command}[arguments][optargdefault]{replacement text}
\renewrobustcmd{command}[arguments]{replacement text}
\renewrobustcmd{command}{replacement text}

\renewrobustcmd*{command}[arguments][optargdefault]{replacement text}
\renewrobustcmd*{command}[arguments]{replacement text}
\renewrobustcmd*{command}{replacement text}

\providerobustcmd{command}[arguments][optargdefault]{replacement text}
\providerobustcmd{command}[arguments]{replacement text}
\providerobustcmd{command}{replacement text}

\providerobustcmd*{command}[arguments][optargdefault]{replacement text}
\providerobustcmd*{command}[arguments]{replacement text}
\providerobustcmd*{command}{replacement text}

\robustify{command}
\protecting{code}
\defcounter{counter}{integer expression}
\deflength{length}{glue expression}


\AfterPreamble{code}
\AtEndPreamble{code}
\AfterEndPreamble{code}
\AfterEndDocument{code}
\AtBeginEnvironment{environment}{code}
\AtEndEnvironment{environment}{code}
\BeforeBeginEnvironment{environment}{code}
\AfterEndEnvironment{environment}{code}
\csdef{csname}{replacement text}
\csgdef{csname}{replacement text}
\csedef{csname}{replacement text}
\csxdef{csname}{replacement text}
\protected@csedef{csname}{replacement text}
\protected@csxdef{csname}{replacement text}
\cslet{csname}{command}
\letcs{command}{csname}
\csletcs{csname}{csname}
\csuse{csname}
\undef
\csundef{csname}
\csshow{csname}
\numdefcommand{integer expression}
\numgdefcommand{integer expression}
\csnumdef{csname}{integer expression}
\csnumgdef{csname}{integer expression}
\dimdefcommand{dimen expression}
\dimgdefcommand{dimen expression}
\csdimdef{csname}{dimen expression}
\csdimgdef{csname}{dimen expression}
\gluedef{glue expression}
\gluegdefcommand{glue expression}
\csgluedef{csname}{glue expression}
\csgluegdef{csname}{glue expression}
\mudef{muglue expression}
\mugdef{muglue expression}
\csmudef{csname}{muglue expression}
\csmugdef{csname}{muglue expression}
\expandonce
\csexpandonce{csname}

\appto{code}
\gappto{code}
\eappto{code}
\xappto{code}
\protected@eappto{code}
\protected@xappto{code}

\csappto{csname}{code}
\csgappto{csname}{code}
\cseappto{csname}{code}
\csxappto{csname}{code}
\protected@cseappto{code}
\protected@csxappto{code}

\preto{code}
\gpreto{code}
\epreto{code}
\xpreto{code}
\protected@epreto{code}
\protected@xpreto{code}
\cspreto{csname}{code}
\csgpreto{csname}{code}
\csepreto{csname}{code}
\csxpreto{csname}{code}
\protected@csepreto{code}
\protected@csxpreto{code}

#patches

\patchcmd[preﬁx]{command}{search}{replace}{success}{failure}
\patchcmd{command}{search}{replace}{success}{failure}

\ifpatchable{command}{search}{true}{false}
\ifpatchable*{command}{true}{false}
\apptocmd{command}{code}{success}{failure}
\pretocmd{command}{code}{success}{failure}
\tracingpatches

#tex boolean flags

\newbool{name}
\providebool{name}
\booltrue{name}
\boolfalse{name}
\setbool{name}{value}
\ifbool{name}{true}{false}
\notbool{name}{nottrue}{notfalse}

#latex toggles
\newtoggle{name}
\providetoggle{name}
\toggletrue{name}
\togglefalse{name}
\settoggle{name}{value}
\iftoggle{name}{true}{false}
\nottoggle{name}{nottrue}{notfalse}
\ifdef{controlsequence}{true}{false}
\ifcsdef{csname}{true}{false}
\ifundef{controlsequence}{true}{false}
\ifcsundef{csname}{true}{false}

\ifdefmacro{controlsequence}{true}{false}
\ifcsmacro{csname}{true}{false}
\ifdefparam{controlsequence}{true}{false}
\ifcsparam{csname}{true}{false}
\ifdefprefix{controlsequence}{true}{false}
\ifcsprefix{csname}{true}{false}
\ifdefprotected{controlsequence}{true}{false}
\ifcsprotected{csname}{true}{false}
\ifdefltxprotect{controlsequence}{true}{false}
\ifcsltxprotect{csname}{true}{false}

\ifdefempty{controlsequence}{true}{false}
\ifcsempty{csname}{true}{false}
\ifdefvoid{controlsequence}{true}{false}
\ifcsvoid{csname}{true}{false}
\ifdefequal{controlsequence}{controlsequence}{true}{false}
\ifcsequal{csname}{csname}{true}{false}
\ifdefstring{command}{string}{true}{false}
\ifcsstring{csname}{string}{true}{false}
\ifdefstrequal{command}{command}{true}{false}
\ifcsstrequal{csname}{csname}{true}{false}
\ifdefcounter{controlsequence}{true}{false}
\ifcscounter{csname}{true}{false}
\ifltxcounter{name}{true}{false}
\ifdeflength{controlsequence}{true}{false}
\ifdefdimen{controlsequence}{true}{false}
\ifcsdimen{csname}{true}{false}
\ifstrequal{string}{string}{true}{false}
\ifstrempty{string}{true}{false}
\ifblank{string}{true}{false}
\notblank{string}{nottrue}{notfalse}

#arithmetic tests
\ifnumcomp{integer expression}{relation}{integer expression}{true}{false}
\ifnumequal{integer expression}{integer expression}{true}{false}
\ifnumgreater{integer expression}{integer expression}{true}{false}
\ifnumless{integer expression}{integer expression}{true}{false}
\ifnumodd{integer expression}{true}{false}
\ifdimcomp{dimen expression}{relation}{dimen expression}{true}{false}
\ifdimequal{dimen expression}{dimen expression}{true}{false}
\ifdimgreater{dimen expression}{dimen expression}{true}{false}
\ifdimless{dimen expression}{dimen expression}{true}{false}
\ifboolexpr{ expression}{true}{false}
\ifboolexpe{ expression}{true}{false}
\whileboolexpr{ expression}{code}
\unlessboolexpr{ expression}{code}

#list processing
\DeclareListParser{command}{separator}
\DeclareListParser*{command}{separator}
\docsvlist{item,item,...}
\forcsvlist{handler}{item,item,...}
\listadd{listmacro}{item}
\listgadd{listmacro}{item}
\listeadd{listmacro}{item}
\listxadd{listmacro}{item}
\listcsadd{listmacro}{item}
\listcsgadd{listmacro}{item}
\listcseadd{listmacro}{item}
\listcsxadd{listmacro}{item}
\dolistloop{listmacro}
\dolistcsloop{listcsname}
\forlistloop{handler}{listmacro}
\forlistcsloop{handler}{listcsname}
\ifinlist{item}{listmacro}{true}{false}
\xifinlist{item}{listmacro}{true}{false}
\ifinlistcs{item}{listcsname}{true}{false}
\xifinlistcs{item}{listcsname}{true}{false}

#misc
\rmntonum{numeral}
\ifrmnum{string}{true}{false}
