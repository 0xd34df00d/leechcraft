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

#include "ipfilterdialog.h"
#include "core.h"
#include "banpeersdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			const int BlockRole = 101;

			IPFilterDialog::IPFilterDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				QMap<Core::BanRange_t, bool> filter = Core::Instance ()->GetFilter ();
				QList<Core::BanRange_t> keys = filter.keys ();
				Q_FOREACH (Core::BanRange_t key, keys)
				{
					QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Tree_);
					item->setText (0, key.first);
					item->setText (1, key.second);
					bool block = filter [key];
					item->setText (2, block ?
							tr ("block") :
							tr ("allow"));
					item->setData (2, BlockRole, block);
				}

				on_Tree__currentItemChanged (0);
			}

			QMap<Core::BanRange_t, bool> IPFilterDialog::GetFilter () const
			{
				QMap<Core::BanRange_t, bool> result;
				for (int i = 0, size = Ui_.Tree_->topLevelItemCount (); i < size; ++i)
				{
					QTreeWidgetItem *item = Ui_.Tree_->topLevelItem (i);
					result [qMakePair (item->text (0), item->text (1))] =
						item->data (2, BlockRole).toBool ();
				}
				return result;
			}

			void IPFilterDialog::on_Tree__currentItemChanged (QTreeWidgetItem *current)
			{
				Ui_.Modify_->setEnabled (current);
				Ui_.Remove_->setEnabled (current);
			}

			void IPFilterDialog::on_Tree__itemClicked (QTreeWidgetItem *item, int column)
			{
				if (column != 2)
					return;

				bool block = !item->data (2, BlockRole).toBool ();
				item->setData (2, BlockRole, block);
				item->setText (2, block ?
						tr ("block") :
						tr ("allow"));
			}

			void IPFilterDialog::on_Add__released ()
			{
				BanPeersDialog dia;
				if (dia.exec () != QDialog::Accepted)
					return;

				QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Tree_);
				item->setText (0, dia.GetStart ());
				item->setText (1, dia.GetEnd ());
				item->setText (2, tr ("block"));
				item->setData (2, BlockRole, true);
			}

			void IPFilterDialog::on_Modify__released ()
			{
				BanPeersDialog dia;
				QTreeWidgetItem *item = Ui_.Tree_->currentItem ();
				dia.SetIP (item->text (0), item->text (1));
				if (dia.exec () != QDialog::Accepted)
					return;

				item->setText (0, dia.GetStart ());
				item->setText (1, dia.GetEnd ());
			}

			void IPFilterDialog::on_Remove__released ()
			{
				delete Ui_.Tree_->currentItem ();
			}
		};
	};
};

