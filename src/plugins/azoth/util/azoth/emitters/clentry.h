/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/azothcommon.h>
#include "../azothutilconfig.h"

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API CLEntry : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief This signal should be emitted whenever a new message
		 * is received.
		 *
		 * @param[out] msg The message that was just received.
		 */
		void gotMessage (QObject *msg);

		/** @brief This signal should be emitted whenever the status of
		 * a variant in this entry changes.
		 *
		 * @param[out] st The new status of this entry.
		 * @param[out] variant What variant is subject to change.
		 */
		void statusChanged (const EntryStatus& st, const QString& variant);

		/** @brief This signal should be emitted whenever the list of
		 * available variants changes.
		 *
		 * @param[out] newVars The list of new variants, as
		 * returned by GetVariants().
		 */
		void availableVariantsChanged (const QStringList& newVars);

		/** @brief This signal should be emitted whenever the entry
		 * changes name.
		 *
		 * This signal should be emitted both if the name of the entry
		 * changes as the result of our actions (particularly, the
		 * SetEntryName() method) and as a result of some other event,
		 * for example, a roster push in XMPP.
		 *
		 * @param[out] name The new name of this entry.
		 */
		void nameChanged (const QString& name);

		/** @brief This signal should be emitted whenever the entry's
		 * groups are changed.
		 *
		 * This signal should be emitted both if the list of groups
		 * changes as the result of our actions (particularly, the
		 * SetGroups() method) and as a result of some other event, for
		 * example, a roster push in XMPP.
		 *
		 * @param[out] groups The new list of groups of this entry.
		 */
		void groupsChanged (const QStringList& groups);

		/** @brief This signal should be emitted whenever the chat
		 * participation state of this entry changes.
		 *
		 * @param[out] state The new chat state.
		 * @param[out] variant The variant that this change applies to,
		 * may be a null string if not applicable.
		 */
		void chatPartStateChanged (const ChatPartState& state, const QString& variant);

		/** @brief This signal should be emitted if it's a MUC
		 * participant and his role/affiliation changes.
		 */
		void permsChanged ();

		/** @brief This signal should be emitted when the entry changes.
		 *
		 * This signal should be emitted only if no other signals apply
		 * (even those from IAdvancedCLEntry or such): it is some kind
		 * of a fall-back notification.
		 */
		void entryGenerallyChanged ();
	};
}
