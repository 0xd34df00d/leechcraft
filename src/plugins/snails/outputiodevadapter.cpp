/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "outputiodevadapter.h"
#include <QIODevice>
#include <QFile>

namespace LC
{
namespace Snails
{
	OutputIODevAdapter::OutputIODevAdapter (QIODevice *dev)
	: Dev_ (dev)
	{
	}
	void OutputIODevAdapter::flush ()
	{
		if (const auto file = qobject_cast<QFile*> (Dev_))
			file->flush ();
	}

	void OutputIODevAdapter::writeImpl (const vmime::byte_t* const data, const size_t size)
	{
		Dev_->write (reinterpret_cast<const char*> (data), size);
	}
}
}
