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
		QVector<QByteArray> consuming;

		{
			const std::lock_guard g { Mutex_ };
			consuming.reserve (Chunks_.size ());

			while (!Chunks_.empty ())
			{
				auto& chunk = Chunks_.front ();
				if (chunk.size () <= maxSize)
				{
					maxSize -= chunk.size ();
					consuming.push_back (std::move (chunk));
					Chunks_.pop_front ();
				}
				else
				{
					consuming.push_back (chunk.left (maxSize));
					chunk.remove (0, maxSize);
					break;
				}
			}
		}

		qint64 read = 0;
		for (const auto& chunk : consuming)
		{
			std::memcpy (data + read, chunk.constData (), chunk.size ());
			read += chunk.size ();
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
