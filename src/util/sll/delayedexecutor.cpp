/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "delayedexecutor.h"
#include <QTimer>

namespace LC
{
namespace Util
{
	DelayedExecutor::DelayedExecutor (Actor_f actor, int timeout, QObject *parent)
	: QObject (parent)
	, Actor_ (actor)
	{
		QTimer::singleShot (timeout,
				this,
				SLOT (handleTimeout ()));
	}

	void DelayedExecutor::handleTimeout ()
	{
		Actor_ ();
		deleteLater ();
	}
}
}
