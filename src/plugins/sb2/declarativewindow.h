/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariantMap>
#include <QQuickWidget>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace SB2
{
	class ViewManager;

	class DeclarativeWindow : public QQuickWidget
	{
		Q_OBJECT
	public:
		DeclarativeWindow (const QUrl&, QVariantMap, const QPoint&, ViewManager*, ICoreProxy_ptr, QWidget* = 0);
	public slots:
		void beforeDelete ();
	};
}
}
