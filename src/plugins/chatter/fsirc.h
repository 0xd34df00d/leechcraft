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

#include <QtGui/QDialog>
#include <QtCore/QUrl>
#include <QtCore/QList>

#include "fsettings.h"
#include "fscmdedit.h"
#include "fstrayicon.h"
#include "ui_fsmain.h"
#include "irc.h"

class QTimer;
class QToolBar;
class FsIrcView;

class fsirc : public QDialog, Ui::fsMainWindow
{
	Q_OBJECT
	public:
		fsirc(QWidget *parent= 0);
		virtual ~fsirc();
		static void finalizeIrcList();
		static int findTab(QString);
		void addTrayIcon();
		void removeTrayIcon();
	private:
		void initConnections();
		fsTrayIcon * trayIcon;
		QTimer * ticker;
		QPushButton * closeTabButton;
		QPushButton * newTabButton;
		QToolBar * cornerButtons;
		static QList<FsIrcView *> ircList;
	public slots:
		void toggleShow();
		void checkIfTop();
		void setTrayPresence(QVariant);
		void newTab(QString uri=QString());
	private slots:
		void gotSomeMsg();
		void gotHlite();
		void anchorClicked(QUrl url);
		void closeCurrentTab();
		void refreshTabNames();
	signals:
		void gotLink(QString);
};
#endif
