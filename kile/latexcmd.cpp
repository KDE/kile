/***************************************************************************
                         latexcmd.cpp
                         ------------
    begin                : Jun 26 2005
    version              : 0.10
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
#include "latexcmd.h"

#include <klocale.h>
#include <kdebug.h>

namespace KileDocument
{

// BEGIN LatexCommands

LatexCommands::LatexCommands(KConfig *config, KileInfo *info) : m_config(config), m_ki(info)
{
	m_envGroupName = "Latex Environments";
	m_cmdGroupName = "Latex Commands";
	
	LatexCommands::resetCommands();
}

void LatexCommands::resetCommands()
{
	// description of the fields for environments
	//  0: standard entry (+,-)
	//  1: environmenty type (a,m,l,t,v)
	//  2: including starred version (*)
	//  3: eol character (\\\\)
	//  4: need mathmode ($) or displaymathmode ($$)
	//  5: standard tabulator (tabulator string, f.e. &=& or &=)
	//  6: optional parameter
	//  7: parameter group(s) 
	
	QStringList envlist;
	envlist
	   // list environments
	   << "itemize,+,l,*,,,,,"  
	   << "enumerate,+,l,*,,,,,"  
	   << "description,+,l,*,,,,,"
	   << "Bitemize,+,l,,,,,,"   
	   << "Benumerate,+,l,,,,,,"  
	   << "Bdescription,+,l,,,,,,"
	   << "labeling,+,l,,,,,[ ],{ }"
	   // tabular environments
	   << "array,+,t,,\\\\,$,&,[tcb],"
	   << "tabular,+,t,*,\\\\,,&,[tcb],"  
	   << "tabularx,+,t,,\\\\,,&,,{w}"
	   << "tabbing,+,t,,\\\\,,\\>,," 
	   << "longtable,+,t,,\\\\,,&,[tcb],"  
	   << "ltxtable,+,t,,\\\\,,&,[tcb],{w}"
	   << "supertabular,+,t,*,\\\\,,&,,"  
	   << "mpsupertabular,+,t,*,\\\\,,&,,"  
	   << "xtabular,+,t,*,\\\\,,&,," 
	   << "mpxtabular,+,t,*,\\\\,,&,,"
	   // math environments
	   << "displaymath,+,m,,,,,,"
	   << "equation,+,m,*,,,,," 
	   << "eqnarray,+,m,*,\\\\,,&=&,,"
	   << "matrix,+,m,,\\\\,$,&,,"         
	   << "pmatrix,+,m,,\\\\,$,&,,"        
	   << "bmatrix,+,m,,\\\\,$,&,,"       
	   << "Bmatrix,+,m,,\\\\,$,&,,"       
	   << "vmatrix,+,m,,\\\\,$,&,,"        
	   << "Vmatrix,+,m,,\\\\,$,&,,"        
	   // amsmath environments
	   << "multline,+,a,*,\\\\,,,," 
	   << "gather,+,a,*,\\\\,,,,"
	   << "split,+,a,,\\\\,$$,,,"          // needs surrounding environment
	   << "align,+,a,*,\\\\,,&=,,"           
	   << "flalign,+,a,*,\\\\,,&=,,"         
	   << "alignat,+,a,*,\\\\,,&=,,{n}"    
	   << "aligned,+,a,,\\\\,$,&=,[tcb],"  
	   << "gathered,+,a,,\\\\,$,,[tcb],"   
	   << "alignedat,+,a,,\\\\,$,&=,[tcb],{n}" 
	   //<< "xalignat,+,a,*,\\\\,,&=,,{n}"   // obsolet
	   //<< "xxalignat,+,a,*,\\\\,,&=,,{n}"  // obsolet
	   << "cases,+,a,,\\\\,$,&,,"           
	   // verbatim environments
	   << "verbatim,+,v,*,,,,," 
	   << "boxedverbatim,+,v,,,,,,"
	   << "Bverbatim,+,v,,,,,[ ],"
	   << "Lverbatim,+,v,,,,,[ ],"
	   << "lstlisting,+,v,,,,,[ ],"
	   ;

	// description of the fields for commands
	//  0: standard entry (+,-)
	//  1: environmenty type (a,m,l,t,v)
	//  2: including starred version (*)
	
	QStringList cmdlist;
	cmdlist
		// Labels
	   << "\\label,+,L,"
		// References
	   << "\\ref,+,R,"
	   << "\\pageref,+,R,"
	   << "\\vref,+,R,"
	   << "\\vpageref,+,R,"
		// Citations
	   << "\\cite,+,C,"
	   ;
	
	// first clear the dictionary
	m_latexCommands.clear();
	
	// insert environments
	addUserCommands(m_envGroupName,envlist);
	insert(envlist);
	
	// insert commands
	addUserCommands(m_cmdGroupName,cmdlist);
	insert(cmdlist);
}


// add user defined environments/commands 

void LatexCommands::addUserCommands(const QString &name, QStringList &list)
{
	if ( m_config->hasGroup(name) ) 
	{
		QMap<QString,QString> map = m_config->entryMap(name);
		if ( ! map.empty() ) 
		{
			QMapConstIterator<QString,QString> it;
			for ( it=map.begin(); it!=map.end(); ++it) 
			{
				list << it.key() + ",-," + it.data();
				kdDebug() << "\tLatexCommands add user command: " <<  it.key() + " --> " + it.data() << endl;
			}
		}
	}
}

// insert all entries into the dictionary

void LatexCommands::insert(const QStringList &list)
{
	// now insert new entries, if they have the right number of attributes
	QStringList::ConstIterator it;
	for ( it=list.begin(); it!=list.end(); ++it ) 
	{
		int pos = (*it).find(',');
		if ( pos >= 0 ) 
		{
			QString key = (*it).left(pos);
			QString value = (*it).right( (*it).length()-pos-1 ); 
			QStringList valuelist = QStringList::split(',',value,true);
			uint attributes = ( key.at(0)=='\\' ) ? MaxCmdAttr : MaxEnvAttr;
			if ( valuelist.count() == attributes ) 
				m_latexCommands[key] = value;
			else
			   kdDebug() << "\tLatexCommands error: wrong number of attributes (" << key << " ---> " << value << ")" << endl;
		} 
		else 
		{
			kdDebug() << "\tLatexCommands error: no separator found (" << (*it) << ")"  << endl;
		}
	}
}

//////////////////// get value from dictionary  ////////////////////

// Get value of a key. A star at the end is stripped.

QString LatexCommands::getValue(const QString &name)
{
	QString key = ( name.find('*',-1) >= 0 ) ? name.right(name.length()-1) : name;
	return ( m_latexCommands.contains(key) ) ? m_latexCommands[key] : QString::null;
}

//////////////////// internal functions  ////////////////////

// get parameter at index

QString LatexCommands::getAttrAt(const QString &name, uint index)
{
	uint attributes = ( name.at(0)=='\\' ) ? MaxCmdAttr : MaxEnvAttr;
	QStringList list = QStringList::split(',',getValue(name),true);
	return ( index<attributes && list.count()==attributes ) ? list[index] : QString::null;
}

// check for a standard environment 

bool LatexCommands::isUserDefined(const QString &name)
{
	return ( getValue(name).at(0) == '-' );          
}

// check for a special environment type

bool LatexCommands::isType(const QString &name, QChar ch)
{
	if ( name.find('*',-1) >= 0 )
	{
		QString envname = name.left( name.length()-1 );
		return ( getValue(envname).at(2)==ch && isStarredEnv(envname) );
	}
	else
	{
		return ( getValue(name).at(2) == ch );          
	}
}

//////////////////// attributes and characters ////////////////////

// convert attribute to character

QChar LatexCommands::getAttrChar(CmdAttribute attr)
{
	QChar ch;
	switch ( attr ) 
	{
		case CmdAttrAmsmath:   ch = 'a'; break;
		case CmdAttrMath:      ch = 'm'; break;
		case CmdAttrList:      ch = 'l'; break;
		case CmdAttrVerbatim:  ch = 'v'; break;
		case CmdAttrTabular:   ch = 't'; break;
		case CmdAttrLabel:     ch = 'L'; break;
		case CmdAttrReference: ch = 'R'; break;
		case CmdAttrCitations: ch = 'C'; break;
		default:
		     kdDebug() << "\tLatexCommands error: unknown type of env/cmd: code " << attr << endl;
			  return '?';
	}	
	
	return ch;
}

// convert character to attribute  

CmdAttribute LatexCommands::getCharAttr(QChar ch)
{
	CmdAttribute attr;
	switch ( ch ) 
	{
		case 'a': attr = CmdAttrAmsmath;   break;
		case 'm': attr = CmdAttrMath;      break;
		case 'l': attr = CmdAttrList;      break;
		case 'v': attr = CmdAttrVerbatim;  break;
		case 't': attr = CmdAttrTabular;   break;
		case 'L': attr = CmdAttrLabel;     break;
		case 'R': attr = CmdAttrReference; break;
		case 'C': attr = CmdAttrCitations; break;
		default:
		     kdDebug() << "\tLatexCommands error: unknown type of env/cmd: " << ch << endl;
			  return CmdAttrNone;
	}	
	
	return attr;
}	

//////////////////// public attributes  ////////////////////

// check for environment types

bool LatexCommands::isMathEnv(const QString &name)
{
	QChar ch = getValue(name).at(2);
	return ( ch=='m' || ch=='a' );
}

// check for some special attributes

bool LatexCommands::isStarredEnv(const QString &name)
{
	return ( getAttrAt(name,2) == "*" );
}

bool LatexCommands::isCrEnv(const QString &name)
{
	return ( getAttrAt(name,3) == "\\\\" );
}

bool LatexCommands::isMathModeEnv(const QString &name)
{
	return ( getAttrAt(name,4) == "$" );
}

bool LatexCommands::isDisplaymathModeEnv(const QString &name)
{
	return ( getAttrAt(name,4) == "$$" );
}

QString LatexCommands::getTabulator(const QString &name)
{
	QString tab = getAttrAt(name,5);
	return ( tab.find('&') >= 0 ) ? tab : QString::null;
}

//////////////////// environments and commands ////////////////////

// get a list of environments and commands. The search can be restricted
// to given attributes and userdefined environments and commands

void LatexCommands::commandList(QStringList &list, uint attr, bool userdefined)
{
	list.clear();
	
	QMapConstIterator<QString,QString> it;
	for ( it=m_latexCommands.begin(); it!=m_latexCommands.end(); ++it) 
	{
		// first check, if we need really need all environments and commands
		// or if a restriction to some attributes is given
		if ( attr != (uint)CmdAttrNone ) 
		{
			if ( ! ( attr & (uint)getCharAttr( it.data().at(2) ) ) )            
				continue;
		}
		
		// second check, if we need only user defined environments or commands
		if ( ! userdefined )
			list.append( it.key() );
		else if ( it.data().at(0) == '-' )
			list.append( it.key() );
	}
}

// get all attributes for a given environment and command

bool LatexCommands::commandAttributes(const QString &name, LatexCmdAttributes &attr)
{
	uint attributes = ( name.at(0)=='\\' ) ? MaxCmdAttr : MaxEnvAttr;
	
	// split attribute list
	QStringList list = QStringList::split(',',getValue(name),true);
	
	// check number of attributes
	if (  list.count() != attributes ) 
		return false;
		
	// check for a standard environment/command
	attr.standard = ( list[0] == "+" );
	
	// most important: type of environment or command
	attr.type = getCharAttr( list[1].at(0) );
	if ( attr.type == CmdAttrNone )
		return false;
		
	// all environments/commands have starred attribute 
	attr.starred = ( list[2] == "*" ) ;
	
	// next attributes are only valid for environments
	if ( attributes == MaxEnvAttr ) 
	{
		attr.cr = ( list[3] == "\\\\" ) ;
		attr.mathmode = ( list[4] == "$" ) ;
		attr.displaymathmode = ( list[4] == "$$" ) ;
		attr.tabulator = list[5];
		attr.option = list[6];
		attr.parameter = list[7];
	}
	
	return true;
}

//////////////////// determine config string ////////////////////

QString LatexCommands::configString(LatexCmdAttributes &attr,bool env)
{
	// most important: type of environment or command
	QChar ch = getAttrChar( attr.type );
	if ( ch == '?' )
		return QString::null;
	QString s = QString("%1,").arg(ch);
	
	// all environments/commands have starred attribute 
	if ( attr.starred )
		s += "*,";
	else
		s += ",";
	
	// next attributes are only valid for environments
	if ( env ) {
		if ( attr.cr )
			s += "\\\\,";
		else
			s += ",";
		if ( attr.mathmode )
			s += "$,";
		else if ( attr.displaymathmode )
			s += "$$";
		else
			s += ",";
		s += attr.tabulator + ",";
		s += attr.option + ",";
		s += attr.parameter + ",";
	}
	
	return s.left(s.length()-1);
}
 
// END LatexCommands

}
#include "latexcmd.moc"
