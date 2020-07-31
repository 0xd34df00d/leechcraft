/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	/** @brief Interface for accounts that may have blacklists.
	 */
	class IHaveBlacklists
	{
	protected:
		virtual ~IHaveBlacklists () = default;
	public:
		/** @brief Checks whether this exact account supports blacklists.
		 *
		 * @return Whether blacklists are supported by this account.
		 */
		virtual bool SupportsBlacklists () const = 0;

		/** @brief Suggests adding the given \em entries to the blacklist.
		 *
		 * It's up to the protocol plugin to show any additional UI that
		 * is required to handle this suggestion.
		 *
		 * @param[in] entries The list of entries to add to the blacklist.
		 */
		virtual void SuggestToBlacklist (const QList<ICLEntry*>& entries) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveBlacklists,
		"org.LeechCraft.Azoth.IHaveBlacklists/1.0")
