/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include "irc.h"
#include "config.h"
#include "fsettings.h"
#include "ircdefs.h"

QHash<QString, IrcServer *> IrcLayer::m_servers;

IrcLayer::IrcLayer(QObject * parent, QString ircUri) : QObject(parent)
{
	qDebug() << "Creating new IRC layer" << ircUri;
	fSettings settings;
	m_active=0;
	// Protocol regexes
	initRegexes();
	m_encoding = "UTF-8";
	m_ircServer=0;
	m_joined=0;
	m_codec = QTextCodec::codecForName(m_encoding);
	ircUseUri(ircUri);
	connect(this, SIGNAL(gotKick(QHash<QString, QString>)), this, SLOT(checkKicked(QHash<QString, QString>)));
	connect(this, SIGNAL(gotNames(QStringList)), this, SLOT(addNames(QStringList)));

	connect(this, SIGNAL(gotPart(QHash<QString, QString>)), this, SIGNAL(userListChanged()));
	connect(this, SIGNAL(gotJoin(QHash<QString, QString>)), this, SIGNAL(userListChanged()));
	connect(this, SIGNAL(gotQuit(QHash<QString, QString>)), this, SIGNAL(userListChanged()));
	connect(this, SIGNAL(gotNick(QHash<QString, QString>)), this, SIGNAL(userListChanged()));
	connect(this, SIGNAL(gotKick(QHash<QString, QString>)), this, SIGNAL(userListChanged()));
//	connect(m_socket, SIGNAL(error()), this, SLOT(gotError()));
}

void IrcLayer::ircConnect()
{
	m_active=true;
	if (!m_ircServer->contact())
	{
		//qDebug("We are connected! Joining...");
		if(targetMode()==ChannelMode)
		{
			//qDebug() << "Joining " << m_target << __LINE__;
			ircJoin(target());
		}
	}
	else
	{
		//qDebug("No we aren't");
		infMsg(tr("Contacting IRC server..."));
	}
	connect(m_ircServer, SIGNAL(connected()), this, SLOT(ircLogon()));
	connect(m_ircServer, SIGNAL(gotLine(QByteArray)), this, SLOT(ircParse(QByteArray)));
	connect(m_ircServer, SIGNAL(disconnected()), this, SLOT(gotDisconnected()));
}

void IrcLayer::contactServer()
{
	if(m_ircServer) m_ircServer->decRefCount();
	m_ircServer = getServer(m_server, m_port);
	connect(m_ircServer, SIGNAL(errMsg(QString)), this, SLOT(errMsg(QString)));
	connect(m_ircServer, SIGNAL(infMsg(QString)), this, SLOT(infMsg(QString)));
}

IrcLayer::~IrcLayer()
{
	qDebug() << "IrcLayer" << getIrcUri() << "is being deleted";
	m_ircServer->decRefCount();
	if(m_targetMode==ChannelMode && joined())
		ircPart(target());
	delete chanPrefix;
	delete mircColors;
	delete mircShit;
	delete genError;
}

void IrcLayer::initRegexes()
{
	// These should be used only on "response"/"command" lines
	prRegexes["privmsg"] = QRegExp("^PRIVMSG (\\S+) :(.+)$");
	prRegexes["topic"] = QRegExp("^TOPIC (\\S+) :(.+)$");
	prRegexes["notice"] = QRegExp("^NOTICE (\\S+) :(.+)$");
	prRegexes["names"] = QRegExp("^(?:=|@) (\\S+) :(?:(.+)\\s?)+$");
	prRegexes["rpltopic"] = QRegExp("^(\\S+) :(.+)$");
	prRegexes["part"] = QRegExp("^PART (\\S+) :(.+)?$");
	prRegexes["join"] = QRegExp("^JOIN :(\\S+)$");
	prRegexes["quit"] = QRegExp("^QUIT :(.+)?$");
	prRegexes["nick"]=QRegExp("^NICK :(.+)$");
	prRegexes["kick"] = QRegExp("^KICK (\\S+) (\\S+) :(.+)?$");
	prRegexes["mode"] = QRegExp("^MODE (\\S+) (\\S+)(?: (\\S+)?(?: (\\S+)?)?)?$");
	// These two may need tuning
	prRegexes["resp"] = QRegExp("^:\\S+ ([0-9]+) (\\S+) (.+)$");
	prRegexes["cmd"] = QRegExp("^:(\\S+)!(\\S+)@(\\S+) (.+)$");
	//"[(\x03([0-9]*(,[0-9]+)?)?)\x37\x02\x26\x22\x21]"
	// etc
	prRegexes["ircUri"] = QRegExp("^irc://([a-zA-Z0-9\\.\\-]+)/(\\S+)$");
	prRegexes["ircUriPort"] = QRegExp("^irc://([a-zA-Z0-9\\.\\-]+):([0-9]+)/(\\S+)$");
	prRegexes["lineBr"] = QRegExp("[\r\n]");
	prRegexes["uPrefix"] = QRegExp("^[@+&].*"); // Possible username prefixes
	prRegexes["action"]=QRegExp("^\x01" "ACTION (.*)" "\x01$");
	mircColors = new QRegExp("\x03(\\,([0-9][0-5]?|)|([0-9][0-5]?)(\\,([0-9][0-5]?|)|)|\\,|)");
	mircShit = new QRegExp("[]");
	chanPrefix = new QRegExp("^[#&\\+]\\S*$");
	genError= new QRegExp("^(\\S+ )?:(.+)$");

}

