/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/lmp/iunmountablesync.h>

template<typename T>
class QFutureWatcher;

namespace LC
{
namespace LMP
{
namespace jOS
{
	class DevManager : public QObject
	{
		Q_OBJECT

		UnmountableDevInfos_t Devices_;
		QFutureWatcher<UnmountableDevInfos_t> *PollWatcher_ = nullptr;
	public:
		DevManager (QObject* = 0);

		UnmountableDevInfos_t GetDevices () const;
	public slots:
		void refresh ();
		void handlePolled ();
	signals:
		void availableDevicesChanged ();
	};
}
}
}
