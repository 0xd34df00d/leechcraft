/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <util/xdg/xdgfwd.h>

class QStandardItem;
class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace Launchy
{
	class FavoritesManager;
	class ItemImageProvider;

	class QuarkManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		FavoritesManager *FavMgr_;
		Util::XDG::ItemsFinder *Finder_;
		ItemImageProvider *ImageProv_;

		QStandardItemModel *Model_;
	public:
		QuarkManager (ICoreProxy_ptr, FavoritesManager*,
				Util::XDG::ItemsFinder*, ItemImageProvider*, QObject* = nullptr);

		QAbstractItemModel* GetModel () const;
	private:
		QStandardItem* MakeItem (const QString&) const;
	public slots:
		void launch (const QString&);
		void remove (const QString&);
	private slots:
		void init ();
		void addItem (const QString&);
		void handleItemRemoved (const QString&);
	};
}
}
