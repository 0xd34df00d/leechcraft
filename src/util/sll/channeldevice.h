/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <list>
#include <mutex>
#include <QIODevice>
#include "sllconfig.h"

namespace LC::Util
{
	class UTIL_SLL_API ChannelDevice : public QIODevice
	{
		mutable std::mutex Mutex_;
		std::list<QByteArray> Chunks_;

		std::atomic_bool Finished_ = false;
	public:
		using QIODevice::QIODevice;

		bool isSequential () const override;
		bool atEnd () const override;

		void FinishWrite ();
	protected:
		qint64 readData (char*, qint64) override;
		qint64 writeData (const char*, qint64) override;
	};
}
