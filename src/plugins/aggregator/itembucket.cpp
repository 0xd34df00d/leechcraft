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

#include <QtDebug>
#include "core.h"
#include "itembucket.h"
#include "itemmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ItemBucket::ItemBucket (QWidget *parent)
			: QDialog (parent)
			{
				ItemModel_ = new ItemModel ();

				Ui_.setupUi (this);
				Ui_.Items_->setModel (ItemModel_);
				Ui_.Items_->addAction (Ui_.ActionDeleteItem_);
				Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
				Ui_.ItemView_->Construct (Core::Instance ().GetWebBrowser ());
			
				connect (Ui_.Items_->selectionModel (),
						SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (currentItemChanged (const QModelIndex&)));
			}
			
			ItemBucket::~ItemBucket ()
			{
				if (ItemModel_)
					ItemModel_->saveSettings ();
				delete ItemModel_;
			}

			ItemModel* ItemBucket::GetItemModel () const
			{
				return ItemModel_;
			}
			
			void ItemBucket::on_Items__activated (const QModelIndex& index)
			{
				ItemModel_->Activated (index);
			}
			
			void ItemBucket::on_ActionDeleteItem__triggered ()
			{
				QModelIndexList indexes = Ui_.Items_->selectionModel ()->selectedRows ();
				for (int i = 0; i < indexes.size (); ++i)
					ItemModel_->RemoveItem (indexes.at (i));
			}
			
			void ItemBucket::currentItemChanged (const QModelIndex& index)
			{
				Ui_.ItemView_->SetHtml (ItemModel_->GetDescription (index));
			}
		};
	};
};

