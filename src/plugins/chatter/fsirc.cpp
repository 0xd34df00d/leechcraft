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
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
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

	actionNewTab = new QAction (QIcon (":/fsirc/data/new.svg"), tr ("New tab"), this);
	actionNewTab->setToolTip (tr ("Create new tab"));
	ircTabHolder->addAction(actionNewTab);

	actionCloseTab = new QAction (QIcon (":/fsirc/data/close.svg"), tr ("Close tab"), this);
	actionCloseTab->setToolTip (tr ("Close current tab"));
	actionCloseTab->setDisabled (true);
	ircTabHolder->addAction(actionCloseTab);

	cornerButtons = new QToolBar(ircTabHolder);
	cornerButtons->addAction(actionNewTab);
	cornerButtons->addAction(actionCloseTab);

	QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*> (layout ());
	mainLayout->insertWidget (0, cornerButtons);

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
	connect(actionCloseTab, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
	connect(actionNewTab, SIGNAL(triggered()), this, SLOT(newTab()));
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
	if (ircTabHolder->count() > 0)
	{
		delete ircList.takeAt(ircTabHolder->currentIndex());
	}

	if (ircTabHolder->count() == 0)
		actionCloseTab->setDisabled(true);
}

void fsirc::newTab(QString uri)
{
//	if (uri.isEmpty()) {
//		QStringList items;
//		items << "irc://irc.freenode.net/#fsirc"
//				<< "irc://irc.freenode.net/#qt-ru";
//
//		bool ok = false;
//		uri = QInputDialog::getItem (this, tr ("IRC URI"), tr ("IRC URI"),
//									 items, -1, true, &ok);
//
//		if (!ok)
//			return;
//	}
//
//	if (uri.isEmpty ()) {
//		QMessageBox::critical (this, "", tr ("SRC URI is empty"));
//		return;
//	}
//
//	if (!IrcLayer::isIrcUri (uri)) {
//		QMessageBox::critical (this, "", tr ("Incorrect SRC URI"));
//		return;
//	}

	FsIrcView *ircView = new FsIrcView ();
	connect(ircView, SIGNAL(uriChanged()), this, SLOT(refreshTabNames()));
	connect(ircView, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
	connect(ircView, SIGNAL(gotSomeMsg()), this, SLOT(gotSomeMsg()));
	connect(ircView, SIGNAL(gotHlite()), this, SLOT(gotHlite()));
	ircList << ircView;
//	ircView->proposeUri(uri);
//	ircView->openIrc(uri);
	ircTabHolder->setCurrentIndex(ircTabHolder->addTab(ircView, uri));

	refreshTabNames();
	actionCloseTab->setEnabled (ircTabHolder->count () > 0);
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
