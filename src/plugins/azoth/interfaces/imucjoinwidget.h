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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCJOINWIDGET_H
#define PLUGINS_AZOTH_INTERFACES_IMUCJOINWIDGET_H
#include <QVariant>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief This interface defines methods that should be implemented
	 * in widgets which are used to join MUC rooms.
	 *
	 * Please note that the join widget is global for the whole protocol
	 * thus account objects are used to distinguish between different
	 * accounts when joining.
	 */
	class IMUCJoinWidget
	{
	public:
		virtual ~IMUCJoinWidget () {}

		/** @brief Called when user selects a different account of this
		 * protocol.
		 *
		 * In this function the join widget may change the account-
		 * dependent options according to the selected account, like
		 * default join nickname and such.
		 *
		 * This function is only called if the account belonging to the
		 * join widget's protocol is selected. If the user selects an
		 * account from another protocol, this function won't be called.
		 *
		 * @param[in] account The account object of the protocol that
		 * returned this widget.
		 */
		virtual void AccountSelected (QObject *account) = 0;

		/** @brief Called when user decides to join a chatroom from the
		 * given account.
		 *
		 * @param[in] account The account object through which the user
		 * wishes to join.
		 */
		virtual void Join (QObject *account) = 0;

		/** @brief Called when user decides to not join any chatrooms.
		 */
		virtual void Cancel () = 0;

		/** @brief Returns the map with current join parameters.
		 *
		 * This map is used later to restore the join parameters, for
		 * example, if they were saved to history, and the user decides
		 * to pick a recent join from the history.
		 *
		 * The returned map should have two mandatory fields. First is
		 * HumanReadableName, its value should be a human-readable
		 * QString, it is used to visually represent this item to the
		 * user. Second field is AccountID, the corresponding value
		 * should be a QByteArray with the ID of the selected account
		 * (IAccount::GetAccountID()).
		 *
		 * You are free to use any other fields.
		 *
		 * @return Join parameters map.
		 *
		 * @sa SetIdentifyingData()
		 */
		virtual QVariantMap GetIdentifyingData () const = 0;

		/** @brief Returns the list of bookmarked MUCs, if any.
		 *
		 * The returned list is a list of QVariantMap. Please see the
		 * documentation for GetIdentifyingData() for more information about
		 * maps' contents.
		 *
		 * @return List of bookmarks parameters.
		 *
		 * @sa GetIdentifyingData()
		 */
		virtual QVariantList GetBookmarkedMUCs () const = 0;

		/** @brief Sets the previously saved join parameters.
		 *
		 * This function is called when the user selects a previously
		 * saved join. See the documentation for GetIdentifyingData()
		 * for more information regarding the map's contents.
		 *
		 * @param[in] data Join parameters map.
		 *
		 * @sa GetIdentifyingData()
		 */
		virtual void SetIdentifyingData (const QVariantMap& data) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCJoinWidget,
		"org.Deviant.LeechCraft.Azoth.IMUCJoinWidget/1.0");

#endif