QString IrcLayer::getIrcUri()
{
	if(!active()) return QString();
	QString pick;
	pick="irc://"+m_server;
	if(m_port!="6667") pick +=":"+m_port;
	pick+="/"+m_target;
	return pick;
}

void IrcLayer::infMsg(QString message)
{
	emit gotInfo(message);
}

void IrcLayer::errMsg(QString message)
{
	emit gotError(message);
}

void IrcLayer::ircThrow(QString what)
{
	//TODO make some kind of auto message splitting by line break
	m_ircServer->ircThrow(m_codec->fromUnicode(what.remove(prRegexes["lineBr"])));
	//qDebug() << "RAW THROW: " << what;
}

void IrcLayer::ircMsg(QString what, QString where)
{
	ircThrow("PRIVMSG "+where+" :"+what);
}

void IrcLayer::ircNotice(QString what, QString where)
{
	ircThrow("NOTICE "+where+" :"+what);
}

void IrcLayer::ircSetNick(QString nick)
{
	m_ircServer->setNick(m_codec->fromUnicode(nick));
}

void IrcLayer::ircLogon()
{
	if(m_targetMode==ChannelMode)
	{
		//qDebug() << " Joining " << m_target << __LINE__;
		ircJoin(target());
	}
}

void IrcLayer::ircMode(QString modes)
{
	ircThrow("MODE "+m_target+" "+modes);
}

void IrcLayer::ircJoin(QString channel)
{
//	if(m_connected) ircPart(m_target,"...");
	ircThrow("JOIN "+channel);
}

void IrcLayer::ircPart(QString channel, QString message)
{
	ircThrow("PART "+channel+" :"+message);
}

void IrcLayer::ircQuit(QString message)
{
	ircThrow("QUIT :"+message);
	if (connected())
		m_ircServer->breakContact();
}

void IrcLayer::ircKick(QString whom, QString where, QString reason)
{
	ircThrow(QString("KICK %1 %2 :%3").arg(where,whom,reason));
}

void IrcLayer::ircParse(QByteArray data)
{
	QString line=m_codec->toUnicode(data);
	line.remove(*mircColors);
	line.remove(*mircShit);
	if(prRegexes["cmd"].exactMatch(line))
	{
		QHash<QString, QString> data;
		data["nick"]=prRegexes["cmd"].cap(1); // nick
		data["ident"]=prRegexes["cmd"].cap(2); // ident
		data["host"]=prRegexes["cmd"].cap(3); // host
		parseCmd(prRegexes["cmd"].cap(4),data);
	} else
	if(prRegexes["resp"].exactMatch(line))
	{
		QHash<QString, QString> data;
		data["target"]=prRegexes["resp"].cap(2);
		parseResp(prRegexes["resp"].cap(1).toInt(), prRegexes["resp"].cap(3),data);
	}
}


