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
#include <functional>
#include <QImage>
#include <interfaces/azothutil.h>
#include "proto/headers.h"
#include "mrimaccount.h"
#include "mrimmessage.h"
#include "vaderutil.h"
#include "groupmanager.h"
#include "proto/connection.h"

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
	, IsAuthorized_ (true)
	{
		Status_.State_ = VaderUtil::StatusID2State (info.StatusID_);
	}

	void MRIMBuddy::HandleMessage (MRIMMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void MRIMBuddy::HandleAttention (const QString& msg)
	{
		emit attentionDrawn (msg, QString ());
	}

	void MRIMBuddy::HandleTune (const QString& tune)
	{
		QVariantMap tuneMap;
		tuneMap ["artist"] = tune;
		ClientInfo_ ["user_tune"] = tuneMap;
		emit tuneChanged (QString ());
	}

	void MRIMBuddy::HandleCPS (ChatPartState cps)
	{
		emit chatPartStateChanged (cps, QString ());
	}

	void MRIMBuddy::SetGroup (const QString& group)
	{
		Info_.GroupNumber_ = A_->GetGroupManager ()->GetGroupNumber (group);
		Group_ = group;
		emit groupsChanged (Groups ());
	}

	void MRIMBuddy::SetAuthorized (bool auth)
	{
		if (auth == IsAuthorized_)
			return;

		IsAuthorized_ = auth;
		if (!IsAuthorized_)
			SetGroup (tr ("Unauthorized"));
		else
			SetGroup (QString ());
	}

	bool MRIMBuddy::IsAuthorized () const
	{
		return IsAuthorized_;
	}

	Proto::ContactInfo MRIMBuddy::GetInfo () const
	{
		return Info_;
	}

	namespace
	{
		template<typename T, typename V>
		void CmpXchg (Proto::ContactInfo& info, Proto::ContactInfo newInfo,
				T g, V n)
		{
			if (g (info) != g (newInfo))
			{
				g (info) = g (newInfo);
				n (g (info));
			}
		}

		template<typename T, typename U>
		std::function<T& (Proto::ContactInfo&)> GetMem (U g)
		{
			return g;
		}
	}

	void MRIMBuddy::UpdateInfo (const Proto::ContactInfo& info)
	{
		CmpXchg (Info_, info,
				GetMem<QString> (&Proto::ContactInfo::Alias_),
				[this] (QString name) { emit nameChanged (name); });
		CmpXchg (Info_, info,
				GetMem<QString> (&Proto::ContactInfo::UA_),
				[this] (QString) { emit entryGenerallyChanged (); });

		bool stChanged = false;
		const int oldVars = Variants ().size ();
		CmpXchg (Info_, info,
				GetMem<quint32> (&Proto::ContactInfo::StatusID_),
				[&stChanged] (quint32) { stChanged = true; });
		CmpXchg (Info_, info,
				GetMem<QString> (&Proto::ContactInfo::StatusTitle_),
				[&stChanged] (QString) { stChanged = true; });
		CmpXchg (Info_, info,
				GetMem<QString> (&Proto::ContactInfo::StatusDesc_),
				[&stChanged] (QString) { stChanged = true; });

		if (stChanged)
		{
			Status_.State_ = VaderUtil::StatusID2State (Info_.StatusID_);
			Status_.StatusString_ = Info_.StatusTitle_;

			if (oldVars != Variants ().size ())
				emit availableVariantsChanged (Variants ());
			emit statusChanged (GetStatus (QString ()), QString ());
		}

		Info_.GroupNumber_ = info.GroupNumber_;
	}

	qint64 MRIMBuddy::GetID () const
	{
		return Info_.ContactID_;
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
		return FPermanentEntry | FSupportsGrouping | FSupportsRenames;
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

	void MRIMBuddy::SetEntryName (const QString& name)
	{
		Info_.Alias_ = name;

		A_->GetConnection ()->ModifyContact (GetID (),
				Info_.GroupNumber_, Info_.Email_, name);
		emit nameChanged (name);
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
		QStringList result;
		if (!Group_.isEmpty ())
			result << Group_;
		return result;
	}

	void MRIMBuddy::SetGroups (const QStringList& list)
	{
		A_->GetGroupManager ()->SetBuddyGroups (this, list);
	}

	QStringList MRIMBuddy::Variants () const
	{
		return Status_.State_ != SOffline ?
				QStringList (QString ()) :
				QStringList ();
	}

	QObject* MRIMBuddy::CreateMessage (IMessage::MessageType type,
			const QString&, const QString& body)
	{
		MRIMMessage *msg = new MRIMMessage (IMessage::DOut, IMessage::MTChatMessage, this);
		msg->SetBody (body);
		return msg;
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
		return ClientInfo_;
	}

	void MRIMBuddy::MarkMsgsRead ()
	{
	}

	IAdvancedCLEntry::AdvancedFeatures MRIMBuddy::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void MRIMBuddy::DrawAttention (const QString& text, const QString&)
	{
		A_->GetConnection ()->SendAttention (GetHumanReadableID (), text);
	}
}
}
}
