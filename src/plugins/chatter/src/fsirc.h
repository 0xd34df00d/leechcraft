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
#ifndef FSIRC_H
#define FSIRC_H

#include <QDialog>
#include <QTimer>
#include <QDebug>
#include <QHash>
#include <QCompleter>
#include <QSystemTrayIcon>

#include "fsettings.h"
#include "fscmdedit.h"
#include "fstrayicon.h"
#include "ui_fsmain.h"
#include "irc.h"

class fsirc : public QDialog, public Ui::fsMainWindow
{
	Q_OBJECT
	public:
		fsirc(QWidget *parent= 0);
		~fsirc();
		void fsEcho(QString message, QString style=QString());
		void fsExec(QString cmd, QString arg="");
		QRegExp * mArg;
		QRegExp * linkRegexp;
		// Dropdown actions
		enum {ACT_URI, ACT_NICK, ACT_ENCODING, ACT_QUIT};
	private:
		ircLayer *irc;
		QHash<int, QStringList> completeLists;
		QHash<int, QCompleter *> actionCompleters;
		void initCompleters();
		void initConnections();
		void addTrayIcon();
		void nickToHistory(QString nick);
		fsTrayIcon * trayIcon;
		QTimer * ticker;
		QHash<QString,QString> msgColors;
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
		void gotSomeMsg();
		void gotHlite();
		void anchorClicked(QUrl url);
	public slots:
		void fsQuit();
		void sayHere();
		void pickAction();
		void takeAction();
		void toggleShow();
		void checkIfTop();
	signals:
		void gotLink(QString);
};
#endif
