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

#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>

#include "connectiondialog.h"

ConnectionDialog::ConnectionDialog (QWidget *parent)
	: QDialog (parent)
{
	serverLabel = new QLabel (tr ("IRC server"), this);

	serverEdit = new QComboBox (this);
	serverEdit->setEditable (true);

	roomLabel = new QLabel (tr ("Room"), this);

	roomEdit = new QComboBox (this);
	roomEdit->setEditable (true);

	nickLabel = new QLabel (tr ("Nick"), this);

	nickEdit = new QComboBox (this);
	nickEdit->setEditable (true);

	encodingLabel = new QLabel (tr ("Encoding"), this);

	encodingEdit = new QComboBox (this);
	encodingEdit->setEditable (true);

	QGridLayout *layout = new QGridLayout ();
	layout->addWidget (serverLabel, 0, 0);
	layout->addWidget (serverEdit, 0, 1);
	layout->addWidget (roomLabel, 1, 0);
	layout->addWidget (roomEdit, 1, 1);
	layout->addWidget (nickLabel, 2, 0);
	layout->addWidget (nickEdit, 2, 1);
	layout->addWidget (encodingLabel, 3, 0);
	layout->addWidget (encodingEdit, 3, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
				Qt::Horizontal,
				this);
	connect (buttonBox, SIGNAL (accepted ()), this, SLOT (accept ()));
	connect (buttonBox, SIGNAL (rejected ()), this, SLOT (reject ()));

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->addLayout (layout);
	mainLayout->addWidget (buttonBox);

	setLayout (mainLayout);
}
