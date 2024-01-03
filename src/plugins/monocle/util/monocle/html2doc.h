/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ihavetoc.h>

class QDomElement;
class QTextDocument;

namespace LC::Monocle
{
	class IDocument;
	struct InternalLink;

	struct DocStructure
	{
		TOCEntryLevel_t TOC_;
		QVector<InternalLink> InternalLinks_;
	};

	DocStructure Html2Doc (QTextDocument& doc, const QDomElement&, IDocument&);
}
