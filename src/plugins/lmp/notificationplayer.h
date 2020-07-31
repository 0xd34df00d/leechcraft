/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>

namespace LC
{
namespace LMP
{
	enum class SourceState;

	class NotificationPlayer : public QObject
	{
		Q_OBJECT
	public:
		NotificationPlayer (const QString&, const ICoreProxy_ptr&, QObject* = nullptr);
	private slots:
		void handleStateChanged (SourceState, SourceState);
	};
}
}
