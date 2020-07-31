/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	struct Filter;

	class LineParser
	{
		Filter *Filter_;
		int Total_ = 0;
		int Success_ = 0;
	public:
		LineParser (Filter*);

		int GetTotal () const;
		int GetSuccess () const;

		void operator() (const QString&);
	};
}
}
}
