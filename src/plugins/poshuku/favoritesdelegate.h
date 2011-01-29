/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_FAVORITESDELEGATE_H
#define PLUGINS_POSHUKU_FAVORITESDELEGATE_H
#include <memory>
#include <QItemDelegate>
#include <plugininterface/tagscompleter.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FavoritesDelegate : public QItemDelegate
			{
				Q_OBJECT

				mutable std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsCompleter_;
			public:
				FavoritesDelegate (QObject* = 0);

				QWidget* createEditor (QWidget*, const QStyleOptionViewItem&,
						const QModelIndex&) const;
				void setEditorData (QWidget*, const QModelIndex&) const;
				void setModelData (QWidget*, QAbstractItemModel*,
						const QModelIndex&) const;
				void updateEditorGeometry (QWidget*, const QStyleOptionViewItem&,
						const QModelIndex&) const;
			};
		};
	};
};

#endif

