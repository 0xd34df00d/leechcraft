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
#include "channel.h"

class QModelIndex;
class QToolBar;
class IWebBrowser;
class QSortFilterProxyModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct ItemsWidget_Impl;
			struct ChannelActions;
			class Aggregator;
			class ItemsFilterModel;

			class ItemsWidget : public QWidget
			{
				Q_OBJECT

				friend class Aggregator;
				ItemsWidget_Impl *Impl_;
			public:
				ItemsWidget (QWidget* = 0);
				virtual ~ItemsWidget ();

				void SetChannelActions (const ChannelActions&);
				void SetChannelsFilter (QSortFilterProxyModel*);

				Item_ptr GetItem (const QModelIndex&) const;
				QToolBar* GetToolBar () const;
				void SetTapeMode (bool);

				/** Merge all that channels that are currently shown.
				 *
				 * @param[in] on Whether the merge mode should be enabled.
				 */
				void SetMergeMode (bool on);

				/** Merge only those channels that match given tags.
				 *
				 * To disable the merge mode enabled by this, either enable
				 * "usual" merge mode via SetMergeMode() or do a
				 * CurrentChannelChanged().
				 *
				 * If "usual" merge mode (as activated by SetMergeMode())
				 * is currently active, this function does nothing.
				 *
				 * @param[in] tags The list of tags to merge.
				 */
				void SetMergeModeTags (const QStringList& tags);
				void SetHideRead (bool);
				bool IsItemCurrent (int) const;
				void Selected (const QModelIndex&);
				void MarkItemAsUnread (const QModelIndex&);
				bool IsItemRead (int) const;
				bool IsItemReadNotCurrent (int) const;
				QStringList GetItemCategories (int) const;
				void AddToItemBucket (const QModelIndex&) const;
				void SubscribeToComments (const QModelIndex&) const;
				void CurrentChannelChanged (const QModelIndex&);
			private:
				void ClearSupplementaryModels ();
				void AddSupplementaryModelFor (const ChannelShort&);
				void SetupActions ();
				QToolBar* SetupToolBar ();
				QString GetHex (QPalette::ColorRole,
						QPalette::ColorGroup = QApplication::palette ().currentColorGroup ());
				QString ToHtml (const Item_ptr&);
				void RestoreSplitter ();
			public slots:
				void handleItemDataUpdated (Item_ptr, Channel_ptr);
			private slots:
				void invalidateMergeMode ();
				void channelChanged (const QModelIndex&);
				void on_ActionHideReadItems__triggered ();
				void on_ActionMarkItemAsUnread__triggered ();
				void on_CaseSensitiveSearch__stateChanged (int);
				void on_ActionAddToItemBucket__triggered ();
				void on_ActionItemCommentsSubscribe__triggered ();
				void on_CategoriesSplitter__splitterMoved ();
				void currentItemChanged (const QItemSelection&);
				void makeCurrentItemVisible ();
				void updateItemsFilter ();
				void selectorVisiblityChanged ();
				void navBarVisibilityChanged ();
			signals:
				void currentChannelChanged (const QModelIndex&);
			};
		};
	};
};

#endif

