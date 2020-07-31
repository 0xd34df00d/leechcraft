/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "screenplatform.h"

class QTimer;

namespace LC
{
namespace Liznoo
{
namespace Screen
{
	class Freedesktop : public ScreenPlatform
	{
		Q_OBJECT

		QTimer * const ActivityTimer_;
	public:
		Freedesktop (QObject* = nullptr);

		void ProhibitScreensaver (bool prohibit, const QString& id);
	private slots:
		void handleTimeout ();
	};
}
}
}
