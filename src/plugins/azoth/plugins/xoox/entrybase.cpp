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
#include <QXmppVCardIq.h>
#include <QXmppPresence.h>
#include <plugininterface/util.h>
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "vcarddialog.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "util.h"
#include "core.h"
#include <QXmppClient.h>
#include <QXmppRosterManager.h>

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
		if (CurrentStatus_.contains (variant))
			return CurrentStatus_ [variant];

		const GlooxCLEntry *entry = qobject_cast<const GlooxCLEntry*> (this);
		QXmppRosterManager& rm = Account_->
				GetClientConnection ()->GetClient ()->rosterManager ();
		if (entry && rm.isRosterReceived ())
		{
			QList<QXmppPresence> press = rm.getAllPresencesForBareJid (GetJID ()).values ();
			if (press.size ())
			{
				QXmppPresence max = press.first ();
				Q_FOREACH (const QXmppPresence& pres, press)
					if (pres.status ().priority () > max.status ().priority ())
						max = pres;
				return EntryStatus (static_cast<State> (max.status ().type ()),
						max.status ().statusText ());
			}
		}
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
		if (Account_->GetState ().State_ == SOffline)
		{
			Entity e = LeechCraft::Util::MakeNotification ("Azoth",
					tr ("Can't view info while offline"),
					PCritical_);
			Core::Instance ().SendEntity (e);

			return;
		}

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

	void EntryBase::SetAvatar (const QByteArray& data)
	{
		if (!data.size ())
			SetAvatar (QImage ());
		else
			SetAvatar (QImage::fromData (data));
	}

	void EntryBase::SetAvatar (const QImage& avatar)
	{
		Avatar_ = avatar;

		emit avatarChanged (Avatar_);
	}

	void EntryBase::SetVCard (const QXmppVCardIq& vcard)
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

	void EntryBase::SetClientInfo (const QString& variant, const QXmppPresence& pres)
	{
		/*
		const QString& client = pres.capabilityNode ();
		const QString& ver = pres.capabilityVer ();
		SetClientInfo (variant, client, ver);
		*/
	}

	QString EntryBase::FormatRawInfo (const QXmppVCardIq& vcard)
	{
		QString text;
		text += tr ("Name: %1")
				.arg (vcard.fullName ());
		text += "\n";

		if (vcard.nickName ().size ())
			text += tr ("Nickname: %1\n")
					.arg (vcard.nickName ());
		if (vcard.url ().size ())
			text += tr ("URL: %1\n")
					.arg (vcard.url ());
		if (vcard.birthday ().isValid ())
			text += tr ("Birthday: %1\n")
					.arg (vcard.birthday ().toString ());
		if (vcard.email ().size ())
			text += tr ("Email: %1\n")
					.arg (vcard.email ());

		if (vcard.photoType ().size ())
		{
			text += tr ("Photo:") + QString ("\ndata:%1;base64,%2\n")
						.arg (vcard.photoType ())
						.arg (vcard.photo ().constData ());
		}

		return text;
	}
}
}
}
}
}
