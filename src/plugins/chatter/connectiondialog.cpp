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
#include <QtGui/QMessageBox>
#include <QtGui/QLineEdit>

#include "connectiondialog.h"
#include "fsettings.h"

ConnectionDialog::ConnectionDialog (QWidget *parent)
	: QDialog (parent)
{
	fSettings settings;

	uriLabel = new QLabel (tr ("IRC URI"), this);
	uriLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	uriEdit = new QLineEdit (this);
	connect (uriEdit, SIGNAL (textChanged (QString)), this, SLOT (uriChanged (QString)));

	serverLabel = new QLabel (tr ("IRC server"), this);
	serverLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	serverEdit = new QComboBox (this);
	serverEdit->setEditable (true);
	serverEdit->addItems(settings.value("Connection/Servers", QStringList()).toStringList());
	serverEdit->setCurrentIndex (-1);
	connect (serverEdit, SIGNAL (editTextChanged (QString)), this, SLOT (serverChanged ()));
	connect (serverEdit, SIGNAL (currentIndexChanged (QString)), this, SLOT (serverChanged ()));

	roomLabel = new QLabel (tr ("Room"), this);
	roomLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	roomEdit = new QComboBox (this);
	roomEdit->setEditable (true);
	roomEdit->setEnabled (false);

	nickLabel = new QLabel (tr ("Nick"), this);
	nickLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	nickEdit = new QComboBox (this);
	nickEdit->setEditable (true);
	nickEdit->setEnabled (false);

	encodingLabel = new QLabel (tr ("Encoding"), this);
	encodingLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	encodingEdit = new QComboBox (this);
	encodingEdit->setEditable (true);
	encodingEdit->setEnabled (false);

	QGridLayout *layout = new QGridLayout ();
	layout->addWidget (uriLabel, 0, 0);
	layout->addWidget (uriEdit, 0, 1);
	layout->addWidget (serverLabel, 1, 0);
	layout->addWidget (serverEdit, 1, 1);
	layout->addWidget (roomLabel, 2, 0);
	layout->addWidget (roomEdit, 2, 1);
	layout->addWidget (nickLabel, 3, 0);
	layout->addWidget (nickEdit, 3, 1);
	layout->addWidget (encodingLabel, 4, 0);
	layout->addWidget (encodingEdit, 4, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
				Qt::Horizontal,
				this);
	connect (buttonBox, SIGNAL (accepted ()), this, SLOT (saveAndAccept ()));
	connect (buttonBox, SIGNAL (rejected ()), this, SLOT (reject ()));

	QVBoxLayout *mainLayout = new QVBoxLayout ();
	mainLayout->addLayout (layout);
	mainLayout->addWidget (buttonBox);

	setLayout (mainLayout);

	serverEdit->setCurrentIndex(settings.value("Connection/LastServer", -1).toInt ());
}

QString ConnectionDialog::server ()
{
	return serverEdit->currentText ();
}

void ConnectionDialog::setServer (const QString& server)
{
	serverEdit->setEditText (server);
}

QString ConnectionDialog::room ()
{
	return roomEdit->currentText ();
}

void ConnectionDialog::setRoom (const QString& room)
{
	roomEdit->setEditText (room);
}

QString ConnectionDialog::nick ()
{
	return nickEdit->currentText ();
}

void ConnectionDialog::setNick (const QString& nick)
{
	nickEdit->setEditText (nick);
}

QString ConnectionDialog::encoding ()
{
	return encodingEdit->currentText ();
}

void ConnectionDialog::setEncoding (const QString& encoding)
{
	encodingEdit->setEditText (encoding);
}

