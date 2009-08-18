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

#include "fstrayicon.h"

fsTrayIcon::fsTrayIcon(QObject * parent) : QSystemTrayIcon(parent)
{
	state=0;
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(seeIfClicked(QSystemTrayIcon::ActivationReason)));
	icons << QIcon(":/fsirc/data/icon.svg") << QIcon(":/fsirc/data/icon-msg.svg") << QIcon(":/fsirc/data/icon-hlite.svg");
	updIcon();
}

void fsTrayIcon::raiseState(int newState)
{
	if(newState>state)
	{
		state=newState;
		updIcon();
	}
}

void fsTrayIcon::resetState()
{
	if(state!=0)
	{
		state=0;
		updIcon();
	}
}

int fsTrayIcon::getState()
{
	return state;
}

void fsTrayIcon::updIcon()
{
	setIcon(icons[state]);
}

void fsTrayIcon::gotMsg()
{
	raiseState(1);
}

void fsTrayIcon::gotHlite()
{
	raiseState(2);
}

void fsTrayIcon::seeIfClicked(QSystemTrayIcon::ActivationReason reason)
{
	if (reason==QSystemTrayIcon::Trigger)
		emit clicked();
}
