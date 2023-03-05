/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QThread>

namespace LC::Util
{
	template<typename F>
	void InObjectThread (QObject& object, F&& function)
	{
		if (object.thread () == QThread::currentThread ())
			std::forward<F> (function) ();
		else
			QMetaObject::invokeMethod (&object, std::forward<F> (function), Qt::QueuedConnection);
	}
}
