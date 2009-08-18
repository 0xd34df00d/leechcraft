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
#include <QtNetwork/QTcpSocket>
#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <ctime>

#include "ircserver.h"
#include "config.h"

IrcServer::IrcServer(QString host, QString port)
{
	qsrand(std::time(NULL));
	qDebug() << "Creating new IRC server" << host << port;
	m_host=host;
	m_port=port;
	m_nick=QString(FS_NICK).arg((int)(qrand()*(9999.0/RAND_MAX)));
	m_nickSet=false;
	m_realname = FS_REALNAME;
	m_ident = FS_IDENT;
	m_socket=new QTcpSocket(this);
	pingRegexp=new QRegExp("^PING :([a-zA-Z0-9\\.\\-]+)$");
	ctcpRegexp=new QRegExp("^:(\\S+)!\\S+@\\S+ PRIVMSG \\S+ :\x01([A-Z]+).*\x01$");
	nickRegexp=new QRegExp("^:(\\S+)!\\S+@\\S+ NICK :(\\S+)$");
	m_refCount=1;
	connect(m_socket, SIGNAL(readyRead()), this, SLOT(getData()));
	connect(m_socket, SIGNAL(connected()), this, SLOT(ircLogon()));
	connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(gotDisconnected()));
}

IrcServer::~IrcServer()
{
	qDebug() << "IrcServer" << m_host << m_port << "is being deleted";
	delete pingRegexp;
	delete ctcpRegexp;
	delete nickRegexp;
}

int IrcServer::contact()
{
	if (!m_socket->isOpen())
	{
		qDebug("Contacting server");
		emit infMsg(tr("Contacting IRC server..."));
		m_socket->connectToHost(m_host, m_port.toInt());
		return 1;
	} else
	return 0;
}

void IrcServer::ircThrow(QString what)
{
	what+="\r\n";
	if(m_socket->isWritable())
	    m_socket->write(what.toAscii());
	else
		emit errMsg(tr("Not connected to server!"));
	//qDebug() << "RAW THROW: " << what;
}

void IrcServer::getData()
{
	while(m_socket->canReadLine())
		preParse(m_socket->readLine());
}

bool IrcServer::isConnected() const
{
	return m_socket->isOpen();
}

void IrcServer::breakContact()
{
	if (m_socket->state () == QAbstractSocket::UnconnectedState)
		return;
	ircThrow("QUIT :"+FS_QUIT_MSG);
	if(!m_socket->waitForDisconnected())
		m_socket->close();
}

void IrcServer::incRefCount()
{
	if (m_refCount<=0)
	{
		m_refCount=0;
		//contact();
	}
	m_refCount++;
}

void IrcServer::decRefCount()
{
	m_refCount--;
	if (m_refCount<=0)
	{
		m_refCount=0;
		breakContact();
	}
}

void IrcServer::preParse(QByteArray line)
{
	QString str=line.simplified();
	bool handled=true;
	// Everything that needs answer here
	//qDebug() << "RAW GET:" << str;
	if(ctcpRegexp->exactMatch(str))
	{
		QString type = ctcpRegexp->cap(2);
		QString from = ctcpRegexp->cap(1);
		if(type=="PING")
		{
			// Ugly.. but what to do?
			ircThrow(QString("NOTICE %1 :\x01PING %2\x01").arg(from,QString::number(time(NULL))));
		} else if(type=="VERSION")
		{
			ircThrow(QString("NOTICE %1 :\x01VERSION %2\x01").arg(from,FS_VERSION_REPLY));
		} else
		handled=false;
	} else
	if(pingRegexp->exactMatch(str))
	{
//		qDebug() << str << "is PING! ";
		ircThrow("PONG :"+pingRegexp->cap(1)); // ping? pong!
	}
	else
	if(nickRegexp->exactMatch(str))
	{
		if(nickRegexp->cap(1)==m_nick)
		{
			m_nick=nickRegexp->cap(2);
			m_nickSet=true;
		}
		handled=false;
	} else
	{
		handled=false;
//		qDebug() << str << "is not a PING" << pingRegexp->pattern();
	}
	if(!handled) emit gotLine(line.simplified());
}

QString IrcServer::nick() const
{
	return m_nick;
}

void IrcServer::setNick(const QString& theValue)
{
	if(isConnected())
		ircThrow("NICK "+theValue);
	else
	{
		m_nick = theValue;
		m_nickSet=true;
	}
}

bool IrcServer::nickSet() const
{
	return m_nickSet;
}

void IrcServer::ircLogon()
{
	emit infMsg(tr("Logging in..."));
	ircThrow("USER "+m_ident+" localhost localhost :"+m_realname);
	ircThrow("NICK "+m_nick);
	emit connected();
}

void IrcServer::setNickSet(bool theValue)
{
	m_nickSet = theValue;
}

void IrcServer::gotDisconnected()
{
	m_socket->close();
}
