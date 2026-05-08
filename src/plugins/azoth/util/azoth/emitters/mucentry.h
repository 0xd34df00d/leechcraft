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
	enum class Liveness : std::uint8_t
	{
		Historical,
		Live,
	};

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

	struct SubjectChange
	{
		QString Subject_;

		std::optional<QString> ActorNick_ {};
	};
}

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API MUCEntry : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Notifies about room subject change.
		 *
		 * @param[out] event The information about the subject change event.
		 */
		void mucSubjectChanged (const MucEvents::SubjectChange& event);

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
		 *   still be emitted after `participantLeaving`.
		 *
		 * So, the expected signal flow is:
		 * 1. `gotCLItems`
		 * 2. `participantJoined`
		 * 3. `participantLeaving`
		 * 4. `removedCLItems`
		 * 5. `participantLeft`
		 * or the last two can be swapped.
		 */
		///@{

		/** @brief Notifies that a `participant` has joined this room.
		 *
		 * The participant might've joined the room before us (as per `liveness`),
		 * in which case this signal is a part of the initial participants batch.
		 *
		 * It is safe to query participant permissions, status, etc, within the
		 * connected slot.
		 *
		 * @param[out] participant The participant that has joined the room.
		 * @param[out] liveness Whether the participant joined before or after us.
		 *
		 * @sa participantLeaving
		 * @sa participantLeft
		 */
		void participantJoined (ICLEntry& participant, MucEvents::Liveness liveness);

		/** @brief Notifies that a `participant` is about to leave this room.
		 *
		 * The participant is still in the `IMUCEntry::GetParticipants()` list.
		 *
		 * This signal is emitted when the participant leaves the room voluntarily,
		 * as well as he is kicked/banned.
		 *
		 * It is safe to query participant permissions, status, etc, within the
		 * connected slot.
		 *
		 * @param[out] participant The participant that is leaving the room.
		 * @param[out] leaveInfo The details of why the participant is leaving the room.
		 *
		 * @sa participantJoined
		 * @sa participantLeft
		 */
		void participantLeaving (ICLEntry& participant, const MucEvents::ParticipantLeaveInfo& leaveInfo);

		/** @brief Notifies that a `participant` has just left the room.
		 *
		 * The participant is no longer in the `IMUCEntry::GetParticipants()` list.
		 *
		 * This signal is emitted when the participant leaves the room voluntarily,
		 * as well as he is kicked/banned.
		 *
		 * The participant name, status, permissions, etc, are no longer available,
		 * and while the corresponding functions can still be called, their return
		 * values are unspecified (or, to simplify, return anything, just don't
		 * crash).
		 *
		 * Whether this signal is emitted before or after the corresponding
		 * `removedCLItems` (if the latter is emitted at all) is unspecified.
		 *
		 * @param[out] participant The participant that has left the room.
		 * @param[out] leaveInfo The details of why the participant has left the room.
		 *
		 * @sa participantJoined
		 * @sa participantLeaving
		 */
		void participantLeft (ICLEntry& participant, const MucEvents::ParticipantLeaveInfo& leaveInfo);

		///@}
	};
}
