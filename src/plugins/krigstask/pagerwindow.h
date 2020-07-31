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

class QScreen;

namespace LC
{
namespace Util
{
	class SettableIconProvider;
}

namespace Krigstask
{
	class DesktopsModel;
	class SingleDesktopModel;
	class ImageProvider;

	class PagerWindow : public QQuickWidget
	{
		Q_OBJECT

		DesktopsModel * const DesktopsModel_;
		const bool ShowThumbs_;

		Util::SettableIconProvider *WinIconProv_;
		ImageProvider *WinSnapshotProv_;
	public:
		PagerWindow (const QScreen*, bool, ICoreProxy_ptr, QWidget* = 0);
	private:
		void FillModel ();
		void FillSubmodel (SingleDesktopModel*, const QList<ulong>&, ulong);
	public slots:
		void showDesktop (int);
		void showWindow (qulonglong);
	};
}
}
