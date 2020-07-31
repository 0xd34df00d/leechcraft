/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "interfaces/poshuku/poshukutypes.h"

namespace LC
{
namespace Poshuku
{
	class IStorageBackend
	{
	public:
		virtual ~IStorageBackend () {}

		/** @brief Get all the history items from the storage.
		 *
		 * Puts all the history items (HistoryItem) from the
		 * storage backend into the passed container.
		 *
		 * @param[out] items The container with items. They would be
		 * appended to the container.
		 */
		virtual void LoadHistory (history_items_t& items) const = 0;
	};

	typedef std::shared_ptr<IStorageBackend> IStorageBackend_ptr;
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IStorageBackend,
		"org.LeechCraft.Poshuku.IStorageBackend/1.0")
