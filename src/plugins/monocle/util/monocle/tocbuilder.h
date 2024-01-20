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
class QTextCursor;

namespace LC::Monocle
{
	class IDocument;

	class TocBuilder
	{
		const QTextCursor& Cursor_;

		TOCEntry Root_;

		QStack<TOCEntry*> CurrentSectionPath_;
	public:
		explicit TocBuilder (const QTextCursor& cursor);

		TOCEntryLevel_t GetTOC () const;
		[[nodiscard]] Util::DefaultScopeGuard HandleElem (const QDomElement&);
	};

}
