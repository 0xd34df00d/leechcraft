/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "entrybase.h"
#include <QImage>
#include <QStringList>
#include <QtDebug>
#include <gloox/rosteritem.h>
#include <gloox/capabilities.h>
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "vcarddialog.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "util.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	EntryBase::EntryBase (GlooxAccount *parent)
	: QObject (parent)
	, Account_ (parent)
	{

	}

	QObject* EntryBase::GetObject ()
	{
		return this;
	}

	QList<QObject*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}

	EntryStatus EntryBase::GetStatus (const QString& variant) const
	{
		const GlooxCLEntry *entry = qobject_cast<const GlooxCLEntry*> (this);
		const gloox::Resource *hr = 0;
		if (entry && entry->GetRI ())
			hr = entry->GetRI ()->highestResource ();
		if (CurrentStatus_.contains (variant))
			return CurrentStatus_ [variant];
		else if (hr)
			return EntryStatus (static_cast<State> (hr->presence ()),
					QString::fromUtf8 (hr->message ().c_str ()));
		else if (CurrentStatus_.size ())
			return *CurrentStatus_.begin ();
		else
			return EntryStatus ();
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		return Actions_;
	}

	QImage EntryBase::GetAvatar () const
	{
		return Avatar_;
	}

	QString EntryBase::GetRawInfo () const
	{
		return RawInfo_;
	}

	void EntryBase::ShowInfo ()
	{
		if (!VCardDialog_)
			VCardDialog_ = new VCardDialog ();

		Account_->GetClientConnection ()->FetchVCard (GetJID ());;
		VCardDialog_->show ();
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString& var) const
	{
		return Variant2ClientInfo_ [var];
	}

	void EntryBase::HandleMessage (GlooxMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::SetStatus (const EntryStatus& status, const QString& variant)
	{
		if (CurrentStatus_.contains (variant) &&
				status == CurrentStatus_ [variant])
			return;

		CurrentStatus_ [variant] = status;
		emit statusChanged (status, variant);
	}

	void EntryBase::SetAvatar (const gloox::VCard::Photo& photo)
	{
		if (!photo.type.size ())
			SetAvatar (QImage ());
		else
		{
			QByteArray data (photo.binval.c_str(), photo.binval.size ());
			SetAvatar (QImage::fromData (data));
		}
	}

	void EntryBase::SetAvatar (const QImage& avatar)
	{
		Avatar_ = avatar;

		emit avatarChanged (Avatar_);
	}

	void EntryBase::SetVCard (const gloox::VCard* vcard)
	{
		QString text = FormatRawInfo (vcard);
		if (!text.isEmpty ())
			text = QString ("gloox VCard:\n") + text;
		SetRawInfo (text);

		if (VCardDialog_)
			VCardDialog_->UpdateInfo (vcard);
	}

	void EntryBase::SetRawInfo (const QString& rawinfo)
	{
		RawInfo_ = rawinfo;

		emit rawinfoChanged (RawInfo_);
	}

	void EntryBase::SetClientInfo (const QString& variant,
			const QString& node, const QString& ver)
	{
		QString type = Util::GetClientIDName (node);
		if (type.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown client type for"
					<< node;
			type = "unknown";
		}
		Variant2ClientInfo_ [variant] ["client_type"] = type;

		QString name = Util::GetClientHRName (node);
		if (name.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown client name for"
					<< node;
			name = "Unknown";
		}
		Variant2ClientInfo_ [variant] ["client_name"] = name;
	}

	void EntryBase::SetClientInfo (const QString& variant, const gloox::Capabilities *caps)
	{
		const QString& client = QString::fromUtf8 (caps->node ().c_str ());
		const QString& ver = QString::fromUtf8 (caps->ver ().c_str ());
		SetClientInfo (variant, client, ver);
	}

	QString EntryBase::FormatRawInfo (const gloox::VCard *vcard)
	{
		QString text;
		text += tr ("Name:");
		text += VCardDialog::GetName (vcard);
		text += "\n";

		if (vcard->nickname ().size ())
			text += tr ("Nickname: %1\n")
					.arg (QString::fromUtf8 (vcard->nickname ().c_str ()));
		if (vcard->url ().size ())
			text += tr ("URL: %1\n")
					.arg (QString::fromUtf8 (vcard->url ().c_str ()));
		if (vcard->bday ().size ())
			text += tr ("Burthday: %1\n")
					.arg (QString::fromUtf8 (vcard->bday ().c_str ()));
		if (vcard->title ().size ())
			text += tr ("Title: %1\n")
					.arg (QString::fromUtf8 (vcard->title ().c_str ()));
		if (vcard->role ().size ())
			text += tr ("Role: %1\n")
					.arg (QString::fromUtf8 (vcard->role ().c_str ()));
		if (vcard->note ().size ())
			text += tr ("Note: %1\n")
					.arg (QString::fromUtf8 (vcard->note ().c_str ()));
		if (vcard->desc ().size ())
			text += tr ("Description: %1\n")
					.arg (QString::fromUtf8 (vcard->desc ().c_str ()));
		if (vcard->tz ().size ())
			text += tr ("Timezone: %1\n")
					.arg (QString::fromUtf8 (vcard->tz ().c_str ()));
		if (vcard->mailer ().size ())
			text += tr ("Mailer: %1\n")
					.arg (QString::fromUtf8 (vcard->mailer ().c_str ()));
		if (vcard->rev ().size ())
			text += tr ("Rev: %1\n")
				.arg (QString::fromUtf8 (vcard->rev ().c_str ()));
		if (vcard->uid ().size ())
			text += tr ("UID: %1\n")
					.arg (QString::fromUtf8 (vcard->uid ().c_str ()));

		// Bug in gloox: emailAddresses should be const
		const gloox::VCard::EmailList& emails =
				const_cast<gloox::VCard*> (vcard)->emailAddresses ();
		Q_FOREACH (const gloox::VCard::Email& email, emails)
		{
			text += QString::fromUtf8 (email.userid.c_str ());
			if (email.home || email.work || email.internet || email.pref || email.x400)
			{
				text += QString ("[");

				QStringList cats;
				if (email.home)
					cats << tr ("Home");
				if (email.work)
					cats << tr (" Work");
				if (email.internet)
					cats << tr ("Internet");
				if (email.pref)
					cats << tr ("Preferred");
				if (email.x400)
					cats << tr ("X.400");

				text += QString ("[%1]")
						.arg (cats.join ("; "));
			}
			text += "\n";
		}
		// TODO: Org, Label Address, Address,
		if (vcard->geo ().latitude.size () || vcard->geo ().longitude.size ())
		{
			text += tr ("Latitude: %1\n")
					.arg (QString::fromUtf8 (vcard->geo ().latitude.c_str ()));
			text += tr ("Longitude: %1\n")
				.arg (QString::fromUtf8 (vcard->geo ().longitude.c_str ()));
		}

		if (vcard->photo ().type.size ())
		{
			gloox::VCard::Photo photo = vcard->photo ();
			QByteArray data (photo.binval.c_str(), photo.binval.size ());
			text += tr ("Photo:") + QString ("\ndata:%1;base64,%2\n")
						.arg (photo.type .c_str ())
						.arg (data.toBase64 ().constData ());
		}

		if (vcard->logo ().type.size ())
		{
			gloox::VCard::Photo photo = vcard->photo ();
			QByteArray data (photo.binval.c_str(), photo.binval.size ());
			text += tr ("Photo:") + QString ("\ndata:%1;base64,%2\n")
						.arg (photo.type .c_str ())
						.arg (data.toBase64 ().constData ());
		}

		return text;
	}
}
}
}
}
}
