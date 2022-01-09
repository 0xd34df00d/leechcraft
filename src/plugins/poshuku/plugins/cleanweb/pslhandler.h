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

class QUrl;

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
		explicit PslHandler (QStringView fileContents);

		int GetTldCount (const QUrl&) const;
		int GetTldCountHost (QStringView) const;
	private:
		static Util::StringPathTrie<Kind> ParseFile (QStringView);
	};
}
