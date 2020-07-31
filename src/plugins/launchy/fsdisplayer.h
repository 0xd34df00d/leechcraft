/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>
#include <util/xdg/xdgfwd.h>

class QStandardItem;
class QStandardItemModel;

class QQuickWidget;

namespace LC
{
struct Entity;

namespace Launchy
{
	class ItemIconsProvider;
	class ItemsSortFilterProxyModel;
	class FavoritesManager;
	class RecentManager;
	class SysPathItemProvider;

	class FSDisplayer : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QString appFilterText READ GetAppFilterText WRITE SetAppFilterText NOTIFY appFilterTextChanged)

		const ICoreProxy_ptr Proxy_;
		Util::XDG::ItemsFinder * const Finder_;
		FavoritesManager * const FavManager_;
		RecentManager * const RecentManager_;

		QStandardItemModel * const CatsModel_;
		QStandardItemModel * const ItemsModel_;
		ItemsSortFilterProxyModel * const ItemsProxyModel_;

		const std::shared_ptr<QQuickWidget> View_;
		ItemIconsProvider * const IconsProvider_;

		SysPathItemProvider * const SysPathHandler_;
	public:
		FSDisplayer (ICoreProxy_ptr, Util::XDG::ItemsFinder*,
				FavoritesManager*, RecentManager*, QObject* = nullptr);

		QString GetAppFilterText () const;
		void SetAppFilterText (const QString&);
	private:
		void MakeStdCategories ();
		void MakeStdItems ();
		void MakeCategories (const QStringList&);
		void MakeItems (const QList<QList<Util::XDG::Item_ptr>>&);
		QStandardItem* FindItem (const QString&) const;
	private slots:
		void handleFinderUpdated ();
		void handleCategorySelected (int);
		void handleExecRequested (const QString&);
		void handleItemBookmark (const QString&);
	signals:
		void appFilterTextChanged ();
	};
}
}