void IrcLayer::parseResp(int code, QString args, QHash<QString, QString> data)
{
	Q_UNUSED (data);
	QStringList argz;
	QString user;
	if((code>=ERR_NOSUCHNICK)&&(code<=ERR_USERSDONTMATCH)&&(genError->exactMatch(args)))
	{
		argz=genError->capturedTexts();
	}
	//qDebug() << "parsing repl" << code;
	switch(code)
	{
		case RPL_NAMREPLY:
	 	if(prRegexes["names"].exactMatch(args))
		{
			//qDebug() << "It's NAMES" << prRegexes["names"].capturedTexts();
			if(!QString::compare(prRegexes["names"].cap(1), target(), Qt::CaseInsensitive))
			{
				QStringList users = prRegexes["names"].cap(2).split(" ");
				//qDebug() << "users" << users;
				QStringList::iterator it;
				for(it=users.begin(); it!=users.end(); ++it)
				{
					if(prRegexes["uPrefix"].exactMatch(*it))
					{
						(*it).remove(0,1);
					}
				}
				addNames(users);
			}
		}
		break;
		case RPL_TOPIC:
		if(prRegexes["rpltopic"].exactMatch(args))
		{
			if(!QString::compare(prRegexes["rpltopic"].cap(1), target(), Qt::CaseInsensitive))
			{
				m_topic=prRegexes["rpltopic"].cap(2);
				emit gotTopic(prRegexes["rpltopic"].capturedTexts()); // Channel,topic
			}
		}
		break;
		case RPL_ENDOFNAMES:
			argz = args.split(" ");
			if(!QString::compare(argz[0], target(), Qt::CaseInsensitive))
			{
				ircSaveNames();
				emit gotNames(users());
			}
		break;
	// TODO: Sort this heap out (ones that does not fit into generic error)
	case ERR_NICKNAMEINUSE:
	case ERR_NOSUCHSERVER:
	case ERR_NOSUCHCHANNEL:
	case ERR_CANNOTSENDTOCHAN:
	case ERR_TOOMANYCHANNELS:
	case ERR_WASNOSUCHNICK:
	case ERR_TOOMANYTARGETS:
	case ERR_NOORIGIN:
	case ERR_NORECIPIENT:
	case ERR_NOTEXTTOSEND:
	case ERR_NOTOPLEVEL:
	case ERR_WILDTOPLEVEL:
	case ERR_UNKNOWNCOMMAND:
	case ERR_NOMOTD:
	case ERR_NOADMININFO:
	case ERR_FILEERROR:
	case ERR_NONICKNAMEGIVEN:
	case ERR_ERRONEUSNICKNAME:
	case ERR_NICKCOLLISION:
	case ERR_USERNOTINCHANNEL:
	case ERR_NOTONCHANNEL:
	case ERR_USERONCHANNEL:
	case ERR_NOLOGIN:
	case ERR_SUMMONDISABLED:
	case ERR_USERSDISABLED:
	case ERR_NOTREGISTERED:
	case ERR_NEEDMOREPARAMS:
	case ERR_ALREADYREGISTRED:
	case ERR_NOPERMFORHOST:
	case ERR_PASSWDMISMATCH:
	// That's my favourite
	case ERR_YOUREBANNEDCREEP:
	case ERR_KEYSET:
	case ERR_CHANNELISFULL:
	case ERR_UNKNOWNMODE:
	case ERR_INVITEONLYCHAN:
	case ERR_BANNEDFROMCHAN:
	case ERR_BADCHANNELKEY:
	case ERR_NOPRIVILEGES:
	case ERR_CHANOPRIVSNEEDED:
	case ERR_CANTKILLSERVER:
	case ERR_NOOPERHOST:
	case ERR_UMODEUNKNOWNFLAG:
	case ERR_USERSDONTMATCH:
	// Generic error.
	errMsg(args);

	break;
	case RPL_NONE:
	case RPL_USERHOST:
	case RPL_ISON:
	case RPL_AWAY:
	case RPL_UNAWAY:
	case RPL_NOWAWAY:
	case RPL_WHOISUSER:
	case RPL_WHOISSERVER:
	case RPL_WHOISOPERATOR:
	case RPL_WHOISIDLE:
	case RPL_WHOISCHANNELS:
	case RPL_WHOWASUSER:
	case RPL_LISTSTART:
	case RPL_LIST:
	case RPL_CHANNELMODEIS:
	case RPL_NOTOPIC:
	case RPL_INVITING:
	case RPL_SUMMONING:
	case RPL_VERSION:
	case RPL_WHOREPLY:
	case RPL_LINKS:
	case RPL_ENDOFLINKS:
	case RPL_BANLIST:
	case RPL_INFO:
	case RPL_YOUREOPER:
	case RPL_REHASHING:
	case RPL_TIME:
	case RPL_USERSSTART:
	case RPL_USERS:
	case RPL_ENDOFUSERS:
	case RPL_NOUSERS:
	case RPL_TRACELINK:
	case RPL_TRACECONNECTING:
	case RPL_TRACEHANDSHAKE:
	case RPL_TRACEUNKNOWN:
	case RPL_TRACEOPERATOR:
	case RPL_TRACEUSER:
	case RPL_TRACESERVER:
	case RPL_TRACENEWTYPE:
	case RPL_TRACELOG:
	case RPL_STATSLINKINFO:
	case RPL_STATSCOMMANDS:
	case RPL_UMODEIS:
	case RPL_ADMINME:
	case RPL_ADMINLOC:
	case RPL_ADMINEMAIL:
	// Generic replies
	infMsg(args/*+" #"+QString::number(code)*/);
	break;

	case RPL_STATSCLINE:
	case RPL_STATSNLINE:
	case RPL_STATSILINE:
	case RPL_STATSKLINE:
	case RPL_STATSYLINE:
	case RPL_ENDOFSTATS:
	case RPL_STATSLLINE:
	case RPL_STATSUPTIME:
	case RPL_STATSOLINE:
	case RPL_STATSHLINE:
	case RPL_LUSERCLIENT:
	case RPL_LUSEROP:
	case RPL_LUSERUNKNOWN:
	case RPL_LUSERCHANNELS:
	case RPL_LUSERME:
	case RPL_ENDOFWHOIS:
	case RPL_ENDOFWHOWAS:
	case RPL_LISTEND:
	case RPL_ENDOFWHO:
	case RPL_ENDOFBANLIST:
	case RPL_MOTDSTART:
	case RPL_MOTD:
	case RPL_ENDOFINFO:
	case RPL_ENDOFMOTD:
	// Trash
	// Silently discarded
	break;

/*	default:
	// What's that?
	infMsg(tr("Unhandled: ")+args);
*/	}
}

