/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStack>
#include <interfaces/monocle/ihavetoc.h>
#include <util/sll/util.h>

class QDomElement;

namespace LC::Monocle::FXB
{
	class TocBuilder
	{
		TOCEntryID Root_;
		QStack<TOCEntryID*> CurrentEntryPath_;
		int IdCounter_ = 0;
	public:
		explicit TocBuilder ();

		TOCEntryID GetToc () const;

		[[nodiscard]] Util::DefaultScopeGuard HandleElem (QDomElement);
	};
}
