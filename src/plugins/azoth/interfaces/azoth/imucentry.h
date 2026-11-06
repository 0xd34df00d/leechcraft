/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#include <QFlags>
#include <QVariant>
#include <util/azoth/emitters/mucentry.h>

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	/** @brief Represents a single MUC entry in the CL.
	 *
	 * This class extends ICLEntry by providing methods and data
	 * specific to MUCs. A well-written plugin should implement this
	 * interface along with ICLEntry for MUC entries.
	 *
	 * See IConfigurableMUC if the MUC supports being configured and
	 * IMUCPerms if the MUC supports adjusting permissions for its
	 * participants.
	 */
	class IMUCEntry
	{
	protected:
		Emitters::MUCEntry MUCEmitter_;

		virtual ~IMUCEntry () = default;
	public:
		Emitters::MUCEntry& GetMUCEntryEmitter () { return MUCEmitter_; }

		enum MUCFeature
		{
			/** This room has a configuration dialog and can be
			 * configured.
			 */
			MUCFCanBeConfigured = 0x0001,

			/** Room can have a (possibly empty) subject which may be
			 * retrieved by GetMUCSubject().
			 */
			MUCFCanHaveSubject = 0x0002,

			/** Room supports inviting contact list entries of its account
			 * via `InviteToMUC(ICLEntry&, const QString&)`.
			 */
			MUCFCanInvite = 0x0004,

			/** Room supports inviting by protocol address via
			 * `InviteToMUC(const QString&, const QString&)`, so anyone the
			 * protocol can address may be invited, not only existing
			 * contact list entries.
			 */
			MUCFCanInviteByAddress = 0x0008,
		};

		Q_DECLARE_FLAGS (MUCFeatures, MUCFeature)

		/** @brief Returns this MUC as a contact list entry.
		 *
		 * Every IMUCEntry also implements ICLEntry, but the two
		 * interfaces are siblings, so this is the way back to the
		 * entry that does not need RTTI.
		 */
		virtual const ICLEntry& AsCLEntry () const = 0;

		/** @brief The list of features of this MUC.
		 *
		 * Returns the list of features supported by this MUC.
		 */
		virtual MUCFeatures GetMUCFeatures () const = 0;

		/** @brief Returns subject of this MUC.
		 *
		 * Returns the subject/topic of this MUC room, possibly empty.
		 * If the protocol or smth doesn't support subjects for MUCs,
		 * this function should return an empty string.
		 *
		 * @return The subject of this MUC.
		 */
		virtual QString GetMUCSubject () const = 0;

		/** @brief Updates the subject of this MUC.
		 *
		 * Sets the subject of the conference. If it fails for some
		 * reason, for example, due to insufficient rights, this
		 * function should do nothing.
		 *
		 * @param[in] subject The new subject of this room to set.
		 */
		virtual void SetMUCSubject (const QString& subject) = 0;

		/** @brief Returns whether MUC subject can be changed.
		 *
		 * This function should return whether our user can change this
		 * MUC's subject.
		 *
		 * @return Whether the MUC subject can be changed by the user.
		 */
		virtual bool CanChangeSubject () const = 0;

		/** @brief The list of participants of this MUC.
		 *
		 * If the protocol plugin chooses to return info about
		 * participants via the IAccount interface, the ICLEntry objects
		 * returned from this function and from IAccount should be the
		 * same for the same participants.
		 *
		 * @return The list of participants of this MUC.
		 */
		virtual QList<ICLEntry*> GetParticipants () = 0;

		/** @brief Whether this MUC room was automatically joined.
		 *
		 * Azoth uses this, for example, to avoid auto-opening MUCs that
		 * where joined as a result of bookmarks/autojoin list.
		 *
		 * @return Whether the room was autojoined.
		 */
		virtual bool IsAutojoined () const = 0;

		/** @brief Requests to join the room.
		 *
		 * If the we aren't joined to this MUC (for example, there was a
		 * nick conflict, or this entry represents a bookmark), the room
		 * should be tried to be joined.
		 */
		virtual void Join () = 0;

		/** @brief Requests to leave the room.
		 *
		 * The protocol implementation is expected to leave the room
		 * with the given leave message. If leaving is impossible for
		 * some reason, it's ok to stay.
		 *
		 * If the room is successfully left, the parent account should
		 * take care of removing the contact list entries corresponding
		 * to its participants and the room itself.
		 *
		 * @param[in] msg The leave message (if applicable).
		 */
		virtual void Leave (const QString& msg = QString ()) = 0;

		/** @brief Returns the nick of our participant.
		 *
		 * @return The nickname or null string if not applicable.
		 */
		virtual QString GetNick () const = 0;

		/** @brief Changes the nick of our participant.
		 *
		 * If changing nicks is not allowed or is not supported, nothing
		 * should be done.
		 *
		 * @param[in] nick New nick for our participant in
		 * this room.
		 */
		virtual void SetNick (const QString& nick) = 0;

		/** @brief Returns human-readable name of participants' group.
		 *
		 * This function should return the human-readable name of the
		 * group which holds the participants of this room.
		 *
		 * @return The human-readable name.
		 */
		virtual QString GetGroupName () const = 0;

		/** @brief Returns the real ID of a participant.
		 *
		 * This function should return a real protocol ID of the given
		 * participant (JID for XMPP protocol, for example), or a null
		 * string if the ID is unknown, or the given participant doesn't
		 * belong to this room.
		 *
		 * @param[in] participant The participant for which to return
		 * the real JID.
		 * @return The real ID of a participant.
		 */
		virtual QString GetRealId (const ICLEntry& participant) const = 0;

		/** @brief Returns the data identifying this room.
		 *
		 * The returned variant map should have the same format as the
		 * one from IMUCJoinWidget::GetIdentifyingData().
		 *
		 * @return The identifying data.
		 *
		 * @sa IMUCJoinWidget::GetIdentifyingData()
		 */
		virtual QVariantMap GetIdentifyingData () const = 0;

		/** @brief Invites a contact list entry to this MUC.
		 *
		 * The invitation is sent by means of the protocol if it has
		 * them, or as a plain message otherwise.
		 *
		 * @param[in] entry The entry to invite. It must belong to this
		 * MUC's account.
		 * @param[in] msg The optional message to send along with the
		 * invitation.
		 *
		 * @sa MUCFCanInvite
		 */
		virtual void InviteToMUC (ICLEntry& entry, const QString& msg) = 0;

		/** @brief Invites the user with the given protocol address to
		 * this MUC.
		 *
		 * Callers must check `MUCFCanInviteByAddress` first: rooms
		 * without it ignore the call.
		 *
		 * @param[in] address The protocol address of the user to
		 * invite, like a JID in XMPP.
		 * @param[in] msg The optional message to send along with the
		 * invitation.
		 *
		 * @sa MUCFCanInviteByAddress
		 */
		virtual void InviteToMUC (const QString& address, const QString& msg)
		{
			Q_UNUSED (address)
			Q_UNUSED (msg)
			qWarning () << "inviting by address is unsupported";
		}
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IMUCEntry, "org.Deviant.LeechCraft.Azoth.IMUCEntry/1.0")
Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Azoth::IMUCEntry::MUCFeatures)

#endif
