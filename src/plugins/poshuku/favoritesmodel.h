/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
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
				typedef std::vector<FavoritesItem> items_t;
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
				virtual ~FavoritesModel ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation,
						int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int,
						const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
				virtual bool setData (const QModelIndex&, const QVariant&,
						int = Qt::EditRole);

				bool AddItem (const QString&, const QString&, const QStringList&);
				void ChangeURL (const QModelIndex&, const QString&);
				const items_t& GetItems () const;
				void SetCheckResults (const QMap<QString, QString>&);
			private:
				QStringList GetVisibleTags (int) const;
			public slots:
				void removeItem (const QModelIndex&);
				void handleItemAdded (const FavoritesModel::FavoritesItem&);
				void handleItemUpdated (const FavoritesModel::FavoritesItem&);
				void handleItemRemoved (const FavoritesModel::FavoritesItem&);
			private slots:
				void loadData ();
			signals:
				void error (const QString&);
			};
		};
	};
};

#endif

