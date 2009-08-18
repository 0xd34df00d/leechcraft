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
#include <QtGui/QDialog>
#include <QtGui/QScrollBar>
#include <QtGui/QToolBar>
#include <QtGui/QPushButton>
#include <QtGui/QDesktopServices>
#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QTimer>

#include "config.h"
#include "fsirc.h"
#include "fsircview.h"

QList<FsIrcView *> fsirc::ircList;

fsirc::fsirc(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	closeTabButton = new QPushButton(QIcon(":/fsirc/data/close.svg"),QString(),this);
	newTabButton = new QPushButton(QIcon(":/fsirc/data/new.svg"),QString(),this);
	clearTabButton = new QPushButton(QIcon(":/fsirc/data/clear.svg"), QString(), this);
	closeTabButton->setFocusPolicy(Qt::NoFocus);
	newTabButton->setFocusPolicy(Qt::NoFocus);
	clearTabButton->setFocusPolicy(Qt::NoFocus);
	cornerButtons = new QToolBar(ircTabHolder);
	cornerButtons->addWidget(closeTabButton);
	cornerButtons->addWidget(newTabButton);
	cornerButtons->addWidget(clearTabButton);
	closeTabButton->setDisabled(true);
	ircTabHolder->setCornerWidget(cornerButtons);
	newTab();
	setWindowIcon(QIcon(":/fsirc/data/icon.svg"));
	ticker = new QTimer;
	ticker->setInterval(700);
	ticker->start();
	initConnections();
	connect(ticker, SIGNAL(timeout()), this, SLOT(checkIfTop()));
	trayIcon = 0;
}

fsirc::~fsirc()
{
	delete ticker;
}

void fsirc::addTrayIcon()
{
	if(!trayIcon)
	{
		trayIcon = new fsTrayIcon(this);
		trayIcon->show();
		connect(trayIcon, SIGNAL(clicked()), this, SLOT(toggleShow()));
	}
}

void fsirc::removeTrayIcon()
{
	if(trayIcon)
		delete trayIcon;
}
void fsirc::initConnections()
{
	connect(closeTabButton, SIGNAL(released()), this, SLOT(closeCurrentTab()));
	connect(newTabButton, SIGNAL(released()), this, SLOT(newTab()));
	connect(clearTabButton, SIGNAL(released()), this, SLOT(clearCurrentTab()));
}

void fsirc::gotSomeMsg()
{
	if (trayIcon && !isActiveWindow())
		trayIcon->raiseState(1);
}

void fsirc::gotHlite()
{
	if (trayIcon && !isActiveWindow())
		trayIcon->raiseState(2);
}

void fsirc::toggleShow()
{
	// I've done some stuff to bring window to top here, but it doesn't work. At least on kde 3.5
	if(trayIcon && isHidden()/* || !isActiveWindow()*/)
	{
		show();
//		window()->activateWindow();
//		window()->raise();
//		window()->setFocus();
		trayIcon->resetState();
	}
	else
	{
//		qDebug("Hiding window");
		hide();
	}

}

void fsirc::checkIfTop()
{
	if(trayIcon && isActiveWindow())
	{
		trayIcon->resetState();
	}
}

void fsirc::anchorClicked(QUrl url)
{
	if(url.scheme()=="irc" && IrcLayer::isIrcUri(url.toString()))
	{
		newTab(IrcLayer::cleanUri(url.toString()));
	} else QDesktopServices::openUrl(url);
}

void fsirc::closeCurrentTab()
{
	if (ircTabHolder->count()>1)
	{
		delete ircList.takeAt(ircTabHolder->currentIndex());
	}
	if(ircTabHolder->count()==1)
		closeTabButton->setDisabled(true);
}

void fsirc::newTab(QString uri)
{
	FsIrcView * ircView = new FsIrcView();
	connect(ircView, SIGNAL(uriChanged()), this, SLOT(refreshTabNames()));
	connect(ircView, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
	connect(ircView, SIGNAL(gotSomeMsg()), this, SLOT(gotSomeMsg()));
	connect(ircView, SIGNAL(gotHlite()), this, SLOT(gotHlite()));
	ircList << ircView;
	if(!uri.isEmpty())
	{
		ircView->proposeUri(uri);
		ircView->openIrc(uri);
	}
	else
		ircView->pickAction();
	ircTabHolder->setCurrentIndex(ircTabHolder->addTab(ircView,"newtab"));

	refreshTabNames();
	if(ircTabHolder->count()>1) closeTabButton->setDisabled(false);
}

void fsirc::refreshTabNames()
{
	for(int i=0; i<ircTabHolder->count(); ++i)
	{
		qDebug() << "updating names" << i << ircList[i]->ircUri();
		ircTabHolder->setTabText(i, ircList[i]->ircUri());
	}
}

int fsirc::findTab(QString uri)
{
	for(int i=0; i<ircList.count(); ++i)
	{
		if (ircList[i]->ircUri() == uri) return i;
	}
	return -1;
}

void fsirc::clearCurrentTab()
{
	ircList[ircTabHolder->currentIndex()]->clearView();
}

void fsirc::finalizeIrcList()
{
	qDeleteAll (ircList);
}

void fsirc::setTrayPresence(QVariant v)
{
	if(v.toBool())
		addTrayIcon();
	else
		removeTrayIcon();
}