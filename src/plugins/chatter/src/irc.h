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
#ifndef IRC_H
#define IRC_H

#include <QtCore/QObject>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>

#include "config.h"
#include "ircserver.h"


class IrcLayer : public QObject
{
Q_OBJECT
public:
	IrcLayer(QObject * parent, QString ircUri);
	~IrcLayer();
	void ircConnect();
	void ircThrow(QString what);
	void ircMsg(QString what, QString where);
	void ircJoin(QString channel);
	void ircQuit(QString message);
	void ircPart(QString channel, QString message="...");
	void ircKick(QString whom, QString where, QString reason=tr("See ya in hell!"));
	void ircMode(QString modes);
	void ircNotice(QString what, QString where);
	void ircNs(QString what);
	void ircCs(QString what);
	void ircMs(QString what);
	// property handling
	void ircSetNick(QString nick);
	QString ircUseUri(QString uri);
	QString getIrcUri();
	QString nick();
	QString ident();
	QString realname();
	QString channel();
	QString server();
	QByteArray encoding();
	QString port();
	int setEncoding(const QString theValue);
	QByteArray encoding() const;
	void contactServer();
	int connected();
	QHash<QString, QString> chewIrcUri(QString uri);
	static QString composeIrcUri(QHash<QString, QString> data);
	enum {ChannelMode, PrivateMode};
	static bool isIrcUri(QString uri);
	static QString cleanUri(QString);
	void setNickChanged(bool theValue);
	bool nickChanged() const;
	QString target() const;
	int joined() const;
	int targetMode() const;
	void say(QString msg);
	QStringList users() const;

	bool active() const;


private:
	void setJoined(int theValue);
	void setTarget(const QString& theValue);
	void setTargetMode(int theValue);
	int m_joined;
	bool m_active;
	IrcServer * m_ircServer;
	static QHash<QString, IrcServer *> m_servers;
	IrcServer * getServer(QString host, QString port="6667");
	void ircSaveNames();
	QString m_ident;
	QString m_realname;
	QString m_server;
	QString m_target;
	QStringList m_users;
	QStringList m_usersTemp; // Used for temporary saving of users' names during long NAMREPLies
	int m_targetMode;
	QString m_port;
	//! int joined: 0 if not on channel; 1 otherwise.
	QTextCodec * m_codec;
	QByteArray m_encoding;
	QHash<QString, QRegExp> prRegexes;
	QRegExp * ircUriRgx;
	QRegExp * ircUriPortRgx;
	QRegExp * chanPrefix;
	QRegExp * mircColors;
	QRegExp * mircShit;
	QRegExp * genError;
	// methods
	void parseCmd(QString cmd, QHash<QString, QString> data);
	void parseResp(int code, QString args, QHash<QString, QString> data);
	void initRegexes();
signals:
	void gotMsg(QHash<QString, QString>);
	void gotChannelMsg(QHash<QString, QString>);
	void gotPrivMsg(QHash<QString, QString>);
	void gotNotice(QHash<QString, QString>);
	void gotInfo(QString);
	void gotError(QString);
	void gotAction(QHash<QString, QString>);
	void gotNames(QStringList);
	void gotTopic(QStringList);
	void gotNick(QHash<QString, QString>);
	void gotJoin(QHash<QString, QString>);
	void gotPart(QHash<QString, QString>);
	void gotQuit(QHash<QString, QString>);
	void gotMode(QHash<QString, QString>);
	void gotKick(QHash<QString, QString>);
protected slots:
	void ircParse(QByteArray data);
	//void gotError();
	void ircLogon();
	void infMsg(QString message);
	void errMsg(QString message);
	void gotDisconnected();
	void checkKicked(QHash<QString, QString>);
	void addNames(QStringList);
};

#endif
