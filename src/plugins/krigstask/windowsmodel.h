/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QAbstractItemModel>
#include <QIcon>
#include <util/x11/winflags.h>
#include <util/models/rolenamesmixin.h>
class QQuickImageProvider;

namespace LC
{
namespace Util
{
class XWrapper;
}

namespace Krigstask
{
	class TaskbarImageProvider;

	class WindowsModel : public Util::RoleNamesMixin<QAbstractItemModel>
	{
		Q_OBJECT

		struct WinInfo
		{
			ulong WID_;

			QString Title_;
			QIcon Icon_;
			int IconGenID_;
			bool IsActive_;

			int DesktopNum_;

			Util::WinStateFlags State_;
			Util::AllowedActionFlags Actions_;
		};
		QList<WinInfo> Windows_;

		int CurrentDesktop_;

		enum Role
		{
			WindowName = Qt::UserRole + 1,
			WindowID,
			IconGenID,
			IsCurrentDesktop,
			IsActiveWindow,
			IsMinimizedWindow
		};

		TaskbarImageProvider *ImageProvider_;
	public:
		WindowsModel (QObject* = 0);

		QQuickImageProvider* GetImageProvider () const;

		int columnCount (const QModelIndex& parent = QModelIndex()) const;
		int rowCount (const QModelIndex& parent = QModelIndex()) const;
		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
		QModelIndex parent (const QModelIndex& child) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	private:
		void AddWindow (ulong, Util::XWrapper&);

		QList<WinInfo>::iterator FindWinInfo (ulong);
		void UpdateWinInfo (ulong, std::function<void (WinInfo&)>);
	private slots:
		void updateWinList ();
		void updateActiveWindow ();

		void updateWindowName (ulong);
		void updateWindowIcon (ulong);
		void updateWindowState (ulong);
		void updateWindowActions (ulong);
		void updateWindowDesktop (ulong);
		void updateCurrentDesktop ();
	};
}
}
