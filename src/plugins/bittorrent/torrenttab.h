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

namespace LC::BitTorrent
{
	class ListActions;
	class SessionHolder;
	class TabViewProxyModel;

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

		TabViewProxyModel *ViewFilter_;
	public:
		TorrentTab (SessionHolder&, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		QToolBar* GetToolBar () const override;
		void Remove () override;

		void SetCurrentTorrent (int);
	signals:
		void removeTab (QWidget*);
	};
}
