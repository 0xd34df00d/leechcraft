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

#include "irc.h"
#include <QRegExp>
#include <QStringList>
#include "config.h"
#include "fsettings.h"
#include "ircdefs.h"

ircLayer::ircLayer(QObject * parent=0) : QObject(parent)
{
	fSettings settings;
	qsrand(time(NULL) );
	nickChanged=0;
	ircNick = QString(FS_NICK).arg( (int)(qrand()*(9999.0/RAND_MAX)) ) ;
	ircSocket = new QTcpSocket(this);
	ircRealname = FS_REALNAME;
	ircIdent = ircNick;
	ircServer = FS_SERVER;
	ircPort = FS_PORT;
	ircChannel = FS_CHANNEL;
	joined=0;
	// Protocol regexes
	initRegexes();
	ircEncoding = "UTF-8";
	ircCodec = QTextCodec::codecForName(ircEncoding);
	connect(ircSocket, SIGNAL(connected()), this, SLOT(ircLogon()));
	connect(ircSocket, SIGNAL(readyRead()), this, SLOT(getData()));
	connect(ircSocket, SIGNAL(disconnected()), this, SLOT(gotDisconnected()));
	connect(this, SIGNAL(gotKick(QHash<QString, QString>)), this, SLOT(checkKicked(QHash<QString, QString>)));
//	connect(ircSocket, SIGNAL(error()), this, SLOT(gotError()));
}

void ircLayer::initRegexes()
{
	// These should be used only on "response"/"command" lines
	privmsgRgx.setPattern("^PRIVMSG (\\S+) :(.+)$");
	noticeRgx.setPattern("^NOTICE (\\S+) :(.+)$");
	pingRgx.setPattern("^PING :([_a-zA-Z0-9\\.\\-]+)$");
	ctcpRgx.setPattern("^\x01([A-Z]+)(?: (.+))?\x01$");
	namesRgx.setPattern("^= (\\S+) :(?:(.+)\\s?)+$");
	topicRgx.setPattern("^(\\S+) :(.+)$");
	nickRgx.setPattern("^NICK :(\\S+)$");
	partRgx.setPattern("^PART (\\S+) :(.+)?$");
	joinRgx.setPattern("^JOIN :(\\S+)$");
	quitRgx.setPattern("^QUIT :(.+)?$");
	kickRgx.setPattern("^KICK (\\S+) (\\S+) :(.+)?$");
	modeRgx.setPattern("^MODE (\\S+) (\\S+)(?: (\\S+)?(?: (\\S+)?)?)?$");
	// These two may need tuning
	respRgx.setPattern("^:\\S+ ([0-9]+) (\\S+) (.+)$");
	cmdRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) (.+)$");
	//"[(\x03([0-9]*(,[0-9]+)?)?)\x37\x02\x26\x22\x21]"
	// etc
	ircUriRgx.setPattern("^irc://([_a-zA-Z0-9\\.\\-]+)/(\\S+)$");
	ircUriPortRgx.setPattern("^irc://([_a-zA-Z0-9\\.\\-]+):([0-9]+)/(\\S+)$");
	lineBrRgx.setPattern("[\r\n]");
	mircColors.setPattern("\x03(\\,([0-9][0-5]?|)|([0-9][0-5]?)(\\,([0-9][0-5]?|)|)|\\,|)");
	mircShit.setPattern("[]");
	chanPrefix.setPattern("^[#&\\+]\\S*$");
	genError.setPattern("^(\\S+ )?:(.+)$");
}

QString ircLayer::getIrcUri()
{
	QString pick;
	pick="irc://"+ircServer;
	if(ircPort!=6667) pick +=":"+ircPort;
	pick+="/"+ircChannel;
	return pick;
}

void ircLayer::ircConnect(QString server, int port)
{
	if(ircSocket->isOpen())
	{
		ircSocket->disconnectFromHost();
		if(connected())
			ircSocket->waitForDisconnected();
	}
	infMsg(tr("Connecting to IRC"));
	if(!server.isEmpty()) ircServer=server;
	ircPort=port;
	ircSocket->connectToHost(ircServer,ircPort);
	if(ircSocket->waitForConnected())
		infMsg(tr("Please stand by."));
}

void ircLayer::infMsg(QString message)
{
	emit gotInfo(message);
}

void ircLayer::errMsg(QString message)
{
	emit gotError(message);
}

void ircLayer::ircLogon()
{
	infMsg(tr("Logging in..."));
	ircThrow("USER "+ircIdent+" localhost localhost :"+ircRealname);
	ircThrow("NICK "+ircNick);
	ircJoin(ircChannel);
}

void ircLayer::ircThrow(QString what)
{
	what+="\r\n";
	if(ircSocket->isWritable())
	    ircSocket->write(ircCodec->fromUnicode(what));
	else
	    errMsg(tr("Not connected to server!"));
	// qDebug() << "RAW THROW: " << ircCodec->fromUnicode(what);
}

void ircLayer::ircMsg(QString what, QString where)
{
	ircThrow("PRIVMSG "+where+" :"+what);
}

