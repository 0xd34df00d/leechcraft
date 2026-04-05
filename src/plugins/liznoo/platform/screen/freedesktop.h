/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "platform.h"

class QTimer;

namespace LC::Liznoo::Screen
{
	class Freedesktop : public Platform
	{
		Q_OBJECT

		QTimer * const ActivityTimer_;
	public:
		Freedesktop (QObject* = nullptr);

		void ProhibitScreensaver (bool prohibit, const QString& id) override;
	private slots:
		void handleTimeout ();
	};
}
