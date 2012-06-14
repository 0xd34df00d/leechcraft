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
#include <QXmppVCardManager.h>
#include "entrybase.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "capsmanager.h"
#include "annotationsmanager.h"
#include "useravatarmanager.h"
#include "vcardlisteditdialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	VCardDialog::VCardDialog (GlooxAccount *acc, QWidget *parent)
	: QDialog (parent)
	, Account_ (acc)
	, PhotoChanged_ (false)
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
	, PhotoChanged_ (false)
	{
		Ui_.setupUi (this);
		Ui_.EditJID_->setText (entry->GetJID ());
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
			Ui_.PhoneButton_->hide ();
			Ui_.EmailButton_->hide ();
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

		BuildPhones (vcard.phones ());
		BuildEmails (vcard.emails ());
		BuildAddresses (vcard.addresses ());

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

	void VCardDialog::BuildPhones (const QXmppVCardPhoneList& phonesList)
	{
		QStringList phones;
		Q_FOREACH (const QXmppVCardPhone& phone, phonesList)
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
	}

	void VCardDialog::BuildEmails (const QXmppVCardEmailList& emailsList)
	{
		QStringList emails;
		Q_FOREACH (const QXmppVCardEmail& email, emailsList)
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
	}

	void VCardDialog::BuildAddresses (const QXmppVCardAddressList& addressList)
	{
		QStringList addresses;
		int addrNum = 1;
		Q_FOREACH (const QXmppVCardAddress& address, addressList)
		{
			if (address.isEmpty ())
				continue;

			QStringList attrs;
			if (address.isHome)
				attrs << tr ("home");
			if (address.isWork)
				attrs << tr ("work");
			if (address.isPostal)
				attrs << tr ("postal");
			if (address.isPref)
				attrs << tr ("preferred");

			QString str;
			str += "<strong>";
			str += attrs.isEmpty () ?
					tr ("Address %1:")
						.arg (addrNum) :
					tr ("Address %1 (%2):")
						.arg (addrNum)
						.arg (attrs.join (", "));
			str += "</strong>";

			QStringList fields;
			auto addField = [&fields] (const QString& label, const QString& val)
			{
				if (!val.isEmpty ())
					fields << label.arg (val);
			};
			addField (tr ("Country: %1"), address.country);
			addField (tr ("Region: %1"), address.region);
			addField (tr ("Locality: %1", "User's locality"), address.locality);
			addField (tr ("Street: %1"), address.street);
			addField (tr ("Additional: %1", "Additional address in user's address"), address.extAdd);
			addField (tr ("Postal code: %1"), address.pCode);
			str += "<ul><li>";
			str += fields.join ("</li><li>");
			str += "</li></ul>";

			addresses << str;
		}
		Ui_.Addresses_->setHtml (addresses.join ("<hr/>"));
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
		if (!entry)
			return;

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
		VCard_.setEmail (QString ());

		const QPixmap *px = Ui_.LabelPhoto_->pixmap ();
		if (px)
		{
			QBuffer buffer;
			buffer.open (QIODevice::WriteOnly);
			px->save (&buffer, "PNG", 100);
			buffer.close ();
			VCard_.setPhoto (buffer.data ());
			if (PhotoChanged_)
				Account_->UpdateOurPhotoHash (QCryptographicHash::hash (buffer.data (), QCryptographicHash::Sha1));
		}
		else
		{
			VCard_.setPhoto (QByteArray ());
			if (PhotoChanged_)
				Account_->UpdateOurPhotoHash ("");
		}

		if (PhotoChanged_)
			Account_->GetClientConnection ()->GetUserAvatarManager ()->
						PublishAvatar (px ? px->toImage () : QImage ());
		PhotoChanged_ = false;

		QXmppVCardManager& mgr = Account_->GetClientConnection ()->
				GetClient ()->vCardManager ();
		mgr.setClientVCard (VCard_);
	}

	void VCardDialog::on_PhoneButton__released ()
	{
		QStringList options;
		options << tr ("preferred")
				<< tr ("home")
				<< tr ("work")
				<< tr ("cell");

		VCardListEditDialog dia (options, this);
		dia.setWindowTitle (tr ("VCard phones"));
		Q_FOREACH (const QXmppVCardPhone& phone, VCard_.phones ())
		{
			if (phone.number.isEmpty ())
				continue;

			QPair<QString, QStringList> pair;
			pair.first = phone.number;
			if (phone.isPref)
				pair.second << options.at (0);
			if (phone.isHome)
				pair.second << options.at (1);
			if (phone.isWork)
				pair.second << options.at (2);
			if (phone.isCell)
				pair.second << options.at (3);
			dia.AddItems (QList<decltype (pair)> () << pair);
		}

		if (dia.exec () != QDialog::Accepted)
			return;

		QXmppVCardPhoneList list;
		Q_FOREACH (const auto& item, dia.GetItems ())
		{
			QXmppVCardPhone phone;
			phone.number = item.first;
			phone.isPref = item.second.contains (options.at (0));
			phone.isHome = item.second.contains (options.at (1));
			phone.isWork = item.second.contains (options.at (2));
			phone.isCell = item.second.contains (options.at (3));
			list << phone;
		}
		VCard_.setPhones (list);
		BuildPhones (list);
	}

	void VCardDialog::on_EmailButton__released ()
	{
		QStringList options;
		options << tr ("preferred")
				<< tr ("home")
				<< tr ("work")
				<< "X400";

		VCardListEditDialog dia (options, this);
		dia.setWindowTitle (tr ("VCard emails"));
		Q_FOREACH (const QXmppVCardEmail& email, VCard_.emails ())
		{
			if (email.address.isEmpty ())
				continue;

			QPair<QString, QStringList> pair;
			pair.first = email.address;
			if (email.isPref)
				pair.second << options.at (0);
			if (email.isHome)
				pair.second << options.at (1);
			if (email.isWork)
				pair.second << options.at (2);
			if (email.isX400)
				pair.second << options.at (3);
			dia.AddItems (QList<decltype (pair)> () << pair);
		}

		if (dia.exec () != QDialog::Accepted)
			return;

		QXmppVCardEmailList list;
		Q_FOREACH (const auto& item, dia.GetItems ())
		{
			QXmppVCardEmail email;
			email.address = item.first;
			email.isPref = item.second.contains (options.at (0));
			email.isHome = item.second.contains (options.at (1));
			email.isWork = item.second.contains (options.at (2));
			email.isX400 = item.second.contains (options.at (3));
			list << email;
		}
		VCard_.setEmails (list);
		BuildEmails (list);
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

		PhotoChanged_ = true;

		const int size = 150;
		if (std::max (px.size ().width (), px.size ().height ()) > size)
			px = px.scaled (size, size,
					Qt::KeepAspectRatio, Qt::SmoothTransformation);

		Ui_.LabelPhoto_->setPixmap (px);
	}

	void VCardDialog::on_PhotoClear__released ()
	{
		Ui_.LabelPhoto_->clear ();
		PhotoChanged_ = true;
	}

	void VCardDialog::EnableEditableMode ()
	{
		Ui_.ButtonBox_->setStandardButtons (QDialogButtonBox::Save |
				QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
		connect (Ui_.ButtonBox_->button (QDialogButtonBox::Save),
				SIGNAL (released ()),
				this,
				SLOT (publishVCard ()));

		auto toEnable = findChildren<QLineEdit*> ();
		toEnable.removeAll (Ui_.EditPhone_);
		toEnable.removeAll (Ui_.EditEmail_);
		Q_FOREACH (QLineEdit *edit, toEnable)
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
