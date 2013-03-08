/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_FAVORITESMODEL_H
#define PLUGINS_POSHUKU_FAVORITESMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <interfaces/iinfo.h>
#include <interfaces/core/ihookproxy.h>

namespace LeechCraft
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

		FavoritesModel (QObject* = 0);
		~FavoritesModel ();

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
		void hookAddedToFavorites (LeechCraft::IHookProxy_ptr,
				QString title, QString url, QStringList tags);
	};
}
}

#endif
