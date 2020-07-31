/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Krigstask
{
	class PagerWindow;

	class TaskbarProxy : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QPointer<PagerWindow> Pager_;
	public:
		TaskbarProxy (ICoreProxy_ptr, QObject* = 0);
	public slots:
		void raiseWindow (const QString&);
		void minimizeWindow (const QString&);
		void maximizeWindow (const QString&);
		void unmaximizeWindow (const QString&);

		void moveWindowTo (const QString&, const QString&);
		void toggleShadeWindow (const QString&);

		void moveToDesktop (const QString&, int);

		void closeWindow (const QString&);

		void showMenu (const QString&, int, int);
		void showPager (int, int, bool);
	private slots:
		void handleAction ();
	};
}
}
