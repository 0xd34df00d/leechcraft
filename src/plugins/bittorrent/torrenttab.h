/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_torrenttab.h"

namespace LC
{
namespace BitTorrent
{
	class SessionHolder;
	class ListActions;

	class TorrentTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::TorrentTab Ui_;

		const SessionHolder& Holder_;

		const TabClassInfo TC_;
		QObject *ParentMT_;

		ListActions * const Actions_;

		QSortFilterProxyModel *ViewFilter_;
	public:
		TorrentTab (SessionHolder&, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();

		void SetCurrentTorrent (int);
	signals:
		void removeTab (QWidget*);
	};
}
}
