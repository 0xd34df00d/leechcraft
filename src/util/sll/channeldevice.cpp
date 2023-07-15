/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channeldevice.h"
#include <cstring>

namespace LC::Util
{
	bool ChannelDevice::isSequential () const
	{
		return true;
	}

	bool ChannelDevice::atEnd () const
	{
		if (!Finished_)
			return false;

		const std::lock_guard g { Mutex_ };
		return Chunks_.empty ();
	}

	void ChannelDevice::FinishWrite ()
	{
		Finished_ = true;
	}

	qint64 ChannelDevice::readData (char *data, qint64 maxSize)
	{
		const std::lock_guard g { Mutex_ };

		qint64 read = 0;
		while (!Chunks_.empty () && maxSize)
		{
			auto& chunk = Chunks_.front ();
			const auto toCopy = std::min<qint64> (chunk.size (), maxSize);
			std::memcpy (data, chunk.constData (), toCopy);
			if (chunk.size () < maxSize)
				Chunks_.pop_front ();
			else
				chunk.remove (0, maxSize);

			data += toCopy;
			maxSize -= toCopy;
			read += toCopy;
		}
		return read;
	}

	qint64 ChannelDevice::writeData (const char *data, qint64 maxSize)
	{
		{
			const std::lock_guard g { Mutex_ };
			Chunks_.emplace_back (data, maxSize);
		}

		emit readyRead ();

		return maxSize;
	}
}
