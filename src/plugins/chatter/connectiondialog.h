/***************************************************************************
 *   Copyright (C) 2010 by PanteR   *
 *   panter.dsd@gmail.com   *
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
#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

class QLabel;
class QComboBox;

#include <QtGui/QDialog>

class ConnectionDialog : public QDialog
{
	Q_OBJECT

private:
	QLabel *serverLabel;
	QComboBox *serverEdit;
	QLabel *roomLabel;
	QComboBox *roomEdit;
	QLabel *nickLabel;
	QComboBox *nickEdit;
	QLabel *encodingLabel;
	QComboBox *encodingEdit;

public:
	ConnectionDialog (QWidget *parent = 0);
	virtual ~ConnectionDialog()
	{}
};

#endif //CONNECTIONDIALOG_H
