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

ircLayer::ircLayer()
{
	fSettings settings;
	// Moved default nick stuff to fsirc.cpp
	qsrand(time(NULL) );
	ircNick = QString("fsirc%1").arg( (int)(qrand()*(9999.0/RAND_MAX)) ) ;
	ircSocket = new QTcpSocket;
	ircRealname = FS_REALNAME;
	ircIdent = ircNick;
	ircServer = FS_SERVER;
	ircPort = FS_PORT;
	ircChannel = FS_CHANNEL;
	// Protocol regexes
	initRegexes();
	ircEncoding = "UTF-8";
	ircCodec = QTextCodec::codecForName(ircEncoding);
	connect(ircSocket, SIGNAL(connected()), this, SLOT(ircLogon()));
	connect(ircSocket, SIGNAL(readyRead()), this, SLOT(getData()));
	connect(ircSocket, SIGNAL(disconnected()), this, SLOT(gotDisconnected()));
//	connect(ircSocket, SIGNAL(error()), this, SLOT(gotError()));
}

void ircLayer::initRegexes()
{
	privmsgRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) PRIVMSG (\\S+) :(.+)$");
	noticeRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) NOTICE (\\S+) :(.+)$");
	pingRgx.setPattern("^PING :([_a-zA-Z0-9\\.\\-]+)$");
	ctcpRgx.setPattern("^\x01([A-Z]+)(?: (.+))?\x01$");
	namesRgx.setPattern("^:[_a-zA-Z0-9\\.\\-]+ 353 \\S+ = (\\S+) :(?:(.+)\\s?)+$");
	topicRgx.setPattern("^:[_a-zA-Z0-9\\.\\-]+ 332 \\S+ (\\S+) :(.+)$");
	nickRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) NICK :(\\S+)$");
	partRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) PART (\\S+) :(.+)?$");
	joinRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) JOIN :(\\S+)$");
	quitRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) QUIT :(.+)?$");
	kickRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) KICK (\\S+) (\\S+) :(.+)?$");
	modeRgx.setPattern("^:(\\S+)!(\\S+)@(\\S+) MODE (\\S+) (\\S+)(?: (\\S+)?(?: (\\S+)?)?)?$");
	//"[(\x03([0-9]*(,[0-9]+)?)?)\x37\x02\x26\x22\x21]"
	// etc
	ircUriRgx.setPattern("^irc://([_a-zA-Z0-9\\.\\-]+)(?:/(\\S+))?$");
	ircUriPortRgx.setPattern("^irc://([_a-zA-Z0-9\\.\\-]+):([0-9]+)(?:/(\\S+))?$");
	lineBrRgx.setPattern("[\r\n]");
	mircColors.setPattern("\x03(\\,([0-9][0-5]?|)|([0-9][0-5]?)(\\,([0-9][0-5]?|)|)|\\,|)");
	mircShit.setPattern("[]");
	chanPrefix.setPattern("^[#&\\+]\\S*$");
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
	    infMsg(tr("Error: not connected to server!"));
//	qDebug() << "RAW THROW: " << ircCodec->fromUnicode(what);
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
		    infMsg(tr("Nickname changed to ")+nick);
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
	ircPart(ircChannel,"...");
	ircThrow("JOIN "+channel);
//	ircChannel=channel;
}

