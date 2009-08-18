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
#ifndef FSTRAYICON_H
#define FSTRAYICON_H
#include <QSystemTrayIcon>

class fsTrayIcon : public QSystemTrayIcon
{
	Q_OBJECT
public:
	fsTrayIcon(QObject * parent = 0);
	fsTrayIcon(const QIcon & icon, QObject * parent = 0);
	int getState();
private:
	int state;
	QList<QIcon> icons;
	void updIcon();
private slots:
	void seeIfClicked(QSystemTrayIcon::ActivationReason reason);
public slots:
	void raiseState(int newState);
	void resetState();
	// For convenience
	void gotMsg();
	void gotHlite();
signals:
	void clicked();
};

#endif