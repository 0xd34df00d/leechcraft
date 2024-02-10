/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ihavetoc.h>
#include "types.h"

class QDomElement;
class QTextCursor;

namespace LC::Monocle
{
	class TocBuilder
	{
		const QTextCursor& Cursor_;

		TOCEntryT<Span> Root_;
		QHash<QByteArray, TOCEntryT<Span>*> Id2Entry_;
	public:
		explicit TocBuilder (const TOCEntryID& tocStructure, const QTextCursor& cursor);

		TOCEntryLevelT<Span> GetTOC () const;
		void HandleElem (const QDomElement&);
	};
}
