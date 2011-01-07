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
#include "glooxmessage.h"
#include "glooxclentry.h"

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
	EntryBase::EntryBase (QObject* parent)
	: QObject (parent)
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

	void EntryBase::SetRawInfo (const gloox::VCard* vcard)
	{
		QString text;
		text += tr ("Name") + QString (":");
		gloox::VCard::Name name = vcard->name ();
		if (name.prefix.size ())
			text += QString (" ") + QString::fromUtf8 (name.prefix.c_str ());
		if (name.family.size ())
			text += QString (" ") + QString::fromUtf8 (name.family.c_str ());
		if (name.given.size ())
			text += QString (" ") + QString::fromUtf8 (name.given.c_str ());
		if (name.middle.size ())
			text += QString (" ") + QString::fromUtf8 (name.middle.c_str ());
		if (name.suffix.size ())
			text += QString (" ") + QString::fromUtf8 (name.prefix.c_str ());
		text += "\n";

		if (vcard->nickname ().size ())
			text += tr ("Nickname") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->nickname ().c_str ()));
		if (vcard->url ().size ())
			text += tr ("URL") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->url ().c_str ()));
		if (vcard->bday ().size ())
			text += tr ("Burn Day") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->bday ().c_str ()));
		if (vcard->title ().size ())
			text += tr ("Title") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->title ().c_str ()));
		if (vcard->role ().size ())
			text += tr ("Role") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->role ().c_str ()));
		if (vcard->note ().size ())
			text += tr ("Note") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->note ().c_str ()));
		if (vcard->desc ().size ())
			text += tr ("Description") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->desc ().c_str ()));
		if (vcard->tz ().size ())
			text += tr ("Timezone") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->tz ().c_str ()));
		if (vcard->mailer ().size ())
			text += tr ("Mailer") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->mailer ().c_str ()));
		if (vcard->rev ().size ())
			text += tr ("Rev") + QString (": %1\n")
				.arg (QString::fromUtf8 (vcard->rev ().c_str ()));
		if (vcard->uid ().size ())
			text += tr ("UID") + QString (": %1\n")
					.arg (QString::fromUtf8 (vcard->uid ().c_str ()));

		// Bug in gloox: emailAddresses should be const
		const gloox::VCard::EmailList& emails =
			const_cast<gloox::VCard*>(vcard)->emailAddresses();
		if (emails.empty())
		{
			foreach (gloox::VCard::Email email, emails)
			{
				text += QString::fromUtf8 (email.userid.c_str ());
				if (email.home || email.work || email.internet || email.pref || email.x400)
				{
					text += QString ("[");

					if (email.home)
						text += tr (" Home");
					if (email.work)
						text += tr (" Work");
					if (email.internet)
						text += tr (" Internet");
					if (email.pref)
						text += tr (" Preferred");
					if (email.x400)
						text += tr (" X.400");

					text += QString (" ]");
				}
				text += QString ("\n");
			}
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
			text += tr ("Photo") + QString (":\ndata:%1;base64,%2\n")
						.arg (photo.type .c_str ()) // "May be forged!" ?
						.arg (data.toBase64 ().constData ());
		}

		if (vcard->logo ().type.size ())
		{
			gloox::VCard::Photo photo = vcard->photo ();
			QByteArray data (photo.binval.c_str(), photo.binval.size ());
			text += tr ("Photo") + QString (":\ndata:%1;base64,%2\n")
						.arg (photo.type .c_str ()) // "May be forged!" ?
						.arg (data.toBase64 ().constData ());
		}

		if (!text.isEmpty ())
			text = QString ("***** gloox VCard *****\n") + text;
		SetRawInfo (text);
	}

	void EntryBase::SetRawInfo (const QString& rawinfo)
	{
		RawInfo_ = rawinfo;

		emit rawinfoChanged (RawInfo_);
	}
}
}
}
}
}
