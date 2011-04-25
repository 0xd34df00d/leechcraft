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
#include <QVariant>

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
	 * 
	 * See IConfigurableMUC if the MUC supports being configured and
	 * IMUCPerms if the MUC supports adjusting permissions for its
	 * participants.
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
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IMUCEntry::MUCFeatures);
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCEntry,
		"org.Deviant.LeechCraft.Azoth.IMUCEntry/1.0");

#endif
