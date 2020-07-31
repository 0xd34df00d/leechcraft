/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace LMP
{
	class Path;

	class PathElement
	{
		friend class Path;
	protected:
		void AddToPathExposed (Path *p);
		void PostAddExposed (Path *p);

		virtual void AddToPath (Path*) = 0;
		virtual void PostAdd (Path*) = 0;
	};
}
}
