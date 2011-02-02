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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#include <QFlags>
#include <QMetaType>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	/** @brief Represents a single MUC entry in the CL.
	 *
	 * This class extends ICLEntry by providing methods and data
	 * specific to MUCs. A well-written plugin should implement this
	 * interface along with ICLEntry for MUC entries.
	 */
	class IMUCEntry
	{
		Q_GADGET
	public:
		virtual ~IMUCEntry () {}

		enum MUCFeature
		{
			/** This room has a configuration dialog and can be
			 * configured.
			 */
			MUCFCanBeConfigured = 0x0001,
			/** Room can have a (possibly empty) subject which may be
			 * retrieved by GetMUCSubject().
			 */
			MUCFCanHaveSubject = 0x0002
		};

		Q_DECLARE_FLAGS (MUCFeatures, MUCFeature);

		/** This enum represents possible affiliations of a participant
		 * in a room.
		 *
		 * Modelled after XMPP MUC room affiliations, ordered to reflect
		 * Gloox's order and thus to allow direct static_casts.
		 */
		enum MUCAffiliation
		{
			/** Invalid affiliation.
			 */
			MUCAInvalid,
			/** The user has been banned from the room.
			 *
			 * Setting this affiliation on a user should effectively ban
			 * it from entering the room if applicable to the protocol
			 * and room type.
			 */
			MUCAOutcast,
			/** No affiliation with the room.
			 */
			MUCANone,
			/** The user is a member of the room.
			 */
			MUCAMember,
			/** The user is a room admin.
			 */
			MUCAAdmin,
			/** The user is a room owner.
			 */
			MUCAOwner
		};

		Q_ENUMS (MUCAffiliation);

		/** This enum represents possible roles of a participant in a
		 * MUC room.
		 *
		 * Modelled after XMPP MUC room roles.
		 */
		enum MUCRole
		{
			/** Invalid role.
			 */
			MUCRInvalid,
			/** Not present in room.
			 *
			 * Setting this role on a user should effectively kick it
			 * from the room if applicable to the protocol and room
			 * type.
			 */
			MUCRNone,
			/** The user visits a room.
			 *
			 * Setting this role on a user should effectively devoice
			 * the user if applicable to the protocol and room type.
			 */
			MUCRVisitor,
			/** The user has voice in a moderated room.
			 */
			MUCRParticipant,
			/** The user is a room moderator.
			 */
			MUCRModerator
		};

		Q_ENUMS (MUCRole);

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

		/** @brief The list of participants of this MUC.
		 *
		 * If the protocol plugin chooses to return info about
		 * participants via the IAccount interface, the ICLEntry objects
		 * returned from this function and from IAccount should be the
		 * same for the same participants.
		 *
		 * @return The list of participants of this MUC.
		 */
		virtual QList<QObject*> GetParticipants () = 0;

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

		/** @brief Whether affiliation of the given participant may be
		 * changed to the given value.
		 *
		 * This functions queries whether at this moment the affiliation
		 * of the given participant may be changed to the given value
		 * aff.
		 *
		 * The participant object is expected to implement ICLEntry and
		 * be the one returned by this room, if any. If the participant
		 * object doesn't match any of these criteria, this function
		 * should return false.
		 *
		 * @param[in] participant The participant to query, implementing
		 * ICLEntry and belonging to this room.
		 * @param[in] aff The target affiliation to be checked.
		 *
		 * @return Whether the affiliation may be changed.
		 *
		 * @sa MayChangeRole(), GetAffiliation(), SetAffiliation()
		 */
		virtual bool MayChangeAffiliation (QObject *participant, MUCAffiliation aff) const = 0;

		/** @brief Whether role of the given participant may be changed
		 * to the given value.
		 *
		 * This function behaves exactly like MayChangeAffiliation(),
		 * but for roles.
		 *
		 * @param[in] participant The participant to query, implementing
		 * ICLEntry and belonging to this room.
		 * @param[in] aff The target affiliation to be checked.
		 *
		 * @return Whether the affiliation may be changed.
		 *
		 * @sa MayChangeAffiliation(), GetRole(), SetRole()
		 */
		virtual bool MayChangeRole (QObject *participant, MUCRole role) const = 0;

		/** @brief Returns current affiliation of the given participant.
		 *
		 * The participant object is expected to implement ICLEntry and
		 * be the one returned by this room. If the passed pointer is
		 * NULL, or the object doesn't match any of these criteria, this
		 * function should return our user's affiliation to this room.
		 *
		 * @note This function is also used to get our user's
		 * affiliation, in which case null pointer is passed.
		 *
		 * @param[in] participant The participant to query, or NULL to
		 * get our affiliation.
		 * @return The affiliation of given participant in this room.
		 *
		 * @sa MayChangeAffiliation(), SetAffiliation(), GetRole()
		 */
		virtual MUCAffiliation GetAffiliation (QObject *participant) const = 0;

		/** @brief Sets the affiliation of the given participant.
		 *
		 * The participant object is expected to implement ICLEntry and
		 * be the one returned by this room. If the object doesn't match
		 * any of these criteria, this function should do nothing.
		 *
		 * If the affiliation can't be updated (for example, due to
		 * insufficient privilegies), this function should do nothing.
		 *
		 * @param[in] participant The participant to update.
		 * @param[in] aff New affiliation of the participant to be set.
		 * @param[in] reason Optional reason for the affiliation change.
		 */
		virtual void SetAffiliation (QObject *participant,
				MUCAffiliation aff,
				const QString& reason = QString ()) = 0;

		/** @brief Returns current role of the given participant.
		 *
		 * The participant object is expected to implement ICLEntry and
		 * be the one returned by this room. If the passed pointer is
		 * NULL, or the object doesn't match any of these criteria, this
		 * function should return our user's affiliation to this room.
		 *
		 * @note This function is also used to get our user's role, in
		 * which case null pointer is passed.
		 *
		 * @param[in] participant The participant to query, or NULL to
		 * get our role.
		 * @return The role of given participant in this room.
		 *
		 * @sa MayChangeRole(), SetRole(), GetAffiliation()
		 */
		virtual MUCRole GetRole (QObject *participant) const = 0;

		/** @brief Sets the role of the given participant.
		 *
		 * The participant object is expected to implement ICLEntry and
		 * be the one returned by the room. If the obejct doesn't match
		 * any of these criteria, this function should do nothing.
		 *
		 * If the role can't be updated (for example, due to
		 * insufficient privilegies), this function should do nothing.
		 *
		 * @param[in] participant The participant to update.
		 * @param[in] role New role of the participant to be set.
		 * @param[in] reason Optional reason for the role change.
		 */
		virtual void SetRole (QObject *participant,
				MUCRole role,
				const QString& reason = QString ()) = 0;

		/** @brief Notifies about new participants in the room.
		 *
		 * This signal should emitted when new participants join this
		 * room.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void gotNewParticipants (const QList<QObject*>&) = 0;

		/** @brief Notifies about subject change.
		 *
		 * This signal should be emitted when room subject is changed
		 * to newSubj.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void mucSubjectChanged (const QString& newSubj) = 0;

		/** @brief Notifies about affiliation change of a participant.
		 *
		 * This signal should be emitted whenever the participant's
		 * affiliation changes to newAff.
		 *
		 * @note This function is expected to be a slot.
		 */
		virtual void participantAffiliationChanged (QObject *participant, MUCAffiliation newAff) = 0;

		/** @brief Notifies about role change of a participant.
		 *
		 * This signal should be emitted whenever the participant's role
		 * changes to newRole.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void participantRoleChanged (QObject *participant, MUCRole newRole) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IMUCEntry::MUCFeatures);
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCEntry,
		"org.Deviant.LeechCraft.Azoth.IMUCEntry/1.0");
Q_DECLARE_METATYPE (LeechCraft::Azoth::IMUCEntry::MUCRole);
Q_DECLARE_METATYPE (LeechCraft::Azoth::IMUCEntry::MUCAffiliation);

#endif