void ircLayer::ircPart(QString channel, QString message)
{
	ircThrow("PART "+channel+" :"+message);
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
//	qDebug() << "RAW GET: " << line;
	line.remove(lineBrRgx);
	line.remove(mircColors);
	line.remove(mircShit);
//	line.setUnicode(line);
	if(privmsgRgx.exactMatch(line))
	{
		QHash<QString, QString> data;

		data["nick"]=privmsgRgx.cap(1); // nick
		data["ident"]=privmsgRgx.cap(2); // ident
		data["host"]=privmsgRgx.cap(3); // host
		data["target"]=privmsgRgx.cap(4); // target
		if(ctcpRgx.exactMatch(privmsgRgx.cap(5)))
		{
			data["text"]=ctcpRgx.cap(2);
			parseCtcp(ctcpRgx.cap(1), ctcpRgx.cap(2), data);
		} else
		{
			data["text"]=privmsgRgx.cap(5); // text
			emit gotMsg(data);
		}
	} else
	if(noticeRgx.exactMatch(line))
	{
		QHash<QString, QString> data;
		data["nick"]=noticeRgx.cap(1); // nick
		data["ident"]=noticeRgx.cap(2); // ident
		data["host"]=noticeRgx.cap(3); // host
		data["target"]=noticeRgx.cap(4); // target
		data["text"]=noticeRgx.cap(5); // text
		emit gotNotice(data);
	} else
		if(partRgx.exactMatch(line))
		{
			QHash<QString, QString> data;
			data["nick"]=partRgx.cap(1); // nick
			data["ident"]=partRgx.cap(2); // ident
			data["host"]=partRgx.cap(3); // host
			data["target"]=partRgx.cap(4); // target
			data["text"]=partRgx.cap(5); // text
			emit gotPart(data);
		} else
		if(quitRgx.exactMatch(line))
		{
			QHash<QString, QString> data;
			data["nick"]=quitRgx.cap(1); // nick
			data["ident"]=quitRgx.cap(2); // ident
			data["host"]=quitRgx.cap(3); // host
			data["text"]=quitRgx.cap(4); // text
			emit gotQuit(data);
		} else
		if(joinRgx.exactMatch(line))
		{
			QHash<QString, QString> data;
			data["nick"]=joinRgx.cap(1); // nick
			data["ident"]=joinRgx.cap(2); // ident
			data["host"]=joinRgx.cap(3); // host
			data["target"]=joinRgx.cap(4); // target
			emit gotJoin(data);
			if((data["nick"]==ircNick)&&(ircChannel!=data["target"]))
			{
				ircPart(ircChannel);
				ircChannel=data["target"];
			}
		} else
		if(kickRgx.exactMatch(line))
		{
			QHash<QString, QString> data;
			data["nick"]=kickRgx.cap(1); // nick
			data["ident"]=kickRgx.cap(2); // ident
			data["host"]=kickRgx.cap(3); // host
			data["target"]=kickRgx.cap(4); // target
			data["subject"]=kickRgx.cap(5); // whom
			data["text"]=kickRgx.cap(6); // reason
			emit gotKick(data);
		} else
		if(modeRgx.exactMatch(line))
		{
			QHash<QString, QString> data;
			data["nick"]=modeRgx.cap(1); // nick
			data["ident"]=modeRgx.cap(2); // ident
			data["host"]=modeRgx.cap(3); // host
			data["target"]=modeRgx.cap(4); // target
			data["text"]=modeRgx.cap(5); // modeline
			data["subject"]=modeRgx.cap(6); // subject
			emit gotMode(data);
		} else
	if(pingRgx.exactMatch(line))
	{
		ircThrow("PONG :"+pingRgx.cap(1)); // ping? pong!
	} else
	if(namesRgx.exactMatch(line))
	{
		emit gotNames(namesRgx.capturedTexts()); // First entry is channel, then nicks.
	} else
	if(topicRgx.exactMatch(line))
	{
		emit gotTopic(topicRgx.capturedTexts()); // Channel,topic
	} else
	if(nickRgx.exactMatch(line))
	{
		QHash<QString, QString> data;
		data["nick"]=nickRgx.cap(1); // nick
		data["ident"]=nickRgx.cap(2); // ident
		data["host"]=nickRgx.cap(3); // host
		data["target"]=nickRgx.cap(4); // target
		// if it's our nick, change it. Don't want to make slot crap for this.
		if(data["nick"]==ircNick)
			ircNick=data["target"];
		emit gotNick(data);
	}
}

void ircLayer::parseCtcp(QString type, QString arg, QHash<QString, QString> data)
{
	if(data["target"]==ircNick) data["target"]=data["nick"];
	if(type=="ACTION")
	{
		emit gotAction(data);
	} else if(type=="PING")
	{
		QString answer;
		answer.setNum(time(NULL));
		ircNotice("\x01PING "+answer+"\x01", data["target"]);
	} else if(type=="VERSION")
	{
		ircNotice("\x01VERSION fsirc v"+QString(FS_VERSION)+", using Qt v"+QString(QT_VERSION_STR)+"\x01", data["target"]);
	}
}

int ircLayer::ircUseUri(QString uri)
{
	QString chan;
	if(ircUriRgx.exactMatch(uri))
	{
		if(!ircUriRgx.cap(2).isEmpty())
		{
			chan=ircUriRgx.cap(2);
			if(!chanPrefix.exactMatch(chan))
				chan="#"+chan;
		}
		if((ircServer!=ircUriRgx.cap(1))||(!ircSocket->isOpen()))
		{
			ircChannel=chan;
			ircConnect(ircUriRgx.cap(1),6667);
		} else ircJoin(chan);
		return 1;
	}else if(ircUriPortRgx.exactMatch(uri))
	{
		if(!ircUriPortRgx.cap(3).isEmpty())
		{
			ircChannel=ircUriPortRgx.cap(3);
			if(!chanPrefix.exactMatch(ircChannel))
				ircChannel="#"+ircChannel;
		}
		if((ircServer!=ircUriRgx.cap(1))||(ircPort!=ircUriPortRgx.cap(2).toInt())||(!ircSocket->isOpen()))
		{
			ircChannel=chan;
			ircConnect(ircUriPortRgx.cap(1),ircUriPortRgx.cap(2).toInt());
		} else ircJoin(chan);
		return 1;
	} else
	return -1;
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
	infMsg(tr("Disconnected from server."));
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

void ircLayer::setEncoding(QString enc)
{
	if(QTextCodec::availableCodecs().contains(enc.toAscii()))
	{
		ircCodec=QTextCodec::codecForName(enc.toAscii());
	} else infMsg(tr("No such encoding!"));
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
