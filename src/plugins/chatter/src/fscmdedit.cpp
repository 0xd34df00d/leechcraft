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

#include "fscmdedit.h"

fsCmdEdit::fsCmdEdit(QWidget * parent) : QLineEdit(parent)
{
	connect(this, SIGNAL(returnPressed()), this, SLOT(currentToHistory()));
}

void fsCmdEdit::keyReleaseEvent(QKeyEvent * event)
{
	if(event->key()==Qt::Key_Up)
	{
		if(++cmdIndex<cmdHistory.count())
		{
			if(cmdIndex==0)
				// add current line to history
				toHistory(text());
			setText(cmdHistory[cmdIndex]);
		}
		else
		{
			cmdIndex=-1;
			clear();
		}
	}
	else if(event->key()==Qt::Key_Down)
	{
		if(--cmdIndex>0)
			setText(cmdHistory[cmdIndex]);
		else
		{
			cmdIndex=-1;
			clear();
		}
	}
	QLineEdit::keyReleaseEvent(event);
}

void fsCmdEdit::currentToHistory()
{
	cmdIndex=-1;
	toHistory(text());
}

void fsCmdEdit::toHistory(QString entry)
{
	if(!entry.isEmpty())
	{
		cmdHistory.prepend(entry);
	}
}