void IrcLayer::parseCmd(QString cmd, QHash<QString, QString> data)
{
//	qDebug() << "Command=" << cmd << ", cmd=" << cmd;
	if(prRegexes["privmsg"].exactMatch(cmd))
	{
		bool isAction=false;
		data["target"]=prRegexes["privmsg"].cap(1); // target
		data["text"]=prRegexes["privmsg"].cap(2); // text
		if(prRegexes["action"].exactMatch(data["text"]))
		{
			data["text"]=prRegexes["action"].cap(1);
			isAction = true;
		}
		if((!QString::compare(data["target"], target(), Qt::CaseInsensitive) && targetMode()==ChannelMode) || (!QString::compare(data["target"], nick(), Qt::CaseInsensitive) && targetMode()==PrivateMode))
		{
			isAction ? emit gotAction(data) : emit gotChannelMsg(data);
		}
		else	emit gotPrivAction(data);
	} else
	if(prRegexes["notice"].exactMatch(cmd))
	{
		data["target"]=prRegexes["notice"].cap(1); // target
		data["text"]=prRegexes["notice"].cap(2); // text
		if((!QString::compare(data["target"], nick(), Qt::CaseInsensitive)) || (!QString::compare(data["target"], target(), Qt::CaseInsensitive)))
			emit gotNotice(data);
	} else
	if(prRegexes["part"].exactMatch(cmd))
	{
		data["target"]=prRegexes["part"].cap(1); // target
		data["text"]=prRegexes["part"].cap(2); // text
		if((!QString::compare(data["target"], target(), Qt::CaseInsensitive))&&(!QString::compare(data["nick"], nick(), Qt::CaseInsensitive)))
			setJoined(0);
		if(!QString::compare(data["target"], target(), Qt::CaseInsensitive))
		{
			if(int i=m_users.indexOf(data["nick"]) != -1) m_users.removeAt(i);
			emit gotPart(data);
		}
	} else
	if(prRegexes["topic"].exactMatch(cmd))
	{
		if(!QString::compare(prRegexes["topic"].cap(1), target(), Qt::CaseInsensitive))
		{
			data["target"]=prRegexes["topic"].cap(1);
			data["text"]=prRegexes["topic"].cap(2);
			m_topic=data["text"];
			emit gotTopic(data);
		}
	}
	else
	if(prRegexes["quit"].exactMatch(cmd))
	{
		data["text"]=prRegexes["quit"].cap(1); // text
		if (m_users.contains(data["nick"]) || !QString::compare(data["nick"], nick(), Qt::CaseInsensitive))
		{
			if(int i=m_users.indexOf(data["nick"]) != -1) m_users.removeAt(i);
			emit gotQuit(data);
		}
	} else
	if(prRegexes["join"].exactMatch(cmd))
	{
		data["target"]=prRegexes["join"].cap(1); // target
		// if it's about me
		if((!QString::compare(data["nick"], nick(), Qt::CaseInsensitive)) && (!QString::compare(data["target"],target(),Qt::CaseInsensitive)) && !joined())
		{
			setTarget(data["target"]);
			setJoined(1);
		}
		if(!QString::compare(data["target"], target(), Qt::CaseInsensitive))
		{
			m_users << data["nick"];
			emit gotJoin(data);
		}
	} else
	if(prRegexes["kick"].exactMatch(cmd))
	{
		data["target"]=prRegexes["kick"].cap(1); // target
		data["subject"]=prRegexes["kick"].cap(2); // whom
		data["text"]=prRegexes["kick"].cap(3); // reason
		if(!QString::compare(data["target"], target(), Qt::CaseInsensitive))
		{
			if(int i=m_users.indexOf(data["nick"]) != -1) m_users.removeAt(i);
			emit gotKick(data);
		}
	} else
	if(prRegexes["mode"].exactMatch(cmd))
	{
		data["target"]=prRegexes["mode"].cap(1); // target
		data["text"]=prRegexes["mode"].cap(2); // modecmd
		data["subject"]=prRegexes["mode"].cap(3); // subject
		if(!QString::compare(data["target"], target(), Qt::CaseInsensitive))
			emit gotMode(data);
	} else
	if(prRegexes["nick"].exactMatch(cmd))
	{
		data["target"]=prRegexes["nick"].cap(1); // target
		qDebug() << data;
		if(m_users.contains(data["nick"]) || !QString::compare(data["target"],nick(),Qt::CaseInsensitive))
		{
			if(int i=m_users.indexOf(data["nick"]) != -1) m_users.removeAt(i);
			m_users << data["target"];
			emit gotNick(data);
		}
	}
}

