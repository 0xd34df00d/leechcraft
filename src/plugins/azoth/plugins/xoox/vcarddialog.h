/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
