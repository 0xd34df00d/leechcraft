/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "confentry.h"
#include <ranges>
#include <interfaces/azoth/azothutil.h>
#include "confmessage.h"
#include "confparticipant.h"
#include "confsmanager.h"
#include "toxaccount.h"
#include "toxthread.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	ConfEntry::ConfEntry (uint32_t confNum, ConfId confId, ConfsManager& mgr)
	: QObject { &mgr }
	, Mgr_ { mgr }
	, ConfId_ { confId }
	, ConfNum_ { confNum }
	, EntryId_ { mgr.GetAccount ().GetAccountID () + '_' + ToxId2HR (ConfId_) }
	{
		emit mgr.GetAccount ().gotCLItems ({ this });
	}

	ConfEntry::~ConfEntry ()
	{
		auto removed = GetParticipants ();
		removed << this;
		emit Mgr_.GetAccount ().removedCLItems (removed);
	}

	ConfsManager& ConfEntry::GetConfsManager ()
	{
		return Mgr_;
	}

	QObject* ConfEntry::GetQObject ()
	{
		return this;
	}

	IAccount* ConfEntry::GetParentAccount () const
	{
		return &Mgr_.GetAccount ();
	}

	ICLEntry::Features ConfEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ConfEntry::GetEntryType () const
	{
		return EntryType::MUC;
	}

	QString ConfEntry::GetEntryName () const
	{
		return {};
	}

	void ConfEntry::SetEntryName (const QString&)
	{
	}

	QString ConfEntry::GetEntryID () const
	{
		return EntryId_;
	}

	QStringList ConfEntry::Groups () const
	{
		return { tr ("Conferences") };
	}

	void ConfEntry::SetGroups (const QStringList&)
	{
	}

	QStringList ConfEntry::Variants () const
	{
		return { {} };
	}

	void ConfEntry::SendMessage (const OutgoingMessage& message)
	{
		// TODO
	}

	QList<IMessage*> ConfEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	void ConfEntry::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void ConfEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus ConfEntry::GetStatus (const QString&) const
	{
		return { SOnline, {} };
	}

	void ConfEntry::ShowInfo ()
	{
	}

	QList<QAction*> ConfEntry::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> ConfEntry::GetClientInfo (const QString&) const
	{
		return {};
	}

	void ConfEntry::MarkMsgsRead ()
	{
	}

	void ConfEntry::ChatTabClosed ()
	{
	}

	IMUCEntry::MUCFeatures ConfEntry::GetMUCFeatures () const
	{
		return MUCFCanHaveSubject;
	}

	QString ConfEntry::GetMUCSubject () const
	{
		return {};
	}

	void ConfEntry::SetMUCSubject (const QString&)
	{
	}

	bool ConfEntry::CanChangeSubject () const
	{
		return false;
	}

	QList<QObject*> ConfEntry::GetParticipants ()
	{
		return { Participants_.begin (), Participants_.end() };
	}

	bool ConfEntry::IsAutojoined () const
	{
		return false;
	}

	void ConfEntry::Join ()
	{
	}

	QString ConfEntry::GetGroupName () const
	{
		return {};
	}

	QString ConfEntry::GetRealID (QObject*) const
	{
		qWarning () << "real IDs are unsupported";
		return {};
	}

	QVariantMap ConfEntry::GetIdentifyingData () const
	{
		// TODO
		return {};
	}

	void ConfEntry::InviteToMUC (const QString& userId, const QString& msg)
	{
	}

	Util::ContextTask<void> ConfEntry::RunLeave ()
	{
		co_await Util::AddContextObject { *this };

		const auto runner = Mgr_.GetAccount ().GetTox ();
		if (!runner)
			co_return;

		// Any error of tox_conference_delete means that conference is already gone,
		// so we can ignore it for our purposes.
		co_await runner->RunWithError (&tox_conference_delete, ConfNum_);
		runner->Run (&ToxW::SaveState);

		Mgr_.HandleSelfLeft (ConfNum_);
	}

	void ConfEntry::Leave (const QString&)
	{
		RunLeave ();
	}

	QString ConfEntry::GetNick () const
	{
		return Mgr_.GetAccount ().GetOurNick ();
	}

	void ConfEntry::SetNick (const QString&)
	{
		qWarning () << "setting per-conference nicknames is not supported";
	}

	void ConfEntry::HandleConnected ()
	{
		// TODO emit a service message
	}

	void ConfEntry::HandleMessage (const ConfMessageEvent& event)
	{
		const auto pkey = OnlineNum2Pubkey_.value (event.PeerNum_);
		const auto participant = Participants_.value (pkey);
		if (!participant)
		{
			qWarning () << "unknown participant for" << event.PeerNum_ << event.Message_;
			return;
		}

		const auto msg = new ConfMessage { event.Message_, *participant };
		msg->Store ();
	}

	void ConfEntry::AppendMessage (ConfMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	namespace
	{
		struct Peer
		{
			QString Name_;
			Pubkey Pkey_;
			uint32_t Num_;
		};

		struct OfflinePeer : Peer
		{
			QDateTime LastActive_;
		};

		struct Peers
		{
			uint32_t Self_ = -1;
			QList<Peer> Online_;
			QList<OfflinePeer> Offline_;
		};

		Util::Either<TOX_ERR_CONFERENCE_PEER_QUERY, Peers> GetPeers (ToxW& self, uint32_t confNum)
		{
			const auto tox = self.GetTox ();

			Peers result;

			for (const auto onlinePeers = co_await WithError (&tox_conference_peer_count, tox, confNum);
				 const auto peerNum : std::views::iota (0u, onlinePeers))
			{
				const auto& name = co_await QueryToxString (&tox_conference_peer_get_name_size,
						&tox_conference_peer_get_name,
						tox, confNum, peerNum);
				const auto& pkey = co_await QueryToxBytes<PubkeySize> (&tox_conference_peer_get_public_key,
						tox, confNum, peerNum);
				result.Online_ << Peer { .Name_ = name, .Pkey_ = pkey, .Num_ = peerNum };

				if (co_await WithError (&tox_conference_peer_number_is_ours, tox, confNum, peerNum))
					result.Self_ = peerNum;
			}

			for (const auto offlinePeers = co_await WithError (&tox_conference_offline_peer_count, tox, confNum);
				 const auto peerNum : std::views::iota (0u, offlinePeers))
			{
				const auto& name = co_await QueryToxString (&tox_conference_offline_peer_get_name_size,
						&tox_conference_offline_peer_get_name,
						tox, confNum, peerNum);
				const auto& pkey = co_await QueryToxBytes<PubkeySize> (&tox_conference_offline_peer_get_public_key,
						tox, confNum, peerNum);
				const auto lastActiveSecs = co_await WithError (&tox_conference_offline_peer_get_last_active,
						tox, confNum, peerNum);
				const auto lastActive = QDateTime::fromSecsSinceEpoch (lastActiveSecs);
				result.Offline_ << OfflinePeer { { .Name_ = name, .Pkey_ = pkey, .Num_ = peerNum }, lastActive };
			}

			co_return result;
		}

		QSet<Pubkey> GetAllPkeys (const Peers& peers)
		{
			QSet<Pubkey> result;
			result.reserve (peers.Online_.size () + peers.Offline_.size ());
			for (const auto& peer : peers.Online_)
				result << peer.Pkey_;
			for (const auto& peer : peers.Offline_)
				result << peer.Pkey_;
			return result;
		}
	}

	Util::ContextTask<void> ConfEntry::RefreshParticipants ()
	{
		const auto runner = Mgr_.GetAccount ().GetTox ();
		if (!runner)
			co_return;

		const auto peersResult = co_await runner->Run (&GetPeers, ConfNum_);
		const auto peers = co_await WithHandler (peersResult,
				[] (TOX_ERR_CONFERENCE_PEER_QUERY error)
				{
					qWarning () << "unable to query peers:" << error << tox_err_conference_peer_query_to_string (error);
				});

		const auto& newPkeys = GetAllPkeys (peers);
		const auto& prevPkeys = QSet<Pubkey> { Participants_.keyBegin (), Participants_.keyEnd () };

		QList<QObject*> removedEntries;
		for (const auto removedPkeys = prevPkeys - newPkeys;
			 auto pkey : removedPkeys)
			removedEntries << Participants_.take (pkey);
		if (!removedEntries.isEmpty ())
			emit Mgr_.GetAccount ().removedCLItems (removedEntries);
		qDeleteAll (removedEntries);

		OnlineNum2Pubkey_.clear ();
		OfflineNum2Pubkey_.clear ();

		QList<QObject*> newEntries;
		const auto handlePeers = [this, &newEntries] (const auto& list,
				QHash<uint32_t, Pubkey>& num2pkey, const auto& getState)
		{
			for (const auto& peer : list)
			{
				num2pkey [peer.Num_] = peer.Pkey_;

				if (auto& entry = Participants_ [peer.Pkey_])
					entry->SetState (getState (peer));
				else
				{
					entry = new ConfParticipant { peer.Pkey_, peer.Name_, getState (peer), *this };
					newEntries << entry;
				}
			}
		};
		handlePeers (peers.Online_,
				OnlineNum2Pubkey_,
				[] (auto) { return ConfParticipant::Online {}; });
		handlePeers (peers.Offline_,
				OfflineNum2Pubkey_,
				[] (const OfflinePeer& peer) { return ConfParticipant::Offline { peer.LastActive_ }; });
		if (!newEntries.isEmpty ())
			emit Mgr_.GetAccount ().gotCLItems (newEntries);
	}
}
