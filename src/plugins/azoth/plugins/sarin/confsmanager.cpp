/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "confsmanager.h"
#include <tox/toxav.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "confentry.h"
#include "groupjoinwidget.h"
#include "toxaccount.h"
#include "toxcontact.h"
#include "toxthread.h"

namespace LC::Azoth::Sarin
{
	ConfsManager::ConfsManager (ToxAccount& acc)
	: QObject { &acc }
	, Acc_ { acc }
	{
		connect (&acc,
				&ToxAccount::threadChanged,
				this,
				&ConfsManager::HandleToxThreadChanged);
		HandleToxThreadChanged (acc.GetTox ());
	}

	ToxAccount& ConfsManager::GetAccount ()
	{
		return Acc_;
	}

	namespace
	{
		void ReportError (const QString& text)
		{
			const auto& e = Util::MakeNotification ("Azoth Sarin"_qs, text, Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		Util::Task<std::optional<uint32_t>> JoinTextConf (std::shared_ptr<ToxRunner> tox, QByteArray cookie, uint32_t friendNum)
		{
			constexpr auto maxRetries = 10;
			for (int i = 0; i < maxRetries; ++i)
			{
				const auto joinResult = co_await tox->RunWithError (&tox_conference_join,
						friendNum, std::bit_cast<const uint8_t*> (cookie.constData ()), cookie.size ());
				if (joinResult.IsRight ())
					co_return joinResult.GetRight ();

				const auto error = joinResult.GetLeft ();
				const auto errMsg = tox_err_conference_join_to_string (error);
				switch (error)
				{
				case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
					using namespace std::chrono_literals;
					co_await (1s * (i + 1));
					continue;
				case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
					ReportError (ConfsManager::tr ("You have already joined this conference."));
					co_return {};
				default:
					ReportError (ConfsManager::tr ("Unable to join the conference: %1.").arg (errMsg));
					co_return {};
				}
			}

			ReportError (ConfsManager::tr ("Unable to join the conference after %n attempt(s).", nullptr, maxRetries));
			co_return {};
		}

		Util::Task<std::optional<uint32_t>> JoinAvConf (std::shared_ptr<ToxRunner> tox, QByteArray cookie, uint32_t friendNum)
		{
			const auto confId = co_await tox->Run ([&] (ToxW& tox)
					{
						return toxav_join_av_groupchat (tox.GetTox (),
								friendNum,
								std::bit_cast<const uint8_t*> (cookie.constData ()),
								cookie.size (),
								nullptr,
								nullptr);
					});
			if (confId >= 0)
				co_return confId;

			qWarning () << "unable to join AV conference";
			ReportError (ConfsManager::tr ("Unable to join AV conference."));
			co_return {};
		}
	}

	Util::ContextTask<void> ConfsManager::Join (QByteArray cookie, ConfType type, uint32_t friendNum)
	{
		co_await Util::AddContextObject { *this };

		const auto tox = Acc_.GetTox ();
		if (!tox)
			co_return;

		std::optional<uint32_t> confNum;
		switch (type)
		{
		case ConfType::Text:
			confNum = co_await JoinTextConf (tox, cookie, friendNum);
			break;
		case ConfType::AV:
			confNum = co_await JoinAvConf (tox, cookie, friendNum);
			break;
		}
		if (!confNum)
			co_return;

		const auto confId = co_await tox->Run ([confNum] (ToxW& tox) -> std::optional<ConfId>
				{
					ConfId id {};
					if (!tox_conference_get_id (tox.GetTox (), *confNum, &id [0]))
						return {};
					return id;
				});
		if (!confId)
		{
			qWarning () << "unable to get conference ID for" << *confNum;
			co_return;
		}

		tox->Run (&ToxW::SaveState);

		const auto entry = new ConfEntry { *confNum, *confId, *this };
		Conf2Entry_ [*confNum] = entry;
		entry->RefreshParticipants ();
	}

	void ConfsManager::HandleSelfLeft (uint32_t confNum)
	{
		if (const auto entry = Conf2Entry_.take (confNum))
			entry->deleteLater ();
		else
			qWarning () << "unknown entry for" << confNum;
	}

	void ConfsManager::HandleInvited (const ConfInvitationEvent& invite)
	{
		const auto inviter = Acc_.GetByPubkey (invite.FriendPKey_);
		const auto& ident = GroupJoinWidget::GetConfIdentifyingData (invite.Cookie_, invite.Type_, invite.FriendNum_);
		emit Acc_.mucInvitationReceived (ident, inviter ? inviter->GetEntryName () : ToxId2HR (invite.FriendPKey_), {});
	}

	void ConfsManager::HandleToxThreadChanged (const std::shared_ptr<ToxRunner>& runner)
	{
		if (!runner)
			return;

		auto route = [this] (auto fun)
		{
			return [this, fun]<typename... Args> (uint32_t confNum, const Args&... args)
			{
				if (const auto groupEntry = Conf2Entry_.value (confNum))
					std::invoke (fun, groupEntry, args...);
				else
					qWarning () << "no entry for conference" << confNum;
			};
		};

		connect (&*runner,
				&ToxRunner::confInvited,
				this,
				&ConfsManager::HandleInvited);
		connect (&*runner,
				&ToxRunner::confConnected,
				this,
				route (&ConfEntry::HandleConnected));
		connect (&*runner,
				&ToxRunner::confMessage,
				this,
				route (&ConfEntry::HandleMessage));
		connect (&*runner,
				&ToxRunner::confPeerListChanged,
				this,
				route (&ConfEntry::RefreshParticipants));
	}
}
