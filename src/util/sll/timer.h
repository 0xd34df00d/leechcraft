/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <source_location>
#include <QElapsedTimer>
#include <QStringView>
#include "sllconfig.h"

namespace LC::Util
{
	class UTIL_SLL_API Timer
	{
		QElapsedTimer Timer_;
	public:
		explicit Timer ();

		Timer (const Timer&) = delete;
		Timer (Timer&&) = delete;

		Timer& operator= (const Timer&) = delete;
		Timer& operator= (Timer&&) = delete;

		void Stamp (const char *context, std::source_location loc = std::source_location::current ());
		void Stamp (QStringView context, std::source_location loc = std::source_location::current ());
	private:
		void RunStamp (auto&& context, std::source_location loc);
	};
}