void ircLayer::ircNotice(QString what, QString where)
{
	ircThrow("NOTICE "+where+" :"+what);
}

void ircLayer::ircSetNick(QString nick)
{
	if(ircNick!=nick)
	{
		if(connected())
		ircThrow("NICK "+nick);
		else
		{
		    infMsg(tr("Nickname has been set to ")+nick);
		    ircNick=nick;
		}
	}
}

void ircLayer::ircMode(QString modes)
{
	ircThrow("MODE "+ircChannel+" "+modes);
}

void ircLayer::ircJoin(QString channel)
{
	if(joined) ircPart(ircChannel,"...");
	ircThrow("JOIN "+channel);
	joined=1;
//	ircChannel=channel;
}

void ircLayer::ircPart(QString channel, QString message)
{
	ircThrow("PART "+channel+" :"+message);
	if(channel==ircChannel)
		joined=0;
}

void ircLayer::ircQuit(QString message)
{
	ircThrow("QUIT :"+message);
	if (connected())
		ircSocket->disconnectFromHost();
}

void ircLayer::getData()
{
	while(ircSocket->canReadLine())
		ircParse(ircCodec->toUnicode(ircSocket->readLine().data()));
}

void ircLayer::ircKick(QString whom, QString reason)
{
	ircThrow("KICK "+ircChannel+" "+whom+" :"+reason);
}

void ircLayer::ircParse(QString line)
{
	line.remove(lineBrRgx);
	line.remove(mircColors);
	line.remove(mircShit);
	if(cmdRgx.exactMatch(line))
	{
		QHash<QString, QString> data;
		data["nick"]=cmdRgx.cap(1); // nick
		data["ident"]=cmdRgx.cap(2); // ident
		data["host"]=cmdRgx.cap(3); // host
		parseCmd(cmdRgx.cap(4),data);
	} else
	if(respRgx.exactMatch(line))
	{
		QHash<QString, QString> data;
		data["target"]=respRgx.cap(2);
		parseResp(respRgx.cap(1).toInt(), respRgx.cap(3),data);
	}
	else
	if(pingRgx.exactMatch(line))
	{
		ircThrow("PONG :"+pingRgx.cap(1)); // ping? pong!
	}
}

void ircLayer::parseCtcp(QString type, QString arg, QHash<QString, QString> data)
{
//	What was that for? Hmmm.
//	if(data["target"]==ircNick) data["target"]=data["nick"];
	if(type=="ACTION")
	{
		emit gotAction(data);
	} else if(type=="PING")
	{
		QString answer;
		answer.setNum(time(NULL));
		ircNotice("\x01PING "+answer+"\x01", data["nick"]);
	} else if(type=="VERSION")
	{
		ircNotice("\x01"+FS_VERSION_REPLY+"\x01", data["nick"]);
	}
}

