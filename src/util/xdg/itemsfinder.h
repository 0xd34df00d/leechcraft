/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>
#include "xdgconfig.h"

namespace LC::Util::XDG
{
	class UTIL_XDG_API Item;
	using Item_ptr = std::shared_ptr<Item>;

	using Cat2Items_t = QHash<QString, QList<Item_ptr>>;

	enum class Type;

	/** @brief Finds and parses XDG <code>.desktop</code> files.
	 *
	 * The <code>.desktop</code> files are found in the directories
	 * matching the types passed to the object's constructor.
	 *
	 * Parsing is done asynchronously in a separate thread, and the
	 * itemsListChanged() signal is emitted each time the list of files
	 * changes.
	 *
	 * This class does not watch for changes in the said paths. Use the
	 * ItemsDatabase instead if that functionality is required.
	 *
	 * @sa ItemsDatabase
	 */
	class UTIL_XDG_API ItemsFinder : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		Cat2Items_t Items_;

		bool IsReady_ = false;
		bool IsScanning_ = false;

		const QList<Type> Types_;
	public:
		/** @brief Constructs the items finder for the given \em types.
		 *
		 * ToPaths() is used to get the list of directories for each of
		 * the \em types.
		 *
		 * The ItemsFinder will asynchronously update itself
		 * automatically a few moments after creation and emit
		 * itemsListChanged() when the update finishes.
		 *
		 * @param[in] proxy The proxy to use to get the icons of the
		 * items that were found.
		 * @param[in] types The item types to watch for.
		 * @param[in] parent The parent object of this finder.
		 */
		ItemsFinder (const ICoreProxy_ptr&, const QList<Type>& types, QObject *parent = nullptr);

		/** @brief Checks whether this items finder is ready.
		 *
		 * An items finder is ready if it has finished at least one search
		 * process. In other words, IsReady() returns true iff
		 * itemsListChanged() has been emitted at least once.
		 *
		 * @return Whether the items finder has finished at least one
		 * items search.
		 */
		bool IsReady () const;

		/** @brief Returns the categorized list of XDG items.
		 *
		 * The returned hash contains the mapping from XDG category ID to
		 * the list of items in that category. A single item can appear in
		 * multiple categories at the same time.
		 *
		 * If the finder is not ready (IsReady() returns false), this
		 * function returns an empty hash.
		 *
		 * @return The categorized list of items.
		 */
		Cat2Items_t GetItems () const;

		/** @brief Finds an XDG item for the given permanent ID.
		 *
		 * @param[in] permanentID The permanent ID of the item as returned
		 * by Item::GetPermanentID().
		 * @return The item matching the \em permanentID or a null pointer
		 * if there is no such item.
		 */
		Item_ptr FindItem (const QString& permanentID) const;
	public slots:
		/** @brief Updates the list of items.
		 *
		 * If IsReady() returns false, and thus no items are known to the
		 * finder yet, this function blocks and emits itemsListChanged()
		 * before returning.
		 *
		 * Otherwise, this function spawns an asynchronous update process.
		 */
		void update ();
	signals:
		/** @brief Notifies when the list of items changes in any way.
		 */
		void itemsListChanged ();
	};
}