void ConnectionDialog::serverChanged ()
{
	roomEdit->clear();
	nickEdit->clear();
	encodingEdit->clear();

	fSettings settings;

	roomEdit->addItems(settings.value("Connection/Rooms_" + serverEdit->currentText (), QStringList()).toStringList());
	roomEdit->setCurrentIndex(settings.value("Connection/LastRoom_" + serverEdit->currentText (), -1).toInt ());
	nickEdit->addItems(settings.value("Connection/Nicks_" + serverEdit->currentText (), QStringList()).toStringList());
	nickEdit->setCurrentIndex(settings.value("Connection/LastNick_" + serverEdit->currentText (), -1).toInt ());
	encodingEdit->addItems(settings.value("Connection/Encodings_" + serverEdit->currentText (), QStringList()).toStringList());
	encodingEdit->setCurrentIndex(settings.value("Connection/LastEncoding_" + serverEdit->currentText (), -1).toInt ());

	roomEdit->setEnabled (!serverEdit->currentText ().isEmpty ());
	nickEdit->setEnabled (!serverEdit->currentText ().isEmpty ());
	encodingEdit->setEnabled (!serverEdit->currentText ().isEmpty ());
}

void ConnectionDialog::saveAndAccept ()
{
	if (serverEdit->currentText ().isEmpty ()
		|| roomEdit->currentText ().isEmpty ()
		|| nickEdit->currentText ().isEmpty ()
		|| encodingEdit->currentText ().isEmpty ()) {

		QMessageBox::critical (this, "", tr ("Parameters is not valid"));
		return;
	}

	fSettings settings;
	if (serverEdit->findText (serverEdit->currentText (), Qt::MatchFixedString) == -1) {
		QStringList l = settings.value("Connection/Servers", QStringList ()).toStringList ();
		l.insert(0, serverEdit->currentText ());
		settings.setValue("Connection/Servers", l);
	}
	if (roomEdit->findText (roomEdit->currentText (), Qt::MatchFixedString) == -1) {
		QStringList l = settings.value("Connection/Rooms_" + serverEdit->currentText (), QStringList ()).toStringList ();
		l.insert(0, roomEdit->currentText ());
		settings.setValue("Connection/Rooms_" + serverEdit->currentText (), l);
	}
	if (nickEdit->findText (nickEdit->currentText (), Qt::MatchFixedString) == -1) {
		QStringList l = settings.value("Connection/Nicks_" + serverEdit->currentText (), QStringList ()).toStringList ();
		l.insert(0, nickEdit->currentText ());
		settings.setValue("Connection/Nicks_" + serverEdit->currentText (), l);
	}
	if (encodingEdit->findText (encodingEdit->currentText (), Qt::MatchFixedString) == -1) {
		QStringList l = settings.value("Connection/Encodings_" + serverEdit->currentText (), QStringList ()).toStringList ();
		l.insert(0, encodingEdit->currentText ());
		settings.setValue("Connection/Encodings_" + serverEdit->currentText (), l);
	}

	settings.setValue("Connection/LastServer", serverEdit->currentIndex ());
	settings.setValue("Connection/LastRoom_" + serverEdit->currentText (), roomEdit->currentIndex ());
	settings.setValue("Connection/LastNick_" + serverEdit->currentText (), nickEdit->currentIndex ());
	settings.setValue("Connection/LastEncoding_" + serverEdit->currentText (), encodingEdit->currentIndex ());

	accept ();
}

void ConnectionDialog::uriChanged (const QString& uri)
{
	static QRegExp serverRegExp("^irc://([a-zA-Z0-9\\.\\-]+)/(\\S+)$");

	if (!serverRegExp.exactMatch (uri)) {
		serverEdit->setCurrentIndex (-1);
		return;
	}

	const QString& server = serverRegExp.cap (1);
	const QString& room = serverRegExp.cap (2);

	int index = serverEdit->findText(server, Qt::MatchFixedString);

	if (index >= 0)
		serverEdit->setCurrentIndex(index);
	else
		serverEdit->setEditText(server);

	index = roomEdit->findText(room, Qt::MatchFixedString);

	if (index >= 0)
		roomEdit->setCurrentIndex(index);
	else
		roomEdit->setEditText(room);
}
