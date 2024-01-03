/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <util/sll/util.h>
#include "types.h"

class QDomElement;
class QTextCursor;

namespace LC::Monocle
{
	class LinksBuilder
	{
		const QTextCursor& Cursor_;

		QHash<QString, Span> Sources_;
		QHash<QString, Span> Targets_;
	public:
		explicit LinksBuilder (const QTextCursor&);

		[[nodiscard]] Util::DefaultScopeGuard HandleElem (const QDomElement& elem);

		QVector<InternalLink> GetLinks () const;
	private:
		Util::DefaultScopeGuard HandleTarget (const QString&);
		Util::DefaultScopeGuard HandleLink (const QDomElement&);
	};

}
