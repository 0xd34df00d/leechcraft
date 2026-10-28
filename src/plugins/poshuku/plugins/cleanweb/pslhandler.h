/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/sll/stringpathtrie.h>

namespace LC::Poshuku::CleanWeb
{
	class PslHandler
	{
		enum class Kind
		{
			Rule,
			Exception
		};
		const Util::StringPathTrie<Kind> Tlds_;
	public:
		explicit PslHandler (QStringView);

		int GetTldCountHost (QStringView) const;

		/** @brief Returns the registrable domain (the public suffix plus one label) of the `host`.
		 *
		 * The result is lowercase, without empty labels, so two hosts belong to the same site iff
		 * their registrable domains are equal. A host that is itself a public suffix is returned whole.
		 */
		QString GetRegistrableDomain (QStringView host) const;
	private:
		static Util::StringPathTrie<Kind> ParseFile (QStringView);
	};
}
