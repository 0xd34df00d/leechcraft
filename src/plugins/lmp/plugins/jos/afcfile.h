/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIODevice>
#include <libimobiledevice/afc.h>

namespace LC
{
namespace LMP
{
namespace jOS
{
	class Connection;

	class AfcFile final : public QIODevice
	{
		const Connection * const Conn_;

		const afc_client_t AFC_;
		const QString Path_;

		uint64_t Handle_ = 0;
	public:
		AfcFile (const QString& path, Connection*, QObject* = nullptr);
		~AfcFile ();

		bool open (OpenMode mode) override;
		void close () override;
		bool seek (qint64 pos) override;
		qint64 size () const override;
	protected:
		qint64 readData (char*, qint64) override;
		qint64 writeData (const char*, qint64) override;
	};
}
}
}
