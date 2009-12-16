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

#include "favoritesdelegate.h"
#include <plugininterface/tagslineedit.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "filtermodel.h"
#include "favoritesmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::TagsLineEdit;
			
			FavoritesDelegate::FavoritesDelegate (QObject *parent)
			: QItemDelegate (parent)
			{
			}
			
			QWidget* FavoritesDelegate::createEditor (QWidget *parent,
					const QStyleOptionViewItem& opt, const QModelIndex& index) const
			{
				if (index.column () != FavoritesModel::ColumnTags)
					return QItemDelegate::createEditor (parent, opt, index);
			
				TagsLineEdit *tle = new TagsLineEdit (parent);
				TagsCompleter_.reset (new TagsCompleter (tle));
				tle->AddSelector ();
				return tle;
			}
			
			void FavoritesDelegate::setEditorData (QWidget *editor,
					const QModelIndex& index) const
			{
				if (index.column () != FavoritesModel::ColumnTags)
				{
					QItemDelegate::setEditorData (editor, index);
					return;
				}
			
				QStringList tags = Core::Instance ().GetFavoritesModel ()->
					data (index, LeechCraft::RoleTags).toStringList ();

				QStringList user;
				Q_FOREACH (QString id, tags)
					user.append (Core::Instance ().GetProxy ()->GetTagsManager ()->
							GetTag (id));

				static_cast<TagsLineEdit*> (editor)->
					setText (Core::Instance ().GetProxy ()->
							GetTagsManager ()->Join (user));
			}
			
			void FavoritesDelegate::setModelData (QWidget *editor,
					QAbstractItemModel *model, const QModelIndex& index) const
			{
				if (index.column () != FavoritesModel::ColumnTags)
				{
					QItemDelegate::setModelData (editor, model, index);
					return;
				}
			
				QStringList tags = Core::Instance ().GetProxy ()->GetTagsManager ()->
					Split (static_cast<TagsLineEdit*> (editor)->text ());
				model->setData (index, tags);
			}
			
			void FavoritesDelegate::updateEditorGeometry (QWidget *editor,
					const QStyleOptionViewItem& option,
					const QModelIndex& index) const
			{
				if (index.column () != FavoritesModel::ColumnTags)
				{
					QItemDelegate::updateEditorGeometry (editor, option, index);
					return;
				}
			
				editor->setGeometry (option.rect);
			}
		};
	};
};

