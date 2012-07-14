/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IHISTORYPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IHISTORYPLUGIN_H
#include <QList>
#include <QVariantMap>

class QObject;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for plugins storing chat history.
	 *
	 * This interface should be implemented by plugins that store chat
	 * history for Azoth to provide additional features using these
	 * plugins.
	 */
	class IHistoryPlugin
	{
	public:
		virtual ~IHistoryPlugin () {}

		/** @brief Whether history is enabled for the given entry.
		 *
		 * This method checks if history logging is enabled for the
		 * given entry.
		 *
		 * @param[in] entry The entry to check (implements ICLEntry).
		 * @return Whether history logging is enabled for this entry.
		 */
		virtual bool IsHistoryEnabledFor (QObject *entry) const = 0;

		/** @brief Requests last messages for the given entry.
		 *
		 * This method, when called, requests last num messages from
		 * the chat log with the entry.
		 *
		 * This method is asynchronous: it is expected to return soon
		 * after being called, and the result is expected to be emitted
		 * via the gotLastMessages() signal.
		 *
		 * @param[in] entry The entry for which to query the history
		 * (implements ICLEntry).
		 * @param[in] num The maximum number of messages to retrieve.
		 *
		 * @sa gotLastMessages()
		 */
		virtual void RequestLastMessages (QObject *entry, int num) = 0;

		/** @brief Adds a raw message to the history.
		 *
		 * The raw message is stored in the rawMsg map. The map contains
		 * the following keys:
		 * - "EntryID" of type QString: the unique ID of the entry.
		 * - "AccountID" of type QString: the unique ID of the
		 *   corresponding account.
		 * - "DateTime" of type QDateTime: the timestamp of the message.
		 * - "Direction" of type QString: either "in" or "out".
		 * - "Body" of type QString: the contents of the message.
		 * - "OtherVariant" of type QString: the variant of the entry
		 *   that has sent the message.
		 * - "MessageType" of type int: the type of the message.
		 * - "VisibleName" of type QString: the human-readable name of
		 *   the entry.
		 *
		 * @param[in] rawMsg The variant map containing the raw message
		 * description.
		 */
		virtual void AddRawMessage (const QVariantMap& rawMsg) = 0;

		/** @brief Notifies about last messages for the given entry.
		 *
		 * This signal should be emitted when last chat messages with
		 * the given entry have been retrieved from the history as the
		 * result of the call to RequestLastMessages().
		 *
		 * If there are no messages for the entry, the implementation
		 * may either emit this signal with empty messages list or
		 * choose to not emit any signals at all.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @sa RequestLastMessages()
		 */
		virtual void gotLastMessages (QObject *entry, const QList<QObject*>& messages) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHistoryPlugin,
		"org.Deviant.LeechCraft.Azoth.IHistoryPlugin/1.0");

#endif
