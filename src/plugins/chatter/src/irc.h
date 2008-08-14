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
#include <QObject>
#include <QTcpSocket>
#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QTextCodec>

class ircLayer : public QObject
{
	Q_OBJECT
	public:
		ircLayer(QObject * parent);
		void ircConnect(QString server, int port=6667);
		void ircThrow(QString what);
		void ircMsg(QString what, QString where);
		void ircJoin(QString channel);
		void ircQuit(QString message);
		void ircPart(QString channel, QString message="...");
		void ircKick(QString whom, QString reason=tr("See ya in hell!"));
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
		int port();
		int connected();
		int isJoined();
		int setEncoding(QString enc);
		//! int nickChanged: 0 if user have not set nick; 1 otherwise.
		int nickChanged;
		QHash<QString, QString> chewIrcUri(QString uri);
	private:
		QString ircNick;
		QString ircIdent;
		QString ircRealname;
		QString ircChannel;
		QString ircServer;
		QByteArray ircEncoding;
		int ircPort;
		//! int joined: 0 if not on channel; 1 otherwise.
		int joined;
		QTcpSocket *ircSocket;
		QTextCodec *ircCodec;
		QRegExp privmsgRgx;
		QRegExp noticeRgx;
		QRegExp pingRgx;
		QRegExp ctcpRgx;
		QRegExp namesRgx;
		QRegExp topicRgx;
		QRegExp lineBrRgx;
		QRegExp nickRgx;
		QRegExp partRgx;
		QRegExp quitRgx;
		QRegExp joinRgx;
		QRegExp modeRgx;
		QRegExp kickRgx;
		QRegExp respRgx;
		QRegExp cmdRgx;
		QRegExp ircUriRgx;
		QRegExp ircUriPortRgx;
		QRegExp chanPrefix;
		QRegExp mircColors;
		QRegExp mircShit;
		QRegExp genError;
		// methods
		void infMsg(QString message);
		void errMsg(QString message);
		void parseCtcp(QString type, QString arg, QHash<QString, QString> data);
		void parseCmd(QString cmd, QHash<QString, QString> data);
		void parseResp(int code, QString args, QHash<QString, QString> data);
		void initRegexes();
	signals:
		void gotMsg(QHash<QString, QString> data);
		void gotChannelMsg(QHash<QString, QString> data);
		void gotPrivMsg(QHash<QString, QString> data);
		void gotNotice(QHash<QString, QString>);
		void gotInfo(QString message);
		void gotError(QString message);
		void gotAction(QHash<QString, QString> data);
		void gotNames(QStringList data);
		void gotTopic(QStringList data);
		void gotNick(QHash<QString, QString>);
		void gotJoin(QHash<QString, QString>);
		void gotPart(QHash<QString, QString>);
		void gotQuit(QHash<QString, QString>);
		void gotMode(QHash<QString, QString>);
		void gotKick(QHash<QString, QString>);

	protected slots:
		void ircLogon();
		void getData();
		void ircParse(QString line);
		//void gotError();
		void gotDisconnected();
		void checkKicked(QHash<QString, QString>);
};
#endif
