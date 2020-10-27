/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickWidget>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;

namespace LC::SB2
{
	class ViewManager;

	class QuarkOrderView : public QQuickWidget
	{
		Q_OBJECT

		ViewManager * const Manager_;
		ICoreProxy_ptr Proxy_;

		QStandardItemModel *Model_;
	public:
		QuarkOrderView (ViewManager*, ICoreProxy_ptr, QWidget* = nullptr);
	private slots:
		void handleQuarkCloseRequested (const QString&);
		void moveQuark (const QString& from, const QString& to, int shift);
	signals:
		void quarkClassHovered (const QString&);
	};
}
