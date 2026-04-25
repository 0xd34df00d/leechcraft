/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <variant>
#include <QObject>
#include "../azothutilconfig.h"

namespace LC::Azoth
{
	class ICLEntry;
}

namespace LC::Azoth::MucEvents
{
	struct ParticipantLeft { QString Message_; };

	struct ParticipantForcedOut
	{
		ICLEntry *Actor_ = nullptr;
		QString Reason_;

		enum class Action : std::uint8_t
		{
			Kicked,
			Banned,
		} Action_;
	};

	using ParticipantLeaveInfo = std::variant<ParticipantLeft, ParticipantForcedOut>;
}

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API MUCEntry : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Notifies about subject change.
		 *
		 * This signal should be emitted when room subject is changed
		 * to newSubj.
		 *
		 * @param[out] newSubj The new subject of this room.
		 */
		void mucSubjectChanged (const QString& newSubj);

		/** @brief Notifies about nick conflict.
		 *
		 * This signal should be emitted when room gets the error from
		 * the server that the nickname is already in use.
		 *
		 * The signal handler could either call SetNick() with some
		 * other nickname (in this case the room should automatically
		 * try to rejoin) or do nothing it all (in this case the room
		 * should, well, do nothing as well).
		 *
		 * This signal should be emitted only if the error arises while
		 * joining, not as result of SetNick().
		 *
		 * @param[out] usedNick The nickname that was used to join the
		 * room.
		 */
		void nicknameConflict (const QString& usedNick);

		/** @brief Notifies about us being kicked.
		 *
		 * This signal should be emitted whenever our user gets kicked
		 * from this room.
		 *
		 * @param[out] reason The optional reason message.
		 */
		void beenKicked (const QString& reason);

		/** @brief Notifies about us being banned.
		 *
		 * This signal should be emitted whenever our user gets banned
		 * from this room.
		 *
		 * @param[out] reason The optional reason message.
		 */
		void beenBanned (const QString& reason);

		/** @name Participant lifecycle signals
		 *
		 * These signals notify about participants appearing or disappearing
		 * from this room. They are purely MUC lifecycle signals: no assumption
		 * shall be made about whether the participants existed before, whether
		 * they'll continue to exist after, or whether they belong to any other
		 * rooms.
		 *
		 * The `IAccount`-level signals are still relevant. For the entries that are:
		 * - just created → `Emitter::Account::gotCLItems` shall still be emitted
		 *   before these signals.
		 * - about to be destroyed → `Emitter::Account::removedCLItems` shall
		 *   still be emitted after these signals.
		 *
		 * It is safe to query participant permissions, status, etc, within the
		 * notification signals.
		 */
		///@{

		/** @brief Notifies about the initial participants list in the room.
		 *
		 * These participants were in the room before we joined.
		 *
		 * @param[out] participants The list of participants that were in the room before we joined.
		 *
		 * @sa participantJoined
		 * @sa participantLeaving
		 */
		void gotInitialParticipants (const QList<ICLEntry*>& participants);

		/** @brief Notifies that a `participant` has just joined this room.
		 *
		 * @param[out] participant The participant that has joined the room.
		 *
		 * @sa gotInitialParticipants
		 * @sa participantLeaving
		 */
		void participantJoined (ICLEntry& participant);

		/** @brief Notifies that a `participant` is about to leave this room.
		 *
		 * @param[out] participant The participant that is leaving the room.
		 *
		 * @sa gotInitialParticipants
		 * @sa participantJoined
		 */
		void participantLeaving (ICLEntry& participant, const MucEvents::ParticipantLeaveInfo& leaveInfo);

		///@}
	};
}
