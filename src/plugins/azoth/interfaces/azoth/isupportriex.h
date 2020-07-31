/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#include <QFlags>
#include <QMetaType>
#include <QStringList>

namespace LC
{
namespace Azoth
{
	/** @brief Represents a single exchange entry.
	 */
	struct RIEXItem
	{
		/** @brief What should be done with this item.
		 *
		 * Since contact exchange is modelled after XEP-0144
		 * ( http://xmpp.org/extensions/xep-0144.html ), the LeechCraft
		 * Core follows the recommended behavior while handling the
		 * items.
		 */
		enum Action
		{
			/** This item should be added to the roster.
			 */
			AAdd,

			/** @brief This item should be removed from the roster.
			 */
			ADelete,

			/** @brief This item should be updated in the roster.
			 */
			AModify
		} Action_;

		/** @brief The human-readable ID of the entry.
		 *
		 * This field must be non-empty.
		 *
		 * This is semantically equivalent to the return value of the
		 * ICLEntry::GetHumanReadableID(). The same semantics apply:
		 * basically, this ID must uniquely identify the target user
		 * in the protocol, but there is no need to make it unique
		 * LeechCraft-wide.
		 *
		 * @sa ICLEntry::GetHumanReadableID().
		 */
		QString ID_;

		/** @brief The suggested nickname of the entry.
		 *
		 * This field may be empty.
		 */
		QString Nick_;

		/** @brief The suggested groups for the entry.
		 *
		 * This field may be empty.
		 *
		 * The exact semantics of this field depend on the value of the
		 * Action_ field.
		 */
		QStringList Groups_;
	};

	inline bool operator== (const RIEXItem& r1, const RIEXItem& r2)
	{
		return r1.Action_ == r2.Action_ &&
			r1.ID_ == r2.ID_ &&
			r1.Nick_ == r2.Nick_ &&
			r1.Groups_ == r2.Groups_;
	}

	/** @brief Interface representing Roster Item Exchange-like things.
	 *
	 * This interface should be implemented by account objects that
	 * support exchanging contact list items between different users.
	 *
	 * This interface is modeled after XEP-0144.
	 */
	class ISupportRIEX
	{
	public:
		virtual ~ISupportRIEX () {}

		/** @brief Sends exchange request to the given entry.
		 *
		 * This function should send the given items to the given entry.
		 *
		 * @param[in] items The list of items to be shared (added,
		 * deleted or modified).
		 * @param[in] entry The target entry whom the items should be
		 * sent.
		 * @param[in] message Optional reason message.
		 */
		virtual void SuggestItems (QList<RIEXItem> items, QObject *entry, QString message) = 0;

		/** @brief Notifies that other part suggested modifying roster.
		 *
		 * This signal should be emitted whenever a remote part suggests
		 * adding, deleting or modifying items in the target user's
		 * contact list.
		 *
		 * Some protocols may allow pushing items to the roster without
		 * any source contact present. In this case, the from parameter
		 * may be set to null.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] items The list of items suggested.
		 * @param[out] from The contact that suggested adding the entry,
		 * or null if not applicable. Must implement ICLEntr.
		 * @param[out] message Optional reason message.
		 */
		virtual void riexItemsSuggested (QList<LC::Azoth::RIEXItem> items,
				QObject *from, QString message) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportRIEX,
		"org.Deviant.LeechCraft.Azoth.ISupportRIEX/1.0")
Q_DECLARE_METATYPE (LC::Azoth::RIEXItem)

#endif
