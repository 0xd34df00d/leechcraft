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
class QMainWindow;
class QTimer;

namespace LC::Util
{
	class UnhoverDeleteMixin;
}

namespace LC::SB2
{
	class TabListView final : public QQuickWidget
	{
		Q_OBJECT

		ICoreTabWidget * const ICTW_;
		QMainWindow * const MW_;

		const QByteArray TC_;
		QStandardItemModel * const Model_;

		Util::UnhoverDeleteMixin *UnhoverDeleteMixin_;
	public:
		TabListView (QByteArray, const QList<QWidget*>&,
				ICoreTabWidget*, QMainWindow*, QWidget* = nullptr);
		~TabListView ();

		QByteArray GetTabClass () const;

		void HandleLauncherHovered ();
		void HandleLauncherUnhovered ();
	private slots:
		void handleTabRemoved (QWidget*);
		void switchToItem (int);
		void closeItem (int);
	};
}
