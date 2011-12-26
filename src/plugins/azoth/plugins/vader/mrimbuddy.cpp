/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "mrimbuddy.h"
#include <QImage>
#include "proto/headers.h"
#include "mrimaccount.h"
#include "mrimmessage.h"
#include <interfaces/azothutil.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	MRIMBuddy::MRIMBuddy (const Proto::ContactInfo& info, MRIMAccount *acc)
	: QObject (acc)
	, A_ (acc)
	, Info_ (info)
	{
		if (info.StatusID_ == Proto::UserState::Online)
			Status_.State_ = SOnline;
		else if (info.StatusID_ == Proto::UserState::Away)
			Status_.State_ = SAway;
		else
			Status_.State_ = SOffline;
	}

	void MRIMBuddy::HandleMessage (MRIMMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	QObject* MRIMBuddy::GetObject ()
	{
		return this;
	}

	QObject* MRIMBuddy::GetParentAccount () const
	{
		return A_;
	}

	ICLEntry::Features MRIMBuddy::GetEntryFeatures () const
	{
		return FPermanentEntry;
	}

	ICLEntry::EntryType MRIMBuddy::GetEntryType () const
	{
		return ETChat;
	}

	QString MRIMBuddy::GetEntryName () const
	{
		return Info_.Alias_.isEmpty () ?
				Info_.Email_ :
				Info_.Alias_;
	}

	void MRIMBuddy::SetEntryName (const QString&)
	{
	}

	QString MRIMBuddy::GetEntryID () const
	{
		return A_->GetAccountID () + "_" + Info_.Email_;
	}

	QString MRIMBuddy::GetHumanReadableID () const
	{
		return Info_.Email_;
	}

	QStringList MRIMBuddy::Groups () const
	{
		return QStringList ();
	}

	void MRIMBuddy::SetGroups (const QStringList&)
	{
	}

	QStringList MRIMBuddy::Variants() const
	{
		return Status_.State_ != SOffline ?
				QStringList (QString ()) :
				QStringList ();
	}

	QObject* MRIMBuddy::CreateMessage (IMessage::MessageType , const QString& , const QString&)
	{
		return 0;
	}

	QList<QObject*> MRIMBuddy::GetAllMessages () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto m, AllMessages_)
			result << m;
		return result;
	}

	void MRIMBuddy::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
	}

	void MRIMBuddy::SetChatPartState (ChatPartState , const QString&)
	{
	}

	EntryStatus MRIMBuddy::GetStatus (const QString&) const
	{
		return Status_;
	}

	QImage MRIMBuddy::GetAvatar () const
	{
		return QImage ();
	}

	QString MRIMBuddy::GetRawInfo () const
	{
		return QString ();
	}

	void MRIMBuddy::ShowInfo ()
	{
	}

	QList<QAction*> MRIMBuddy::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> MRIMBuddy::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void MRIMBuddy::MarkMsgsRead ()
	{
	}
}
}
}