QString IrcLayer::composeIrcUri(QHash<QString, QString> data)
{
	if(!(data.contains("server") && data.contains("target")))
		return QString();
	if(data.contains("port") && data["port"]!="6667")
		return QString("irc://%1:%2/%3").arg(data["server"],data["port"],data["target"]);
	else
		return QString("irc://%1/%2").arg(data["server"],data["target"]);
}

QString IrcLayer::ircUseUri(QString uri)
{
	QHash<QString, QString> uriData = chewIrcUri(uri);
	if(uriData.isEmpty()) return uri;
	m_target=uriData["target"];
	m_server=uriData["server"];
	m_port=uriData["port"];
	setJoined(0);
	chanPrefix->exactMatch(uriData["target"]) ? m_targetMode=ChannelMode : m_targetMode=PrivateMode;
	if((m_server!=uriData["server"])||(m_port!=uriData["port"])||(!m_ircServer))
	{
		// some real action!
		contactServer();
	}
	else
	{
		if (m_targetMode==ChannelMode)
		{
			//qDebug() << "Joining " << m_target << __LINE__;
			ircJoin(m_target);
		}
	}
	return composeIrcUri(uriData);
}

void IrcLayer::ircNs(QString what)
{
	ircThrow("NICKSERV "+what);
}

void IrcLayer::ircCs(QString what)
{
	ircThrow("CHANSERV "+what);
}

void IrcLayer::ircMs(QString what)
{
	ircThrow("MEMOSERV "+what);
}

void IrcLayer::gotDisconnected()
{
	errMsg(tr("Disconnected from server."));
	m_active=false;
}

