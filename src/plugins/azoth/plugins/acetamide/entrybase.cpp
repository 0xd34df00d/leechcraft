/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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
#include <QAction>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
#include "clientconnection.h"
#include "ircprotocol.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "vcarddialog.h"
#include "ircparticipantentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	EntryBase::EntryBase (IrcAccount *account)
	: QObject (account)
	, Account_ (account)
	, VCardDialog_ (0)
	{
	}

	EntryBase::~EntryBase ()
	{
		qDeleteAll (AllMessages_);
		qDeleteAll (Actions_);
		delete VCardDialog_;
	}

	QObject* EntryBase::GetObject ()
	{
		return this;
	}

	QList<QObject*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}

	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
	}

	void EntryBase::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus EntryBase::GetStatus (const QString&) const
	{
		return CurrentStatus_;
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		return Actions_;
	}

	QImage EntryBase::GetAvatar () const
	{
		return QImage ();
	}

	QString EntryBase::GetRawInfo () const
	{
		return QString ();
	}

	void EntryBase::ShowInfo ()
	{
		IrcParticipantEntry *entry = qobject_cast<IrcParticipantEntry*> (this);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< this
					<< "is not an IrcParticipantEntry object";
			return;
		}

		if (!VCardDialog_)
			VCardDialog_ = new VCardDialog (this);

		Account_->GetClientConnection ()->FetchVCard (entry->GetServerID (),
				entry->GetEntryName());
		VCardDialog_->show ();
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void EntryBase::MarkMsgsRead ()
	{
	}

	void EntryBase::HandleMessage (IrcMessage *msg)
	{
		msg->SetOtherPart (this);
		IrcProtocol *proto = qobject_cast<IrcProtocol*> (Account_->
				GetParentProtocol ());
		IProxyObject *proxy =
				qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		proxy->PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::SetStatus (const EntryStatus& status)
	{
		CurrentStatus_ = status;
		emit statusChanged (CurrentStatus_, QString ());
	}

	void EntryBase::SetAvatar (const QByteArray&)
	{
	}

	void EntryBase::SetAvatar (const QImage&)
	{
	}

	void EntryBase::SetRawInfo (const QString&)
	{
	}

	void EntryBase::SetInfo (const WhoIsMessage& msg)
	{
		if (VCardDialog_)
			VCardDialog_->UpdateInfo (msg);
	}

};
};
};
