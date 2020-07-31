/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QXmppVCardIq.h>
#include "ui_vcarddialog.h"
#include "xeps/xmppannotationsiq.h"

namespace LC::Azoth::Xoox
{
	class EntryBase;
	class GlooxAccount;

	typedef QList<QXmppVCardPhone> QXmppVCardPhoneList;
	typedef QList<QXmppVCardEmail> QXmppVCardEmailList;
	typedef QList<QXmppVCardAddress> QXmppVCardAddressList;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;
		GlooxAccount *Account_;
		QString JID_;
		XMPPAnnotationsIq::NoteItem Note_;

		QXmppVCardIq VCard_;

		bool PhotoChanged_ = false;

		QPixmap ShownPixmap_;
	public:
		explicit VCardDialog (GlooxAccount*, QWidget* = nullptr);
		explicit VCardDialog (EntryBase*, QWidget* = nullptr);

		void UpdateInfo (const QXmppVCardIq&);

		bool eventFilter (QObject*, QEvent*) override;
	private:
		void SetPixmapLabel (QPixmap);

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
		void on_PhoneButton__released ();
		void on_EmailButton__released ();
		void on_PhotoBrowse__released ();
		void on_PhotoClear__released ();
	};
}
