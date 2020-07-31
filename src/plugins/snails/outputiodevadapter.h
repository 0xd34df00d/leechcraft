/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vmime/utility/outputStream.hpp>

class QIODevice;

namespace LC
{
namespace Snails
{
	class OutputIODevAdapter : public vmime::utility::outputStream
	{
		QIODevice *Dev_;
	public:
		OutputIODevAdapter (QIODevice*);

		void flush ();
	protected:
		void writeImpl (const vmime::byte_t* const, const size_t);
	};
}
}
