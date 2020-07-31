/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

class QDomDocument;

namespace LC
{
namespace Aggregator
{
	class Parser;

	class ParserFactory
	{
		QList<Parser*> Parsers_;

		ParserFactory () = default;
	public:
		static ParserFactory& Instance ();

		void Register (Parser*);
		void RegisterDefaultParsers ();

		Parser* Return (const QDomDocument&) const;
	};
}
}
