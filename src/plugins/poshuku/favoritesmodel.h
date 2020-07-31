/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <interfaces/iinfo.h>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Poshuku
{
	class FavoritesModel : public QAbstractItemModel
	{
		Q_OBJECT

		QStringList ItemHeaders_;
	public:
		struct FavoritesItem
		{
			QString Title_;
			QString URL_;
			/// Contains ids of the real tags.
			QStringList Tags_;

			bool operator== (const FavoritesItem&) const;
		};
		typedef QList<FavoritesItem> items_t;
	private:
		items_t Items_;
		QMap<QString, QString> CheckResults_;
	public:
		enum Columns
		{
			ColumnTitle
			, ColumnURL
			, ColumnTags
		};

		FavoritesModel (QObject* = nullptr);

		void HandleStorageReady ();

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		Qt::ItemFlags flags (const QModelIndex&) const;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;
		bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole);

		Qt::DropActions supportedDropActions () const;
		QStringList mimeTypes () const;
		QMimeData* mimeData (const QModelIndexList& indexes) const;
		bool dropMimeData (const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

		void EditBookmark (const QModelIndex&);
		void ChangeURL (const QModelIndex&, const QString&);
		const items_t& GetItems () const;
		void SetCheckResults (const QMap<QString, QString>&);

		bool IsUrlExists (const QString&) const;
	private:
		QStringList GetVisibleTags (int) const;
		FavoritesItem GetItemFromUrl (const QString& url);
	public slots:
		QModelIndex addItem (const QString&, const QString&, const QStringList&);
		QList<QVariant> getItemsMap () const;
		void removeItem (const QModelIndex&);
		void removeItem (const QString&);
		void handleItemAdded (const FavoritesModel::FavoritesItem&);
		void handleItemUpdated (const FavoritesModel::FavoritesItem&);
		void handleItemRemoved (const FavoritesModel::FavoritesItem&);
	private slots:
		void loadData ();
	signals:
		void error (const QString&);

		// Hook support
		void hookAddedToFavorites (LC::IHookProxy_ptr,
				QString title, QString url, QStringList tags);
	};
}
}
