/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ihavetoc.h>
#include <util/monocle/types.h>

class QDomElement;
class QTextDocument;

namespace LC::Monocle
{
	struct InternalLink;

	struct DocStructure
	{
		TOCEntryLevelT<Span> TOC_;
		QVector<InternalLink> InternalLinks_;
	};

	DocStructure Html2Doc (QTextDocument& doc, const QDomElement&, const CustomStyler_f&);
}