QString IrcLayer::nick()
{
	return m_codec->toUnicode(m_ircServer->nick().toAscii());
}
QString IrcLayer::ident()
{
	return m_ident;
}
QString IrcLayer::realname()
{
	return m_realname;
}
QString IrcLayer::channel()
{
	return m_target;
}
QString IrcLayer::server()
{
	return m_server;
}
QByteArray IrcLayer::encoding()
{
	return m_encoding;
}
QString IrcLayer::port()
{
	return m_port;
}

int IrcLayer::setEncoding(QString theValue)
{
	if(QTextCodec::availableCodecs().contains(theValue.toAscii()))
	{
		m_codec=QTextCodec::codecForName(theValue.toAscii());
		infMsg(tr("Encoding has been set to %1").arg(theValue));
		return 1;
	} else
	{
		errMsg(tr("No such encoding!"));
		return 0;
	}
}

QHash<QString, QString> IrcLayer::chewIrcUri(QString uri)
{
	QHash<QString, QString> ret;
	if(prRegexes["ircUriPort"].exactMatch(uri))
	{
		ret["server"]=prRegexes["ircUriPort"].cap(1);
		ret["port"]=prRegexes["ircUriPort"].cap(2);
		ret["target"]=prRegexes["ircUriPort"].cap(3);
	}
	else if(prRegexes["ircUri"].exactMatch(uri))
	{
		ret["port"]="6667";
		ret["server"]=prRegexes["ircUri"].cap(1);
		ret["target"]=prRegexes["ircUri"].cap(2);
	} else errMsg(tr("Invalid IRC URI"));
	return ret;
}

void IrcLayer::checkKicked(QHash<QString, QString> data)
{
	if((!QString::compare(data["subject"],nick(),Qt::CaseInsensitive))&&(!QString::compare(data["target"],target(),Qt::CaseInsensitive)))
		setJoined(0); // Alas.
}

QByteArray IrcLayer::encoding() const
{
	return m_codec->name();
}

IrcServer * IrcLayer::getServer(QString host, QString port)
{
	QString nameport=QString("%1:%2").arg(host,port);
	if(!IrcLayer::m_servers.contains(nameport))
	{
		IrcLayer::m_servers[nameport]=new IrcServer(host,port);
	}
	else IrcLayer::m_servers[nameport]->incRefCount();
	return IrcLayer::m_servers[nameport];
}

int IrcLayer::connected()
{
	return m_ircServer->isConnected();
}

bool IrcLayer::isIrcUri(QString uri)
{
	return QRegExp("^irc://[a-zA-Z0-9\\.\\-]+(?::[0-9]+)?/\\S+$").exactMatch(uri);
}


bool IrcLayer::nickChanged() const
{
	return m_ircServer->nickSet();
}

void IrcLayer::setNickChanged(bool theValue)
{
	m_ircServer->setNickSet(theValue);
}

int IrcLayer::joined() const
{
	return m_joined;
}

void IrcLayer::setJoined(int theValue)
{
	m_joined = theValue;
}

QString IrcLayer::target() const
{
	return m_target;
}

void IrcLayer::setTarget(const QString& theValue)
{
	m_target = theValue;
}

int IrcLayer::targetMode() const
{
	return m_targetMode;
}


void IrcLayer::setTargetMode(int theValue)
{
	m_targetMode = theValue;
}

void IrcLayer::say(QString msg)
{
	ircMsg(msg, target());
}

void IrcLayer::addNames(QStringList names)
{
	m_usersTemp << names;
}

void IrcLayer::ircSaveNames()
{
	m_users = m_usersTemp;
	m_usersTemp = QStringList();
}

QStringList IrcLayer::users() const
{
	return m_users;
}


QString IrcLayer::cleanUri(QString uri)
{
	return uri.remove(":6667");
}

bool IrcLayer::active() const
{
	return m_active;
}

void IrcLayer::finalizeServers ()
{
	qDeleteAll (m_servers);
}

QString IrcLayer::topic() const
{
	return m_topic;
}

void IrcLayer::setTopic(const QString& theValue)
{
	if(connected() && targetMode()==ChannelMode)
		ircThrow(QString("TOPIC %1 :%2").arg(target(), theValue));
}

/*
101
111
these are secret self-destruction preventing codes, do not remove
*/
