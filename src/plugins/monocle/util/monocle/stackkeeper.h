/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>

namespace LC::Monocle
{
	template<typename T>
	class StackKeeper final
	{
		T& Current_;
		T Saved_;
		bool NeedRestore_ = false;
	public:
		StackKeeper (T& toSave)
		: Current_ { toSave }
		{
		}

		~StackKeeper ()
		{
			if (NeedRestore_)
				Current_ = Saved_;
		}

		void Save (const T& val)
		{
			Saved_ = val;
			NeedRestore_ = true;
		}

		void Set (T&& val)
		{
			Saved_ = Current_;
			Current_ = std::move (val);
			NeedRestore_ = true;
		}

		StackKeeper (const StackKeeper&) = delete;
		StackKeeper (StackKeeper&) = delete;
		StackKeeper& operator= (const StackKeeper&) = delete;
		StackKeeper& operator= (StackKeeper&) = delete;
	};
}
