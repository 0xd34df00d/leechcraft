/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chunkiodevice.h"

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	ChunkIODevice::ChunkIODevice (const QString& path, QObject *parent)
	: QIODevice (parent)
	, ChunkSize_ (4 * 1024 * 1024)
	{
		File_.setFileName (path);
	}

	bool ChunkIODevice::atEnd () const
	{
		return File_.atEnd ();
	}

	bool ChunkIODevice::open (QIODevice::OpenMode mode)
	{
		return File_.open (mode);
	}

	void ChunkIODevice::close ()
	{
		File_.close ();
	}

	QByteArray ChunkIODevice::GetNextChunk ()
	{
		return File_.read (ChunkSize_);
	}

	qint64 ChunkIODevice::readData (char *data, qint64 maxlen)
	{
		return File_.read (data, maxlen);
	}

	qint64 ChunkIODevice::writeData (const char *data, qint64 len)
	{
		return File_.write (data, len);
	}
}
}
}
