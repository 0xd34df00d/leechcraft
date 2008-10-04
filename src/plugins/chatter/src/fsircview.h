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

#include <QtCore/QHash>

#include "ui_fsircview.h"

class IrcLayer;

class FsIrcView : public QWidget, Ui::fsIrcView
{
	Q_OBJECT
public:
	FsIrcView(QWidget * parent=0);
	~FsIrcView();
	void fsEcho(QString message, QString style=QString());
	void fsOut(QString message); // raw HTML output
	void fsExec(QString cmd, QString arg=QString());
	void openIrc(QString uri);
	QString ircUri() const;
	void proposeUri(QString);
private:
	void nickToHistory(QString nick);
	void initCompleters();
	void initConnections();
	IrcLayer * m_irc;
	QHash<int, QCompleter *> m_actionCompleters;
	QRegExp * m_mArg;
	QRegExp * m_linkRegexp;
	QRegExp * m_chanRegexp;
	// Dropdown actions
	enum {ACT_URI, ACT_NICK, ACT_ENCODING, ACT_QUIT};
	QHash<QString,QString> m_msgColors;
private slots:
	void gotChannelMsg(QHash<QString, QString> data);
	void gotPrivMsg(QHash<QString, QString> data);
	void gotNotice(QHash<QString, QString> data);
	void gotAction(QHash<QString, QString> data);
	void gotInfo(QString message);
	void gotError(QString message);
	void gotNames(QStringList data);
	void gotTopic(QStringList data);
	void gotNick(QHash<QString, QString> data);
	void gotJoin(QHash<QString, QString> data);
	void gotPart(QHash<QString, QString> data);
	void gotQuit(QHash<QString, QString> data);
	void gotMode(QHash<QString, QString> data);
	void gotKick(QHash<QString, QString> data);
	void gotPrivAction(QHash<QString, QString>);
public slots:
	void fsQuit();
	void sayHere();
	void pickAction();
	void takeAction();
	void clearView();
signals:
	void gotLink(QString);
	void errMsg(QString);
	void uriChanged();
	void anchorClicked(QUrl url);
	void gotSomeMsg();
	void gotHlite();
	//retranslation
	/*
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
	*/
};
