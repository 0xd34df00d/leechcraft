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

#include <QtCore/QObject>
#include <QtCore/QString>

class QTcpSocket;
class QRegExp;

class IrcServer : public QObject
{
	Q_OBJECT
public:
	IrcServer(QString host, QString port="6667");
	~IrcServer();
	void breakContact();
	void ircThrow(QString what);
	bool isConnected() const;
	void decRefCount();
	void incRefCount();
	void setNick(const QString& theValue);
	QString nick() const;
	bool nickSet() const;
	void setNickSet(bool theValue);
	int contact();
private:
	QString m_host;
	QString m_port;
	QString m_nick;
	bool m_nickSet;
	QString m_realname;
	QString m_ident;
	QTcpSocket *m_socket;
	int m_refCount;
	void preParse(QByteArray line);
	QRegExp * pingRegexp;
	QRegExp * ctcpRegexp;
	QRegExp * nickRegexp;
private slots:
	void getData();
	void ircLogon();
	void gotDisconnected();
signals:
	void gotLine(QByteArray);
	void connected();
	void disconnected();
	void errMsg(QString);
	void infMsg(QString);
};