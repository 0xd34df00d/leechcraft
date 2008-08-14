#ifndef FSCMDEDIT_H
#define FSCMDEDIT_H
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
#include <QLineEdit>
#include <QKeyEvent>
class fsCmdEdit : public QLineEdit
{
	Q_OBJECT
	public:
		fsCmdEdit(QWidget * parent=0);
		void toHistory(QString entry);
	public slots:
		// public slot toHistory() — adds current command line to history
		void currentToHistory();
	protected:
		void keyReleaseEvent(QKeyEvent * event);
		int cmdIndex;
		QStringList cmdHistory;

};
#endif