void ircLayer::parseResp(int code, QString args, QHash<QString, QString> data)
{
	QStringList argz;
	if((code>=ERR_NOSUCHNICK)&&(code<=ERR_USERSDONTMATCH)&&(genError.exactMatch(args)))
	{
		argz=genError.capturedTexts();
	}
	switch(code)
	{
		case RPL_NAMREPLY:
	 	if(namesRgx.exactMatch(args))
		{
			emit gotNames(namesRgx.capturedTexts()); // First entry is 	channel, then nicks.
		}
		break;
		case RPL_TOPIC:
		if(topicRgx.exactMatch(args))
		{
			emit gotTopic(topicRgx.capturedTexts()); // Channel,topic
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
	case ERR_YOUREBANNEDCREEP:
	// That's my favourite
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
	case RPL_ENDOFNAMES:
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
void ircLayer::parseCmd(QString cmd, QHash<QString, QString> data)
{
//	qDebug() << "Command=" << cmd << ", cmd=" << cmd;
	if(privmsgRgx.exactMatch(cmd))
	{
		data["target"]=privmsgRgx.cap(1); // target
		if(ctcpRgx.exactMatch(privmsgRgx.cap(2)))
		{
			data["text"]=ctcpRgx.cap(2);
			parseCtcp(ctcpRgx.cap(1), ctcpRgx.cap(2), data);
		} else
		{
			data["text"]=privmsgRgx.cap(2); // text
			if(chanPrefix.exactMatch(data["target"]))
				emit gotChannelMsg(data);
			else
				emit gotPrivMsg(data);
			emit gotMsg(data);
		}
	} else
	if(noticeRgx.exactMatch(cmd))
	{
		data["target"]=noticeRgx.cap(1); // target
		data["text"]=noticeRgx.cap(2); // text
//		qDebug() << "Target, text=" << data["target"] << data["text"];
		emit gotNotice(data);
	} else
	if(partRgx.exactMatch(cmd))
	{
		data["target"]=partRgx.cap(1); // target
		data["text"]=partRgx.cap(2); // text
		if((data["target"]==ircChannel)&&(data["nick"]==ircNick))
			joined=0;
		emit gotPart(data);
	} else
	if(quitRgx.exactMatch(cmd))
	{
		data["text"]=quitRgx.cap(1); // text
		emit gotQuit(data);
	} else
	if(joinRgx.exactMatch(cmd))
	{
		data["target"]=joinRgx.cap(1); // target
		// if it's about me
		if((data["nick"]==ircNick)&&(ircChannel!=data["target"]))
		{
			ircChannel=data["target"];
			joined=1;
		}
		emit gotJoin(data);
	} else
	if(kickRgx.exactMatch(cmd))
	{
		data["target"]=kickRgx.cap(1); // target
		data["subject"]=kickRgx.cap(2); // whom
		data["text"]=kickRgx.cap(3); // reason
		emit gotKick(data);
	} else
	if(modeRgx.exactMatch(cmd))
	{
		data["target"]=modeRgx.cap(1); // target
		data["text"]=modeRgx.cap(2); // modecmd
		data["subject"]=modeRgx.cap(3); // subject
		emit gotMode(data);
	} else
	if(nickRgx.exactMatch(cmd))
	{
		data["target"]=nickRgx.cap(1); // target
		// if it's our nick, change it. Don't want to make slot crap for this.
		if(data["nick"]==ircNick)
			ircNick=data["target"];
		emit gotNick(data);
	}
}

QString ircLayer::ircUseUri(QString uri)
{
	QString chan;
	if(ircUriRgx.exactMatch(uri))
	{
		if(!ircUriRgx.cap(2).isEmpty())
		{
			chan=ircUriRgx.cap(2);
			if(!chanPrefix.exactMatch(chan))
				chan.prepend("#");
		}
		if((ircServer!=ircUriRgx.cap(1))||(!ircSocket->isOpen()))
		{
			joined=0;
			ircChannel=chan;
			ircConnect(ircUriRgx.cap(1),6667);
		} else
		ircJoin(chan);
		return "irc://"+ircUriRgx.cap(1)+"/"+chan;
	}else if(ircUriPortRgx.exactMatch(uri))
	{
		if(!ircUriPortRgx.cap(3).isEmpty())
		{
			chan=ircUriPortRgx.cap(3);
			if(!chanPrefix.exactMatch(chan))
				chan.prepend("#");
		}
		if((ircServer!=ircUriRgx.cap(1))||(ircPort!=ircUriPortRgx.cap(2).toInt())||(!ircSocket->isOpen()))
		{
			joined=0;
			ircChannel=chan;
			ircConnect(ircUriPortRgx.cap(1),ircUriPortRgx.cap(2).toInt());
		} else
		ircJoin(chan);
		return "irc://"+ircUriPortRgx.cap(1)+":"+ircUriPortRgx.cap(2)+"/"+chan;
	} else
	{
		errMsg(tr("Invalid IRC URI"));
		return "";
	}
}

void ircLayer::ircNs(QString what)
{
	ircThrow("NICKSERV "+what);
}

void ircLayer::ircCs(QString what)
{
	ircThrow("CHANSERV "+what);
}

void ircLayer::ircMs(QString what)
{
	ircThrow("MEMOSERV "+what);
}

void ircLayer::gotDisconnected()
{
	errMsg(tr("Disconnected from server."));
}

QString ircLayer::nick()
{
	return ircNick;
}
QString ircLayer::ident()
{
	return ircIdent;
}
QString ircLayer::realname()
{
	return ircRealname;
}
QString ircLayer::channel()
{
	return ircChannel;
}
QString ircLayer::server()
{
	return ircServer;
}
QByteArray ircLayer::encoding()
{
	return ircEncoding;
}
int ircLayer::port()
{
	return ircPort;
}
int ircLayer::connected()
{
	return (ircSocket->state()==QAbstractSocket::ConnectedState);
}

int ircLayer::setEncoding(QString enc)
{
	if(QTextCodec::availableCodecs().contains(enc.toAscii()))
	{
		ircCodec=QTextCodec::codecForName(enc.toAscii());
		infMsg(tr("Encoding has been set to ")+enc);
		return 1;
	} else
	{
		errMsg(tr("No such encoding!"));
		return 0;
	}
}

QHash<QString, QString> ircLayer::chewIrcUri(QString uri)
{
	QHash<QString, QString> ret;
	if(ircUriPortRgx.exactMatch(uri))
	{
		ret["server"]=ircUriPortRgx.cap(1);
		ret["port"]=ircUriPortRgx.cap(2);
		ret["channel"]=ircUriPortRgx.cap(3);
	}
	else if(ircUriRgx.exactMatch(uri))
	{
		ret["server"]=ircUriRgx.cap(1);
		ret["channel"]=ircUriRgx.cap(2);
	}
	return ret;
}

int ircLayer::isJoined()
{
	return joined;
}

void ircLayer::checkKicked(QHash<QString, QString> data)
{
	if((data["subject"]==ircNick)&&(data["target"]==ircChannel))
		joined=0; // Alas.
}
/*
101
111
*/
