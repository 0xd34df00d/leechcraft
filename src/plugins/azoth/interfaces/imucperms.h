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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCPERMS_H
#define PLUGINS_AZOTH_INTERFACES_IMUCPERMS_H
#include <QFlags>
#include <QMetaType>
#include <QMap>
#include <QByteArray>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief This interface describes permissions in the given room.
	 *
	 * If a room supports getting/changing permissions for participants,
	 * this interface should be implemented by the room's CL entry
	 * object.
	 *
	 * There may be different permission classes in a room, and each
	 * permission class has several permission variants, which are
	 * exclusive, while permissions from different classes are not
	 * exclusive.
	 *
	 * For example, an XMPP MUC room has two permission classes: roles
	 * and affiliations. Role class and affiliation class have a
	 * different set of permissions, like participant/moderator or
	 * outcast/member/admin respectively.
	 */
	class IMUCPerms
	{
	public:
		virtual ~IMUCPerms () {}

		/** @brief Returns all possible permission classes and values.
		 *
		 * Returns all possible permission classes and values for this
		 * MUC. In the map, the key is used as the permission class
		 * identifier, while the corresponding list of QByteArrays is
		 * the list of permission variants for this class.
		 *
		 * Please note that this list should not change between calls to
		 * this method.
		 *
		 * @return The possible permissions for this room.
		 */
		virtual QMap<QByteArray, QList<QByteArray> > GetPossiblePerms () const = 0;

		/** @brief Returns current permissions for the given participant.
		 *
		 * The returned map must have the same keys as the one from
		 * GetPossiblePerms(), and the values must be contained in the
		 * corresponding lists in GetPossiblePerms()'s return value.
		 *
		 * @note This function is also used for getting our user's
		 * permissions. For this, null pointer is passed instead of
		 * participant, and permissions of our user are expected to be
		 * returned.
		 *
		 * @param[in] participant The participant for which to query the
		 * permissions, or NULL to query self.
		 * @return The current permissions for the given participant.
		 */
		virtual QMap<QByteArray, QList<QByteArray> > GetPerms (QObject *participant) const = 0;

		/** @brief Returns the name of the affiliation icon.
		 *
		 * Returns the name of the icon which is somewhat analogous to
		 * XMPP's affiliation system. The plugin is free to choose what
		 * participants should have as the affiliation.
		 *
		 * The name should be one of the following:
		 * - "noaffiliation"
		 * - "outcast"
		 * - "member"
		 * - "admin"
		 * - "owner"
		 *
		 * @param[in] participant The participant to query.
		 * @return The affiliation analog for this participant
		 */
		virtual QByteArray GetAffName (QObject *participant) const = 0;

		/** @brief Whether given participant's permission may be changed
		 * to the given value.
		 *
		 * This function is used to query whether at this moment the
		 * permission from the given permClass could be set to the
		 * targetPerm value by our user.
		 *
		 * targetPerm is one of the corresponding values from the map
		 * returned from GetPossiblePerms() map.
		 *
		 * In case of failure (for example, participant doesn't belong
		 * to this room), this function should return false.
		 *
		 * @param[in] participant The participant to query.
		 * @param[in] permClass Permission class to operate in.
		 * @param[in] targetPerm Target permission in that class.
		 * @return Whether permission could be set successfully.
		 */
		virtual bool MayChangePerm (QObject *participant,
				const QByteArray& permClass, const QByteArray& targetPerm) const = 0;

		/** @brief Sets the permission for the given participant.
		 *
		 * This function is used to set the participant's permission
		 * from the given permClass to the given targetPerm value (which
		 * is one of returned from GetPossiblePerms() for that class).
		 * If applicable, reason is used to describe the reason for
		 * changing the permission, which may be extremely useful, for
		 * example, when kicking or banning.
		 *
		 * @param[in] participant The participant to change the affiliation.
		 */
		virtual void SetPerm (QObject *participant,
				const QByteArray& permClass, const QByteArray& targetPerm, const QString& reason) = 0;

		/** @brief Returns if one participant has less perms than another.
		 *
		 * This method is used for ordering participants when displaying
		 * them in MUC's participants list. For example, an XMPP room
		 * would typically use participants' roles for ordering.
		 *
		 * @param[in] part1 First participant.
		 * @param[in] part2 Second participant.
		 * @return True if first participant is "less than" the other
		 * one, false otherwise.
		 */
		virtual bool IsLessByPerm (QObject *part1, QObject *part2) const = 0;

		/** @brief Returns whether users can have many perms of the
		 * given class at once.
		 *
		 * This function is used to query the whether users in this MUC
		 * can have multiple perms of the given permClass at once.
		 *
		 * @param[in] permClass The permission class to query.
		 * @return True if a user can have multiple permissions at once.
		 */
		virtual bool IsMultiPerm (const QByteArray& permClass) const = 0;

		/** @brief Returns a human-readable string for the given id.
		 *
		 * @param[in] id The identifier to query.
		 * @return The human-readable translated string for this id.
		 */
		virtual QString GetUserString (const QByteArray& id) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCPerms,
		"org.Deviant.LeechCraft.Azoth.IMUCPerms/1.0");

#endif
