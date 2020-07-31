/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIODevice>
#include <QFile>

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	class ChunkIODevice : public QIODevice
	{
		Q_OBJECT

		QFile File_;
		const int ChunkSize_;

	public:
		ChunkIODevice (const QString& path, QObject *parent = 0);

		bool atEnd () const override;
		bool open (OpenMode mode) override;
		void close () override;

		QByteArray GetNextChunk ();
	protected:
		qint64 readData (char *data, qint64 maxlen) override;
		qint64 writeData (const char *data, qint64 len) override;
	};
}
}
}
