/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef VCARDDIALOG_H
#define VCARDDIALOG_H
#include <QDialog>
#include <QXmppAnnotationsIq.h>
#include <QXmppVCardIq.h>
#include "ui_vcarddialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class EntryBase;
	class GlooxAccount;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;
		GlooxAccount *Account_;
		QString JID_;
		QXmppAnnotationsIq::NoteItem Note_;

		QXmppVCardIq VCard_;
	public:
		VCardDialog (GlooxAccount*, QWidget* = 0);
		VCardDialog (EntryBase*, QWidget* = 0);

		void UpdateInfo (const QXmppVCardIq&);
	private:
		void BuildPhones (const QXmppVCardPhoneList&);
		void BuildEmails (const QXmppVCardEmailList&);
		void BuildAddresses (const QXmppVCardAddressList&);
		void InitConnections (EntryBase*);
		void EnableEditableMode ();
		void UpdateNote (GlooxAccount*, const QString&);
	private slots:
		void rebuildClientInfo ();
		void setNote ();
		void publishVCard ();
		void on_PhotoBrowse__released ();
		void on_PhotoClear__released ();
	};
}
}
}

#endif
