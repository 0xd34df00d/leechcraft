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

#include "vcarddialog.h"
#include <QPushButton>
#include <QFileDialog>
#include <QBuffer>
#include <QXmppVCardIq.h>
#include <QXmppVCardManager.h>
#include "entrybase.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "capsmanager.h"
#include "annotationsmanager.h"
#include "useravatarmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	VCardDialog::VCardDialog (GlooxAccount *acc, QWidget *parent)
	: QDialog (parent)
	, Account_ (acc)
	{
		Ui_.setupUi (this);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (setNote ()));

		Ui_.EditBirthday_->setVisible (false);
	}

	VCardDialog::VCardDialog (EntryBase *entry, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (setNote ()));

		GlooxAccount *account = qobject_cast<GlooxAccount*> (entry->GetParentAccount ());
		UpdateNote (account, entry->GetJID ());

		if (entry->GetJID () == account->GetJID ())
			EnableEditableMode ();
		else
		{
			Ui_.PhotoBrowse_->hide ();
			Ui_.PhotoClear_->hide ();
		}

		Ui_.EditBirthday_->setVisible (false);

		InitConnections (entry);
		rebuildClientInfo ();
	}

	void VCardDialog::UpdateInfo (const QXmppVCardIq& vcard)
	{
		VCard_ = vcard;

		const QString& forString = vcard.nickName ().isEmpty () ?
				vcard.from () :
				vcard.nickName ();
		setWindowTitle (tr ("VCard for %1").arg (forString));

		Ui_.EditJID_->setText (vcard.from ());
		Ui_.EditRealName_->setText (vcard.fullName ());
		Ui_.EditNick_->setText (vcard.nickName ());
		const QDate& date = vcard.birthday ();
		if (date.isValid ())
			Ui_.EditBirthday_->setDate (date);
		Ui_.EditBirthday_->setVisible (date.isValid ());

		QStringList phones;
		Q_FOREACH (const QXmppVCardPhone& phone, vcard.phones ())
		{
			if (phone.number.isEmpty ())
				continue;

			QStringList attrs;
			if (phone.isPref)
				attrs << tr ("preferred");
			if (phone.isHome)
				attrs << tr ("home");
			if (phone.isWork)
				attrs << tr ("work");
			if (phone.isCell)
				attrs << tr ("cell");

			phones << (attrs.isEmpty () ?
						phone.number :
						(phone.number + " (" + attrs.join (", ") + ")"));
		}
		Ui_.EditPhone_->setText (phones.join ("; "));

		QStringList emails;
		Q_FOREACH (const QXmppVCardEmail& email, vcard.emails ())
		{
			if (email.address.isEmpty ())
				continue;

			QStringList attrs;
			if (email.isPref)
				attrs << tr ("preferred");
			if (email.isHome)
				attrs << tr ("home");
			if (email.isWork)
				attrs << tr ("work");
			if (email.isX400)
				attrs << "X400";

			emails << (attrs.isEmpty () ?
						email.address :
						(email.address + " (" + attrs.join (", ") + ")"));
		}
		Ui_.EditEmail_->setText (emails.join ("; "));

		Ui_.EditURL_->setText (vcard.url ());

		QPixmap px = QPixmap::fromImage (QImage::fromData (vcard.photo ()));
		if (!px.isNull ())
		{
			const QSize& maxPx = Ui_.LabelPhoto_->maximumSize ();
			if (px.width () > maxPx.width () ||
					px.height () > maxPx.height ())
				px = px.scaled (maxPx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			Ui_.LabelPhoto_->setPixmap (px);
		}
		else
			Ui_.LabelPhoto_->setText (tr ("No photo"));

		Ui_.OrgName_->setText (vcard.orgName ());
		Ui_.OrgUnit_->setText (vcard.orgUnit ());
		Ui_.Title_->setText (vcard.title ());
		Ui_.Role_->setText (vcard.role ());
		Ui_.About_->setPlainText (vcard.desc ());
	}

	void VCardDialog::InitConnections (EntryBase *entry)
	{
		connect (entry,
				SIGNAL (statusChanged (const EntryStatus&, const QString&)),
				this,
				SLOT (rebuildClientInfo ()));
		connect (entry,
				SIGNAL (entryGenerallyChanged ()),
				this,
				SLOT (rebuildClientInfo ()));
	}

	void VCardDialog::rebuildClientInfo ()
	{
		if (!Account_)
			return;

		EntryBase *entry = qobject_cast<EntryBase*> (Account_->GetClientConnection ()->GetCLEntry (JID_));
		CapsManager *mgr = Account_->GetClientConnection ()->GetCapsManager ();

		QString html;
		Q_FOREACH (const QString& variant, entry->Variants ())
		{
			const auto& info = entry->GetClientInfo (variant);
			const QString& client = info ["raw_client_name"].toString ();

			html += "<strong>" + client + "</strong> (" +
					QString::number (info ["priority"].toInt ()) + ")<br />";

			const auto& version = entry->GetClientVersion (variant);
			auto gapp = [&html] (QString user, QString part)
			{
				if (!part.isEmpty ())
					html += user + ": " + part + "<br />";
			};
			gapp (tr ("Name"), version.name ());
			gapp (tr ("Version"), version.version ());
			gapp (tr ("OS"), version.os ());

			QStringList caps = mgr->GetCaps (entry->GetVariantVerString (variant));
			caps.sort ();
			if (caps.size ())
				html += "<strong>" + tr ("Capabilities") +
						"</strong>:<ul><li>" + caps.join ("</li><li>") + "</li></ul>";
		}

		Ui_.ClientInfo_->setHtml (html);
	}

	void VCardDialog::setNote ()
	{
		if (!Account_)
			return;

		Note_.setJid (JID_);
		Note_.setNote (Ui_.NotesEdit_->toPlainText ());
		Note_.setMdate (QDateTime::currentDateTime ());
		Account_->GetClientConnection ()->
				GetAnnotationsManager ()->SetNote (JID_, Note_);
	}

	void VCardDialog::publishVCard ()
	{
		VCard_.setFullName (Ui_.EditRealName_->text ());
		VCard_.setNickName (Ui_.EditNick_->text ());
		VCard_.setBirthday (Ui_.EditBirthday_->date ());
		VCard_.setUrl (Ui_.EditURL_->text ());
		VCard_.setOrgName (Ui_.OrgName_->text ());
		VCard_.setOrgUnit (Ui_.OrgUnit_->text ());
		VCard_.setTitle (Ui_.Title_->text ());
		VCard_.setRole (Ui_.Role_->text ());
		VCard_.setDesc (Ui_.About_->toPlainText ());

		const QPixmap *px = Ui_.LabelPhoto_->pixmap ();
		if (px)
		{
			QBuffer buffer;
			buffer.open (QIODevice::WriteOnly);
			px->save (&buffer, "PNG", 100);
			buffer.close ();
			VCard_.setPhoto (buffer.data ());
		}
		else
			VCard_.setPhoto (QByteArray ());

		Account_->GetClientConnection ()->GetUserAvatarManager ()->
					PublishAvatar (px ? px->toImage () : QImage ());

		QXmppVCardManager& mgr = Account_->GetClientConnection ()->
				GetClient ()->vCardManager ();
		mgr.setClientVCard (VCard_);
	}

	void VCardDialog::on_PhotoBrowse__released ()
	{
		const QString& fname = QFileDialog::getOpenFileName (this,
				tr ("Choose new photo"),
				QDir::homePath (),
				tr ("Images (*.png *.jpg *.gif);;All files (*.*)"));
		if (fname.isEmpty ())
			return;

		QPixmap px (fname);
		if (px.isNull ())
			return;

		const int size = 150;
		if (std::max (px.size ().width (), px.size ().height ()) > size)
			px = px.scaled (size, size,
					Qt::KeepAspectRatio, Qt::SmoothTransformation);

		Ui_.LabelPhoto_->setPixmap (px);
	}

	void VCardDialog::on_PhotoClear__released ()
	{
		Ui_.LabelPhoto_->clear ();
	}

	void VCardDialog::EnableEditableMode ()
	{
		Ui_.ButtonBox_->setStandardButtons (QDialogButtonBox::Save |
				QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
		connect (Ui_.ButtonBox_->button (QDialogButtonBox::Save),
				SIGNAL (released ()),
				this,
				SLOT (publishVCard ()));

		Q_FOREACH (QLineEdit *edit, findChildren<QLineEdit*> ())
			edit->setReadOnly (false);

		Ui_.About_->setReadOnly (false);
	}

	void VCardDialog::UpdateNote (GlooxAccount *acc, const QString& jid)
	{
		if (!acc)
			return;

		Account_ = acc;
		JID_ = jid;
		Note_ = acc->GetClientConnection ()->
				GetAnnotationsManager ()->GetNote (jid);
		Ui_.NotesEdit_->setPlainText (Note_.note ());

		rebuildClientInfo ();

		QObject *entryObj = acc->GetClientConnection ()->GetCLEntry (jid);
		InitConnections (qobject_cast<EntryBase*> (entryObj));
	}
}
}
}
