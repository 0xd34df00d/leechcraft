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

#ifndef PLUGINS_AGGREGATOR_ITEMBUCKET_H
#define PLUGINS_AGGREGATOR_ITEMBUCKET_H
#include <QDialog>
#include "item.h"
#include "ui_itembucket.h"

class QModelIndex;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemModel;

			class ItemBucket : public QDialog
			{
				Q_OBJECT

				ItemModel *Model_;
				Ui::ItemBucket Ui_;
			public:
				ItemBucket (QWidget* = 0);
				virtual ~ItemBucket ();
			private slots:
				void on_Items__activated (const QModelIndex&);
				void on_ActionDeleteItem__triggered ();
				void currentItemChanged (const QModelIndex&);
			};
		};
	};
};

#endif

