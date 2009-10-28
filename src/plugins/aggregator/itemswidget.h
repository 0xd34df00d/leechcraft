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

#ifndef PLUGINS_AGGREGATOR_ITEMSWIDGET_H
#define PLUGINS_AGGREGATOR_ITEMSWIDGET_H
#include <QWidget>
#include "ui_itemswidget.h"
#include "item.h"

class QModelIndex;
class IWebBrowser;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct ItemsWidget_Impl;

			class ItemsWidget : public QWidget
			{
				Q_OBJECT

				ItemsWidget_Impl *Impl_;
			public:
				ItemsWidget (QWidget* = 0);
				virtual ~ItemsWidget ();

				void SetTapeMode (bool);
				void SetHideRead (bool);
			private:
				void Construct (bool);
				QString GetHex (QPalette::ColorRole,
						QPalette::ColorGroup = QApplication::palette ().currentColorGroup ());
				QString ToHtml (const Item_ptr&);
				void RestoreSplitter ();
			private slots:
				void channelChanged (const QModelIndex&);
				void on_ActionMarkItemAsUnread__triggered ();
				void on_CaseSensitiveSearch__stateChanged (int);
				void on_ActionAddToItemBucket__triggered ();
				void on_ActionItemCommentsSubscribe__triggered ();
				void on_CategoriesSplitter__splitterMoved ();
				void currentItemChanged (const QItemSelection&);
				void makeCurrentItemVisible ();
				void updateItemsFilter ();
				void selectorVisiblityChanged ();
			};
		};
	};
};

#endif

