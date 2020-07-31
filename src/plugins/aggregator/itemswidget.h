/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_itemswidget.h"
#include "item.h"
#include "channel.h"

class QModelIndex;
class QToolBar;
class IWebBrowser;
class QAbstractItemModel;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Aggregator
{
	struct ItemsWidget_Impl;
	struct ChannelActions;
	struct AppWideActions;
	class Aggregator;
	class ItemsFilterModel;

	struct ItemsWidgetDependencies
	{
		Util::ShortcutManager *ShortcutsMgr_;
		QAbstractItemModel *ChannelsModel_;
		const AppWideActions& AppWideActions_;
		const ChannelActions& ChannelActions_;
		std::function<void (QString, QStringList)> FeedAdder_;
	};

	class ItemsWidget : public QWidget
	{
		Q_OBJECT

		friend class Aggregator;
		ItemsWidget_Impl *Impl_;
	public:
		enum class Action
		{
			MarkAsRead,
			MarkAsUnread,
			MarkAsImportant,
			PrevUnreadItem,
			PrevItem,
			NextItem,
			NextUnreadItem,
			Delete,
			OpenLink,
			CopyLink,

			MaxAction
		};

		using Dependencies = ItemsWidgetDependencies;

		explicit ItemsWidget (QWidget* = nullptr);
		~ItemsWidget () override;

		void InjectDependencies (const Dependencies&);

		Item GetItem (const QModelIndex&) const;
		QToolBar* GetToolBar () const;
		void SetTapeMode (bool);

		QAction* GetAction (Action) const;

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
		void Selected (const QModelIndex&);
		bool IsItemRead (int) const;
		bool IsItemReadNotCurrent (int) const;
		QStringList GetItemCategories (int) const;
		IDType_t GetItemIDFromRow (int) const;
		void SubscribeToComments (const QModelIndex&) const;
		void CurrentChannelChanged (const QModelIndex&);

		void ConstructBrowser ();
		void LoadUIState ();
		void SaveUIState ();
	private:
		void MarkItemReadStatus (const QModelIndex&, bool);
		void ClearSupplementaryModels ();
		void AddSupplementaryModelFor (IDType_t);
		void SetupActions ();
		QToolBar* SetupToolBar ();
		QString GetHex (QPalette::ColorRole,
				QPalette::ColorGroup = QApplication::palette ().currentColorGroup ());
		QString ToHtml (const Item&);
		void RestoreSplitter ();
		QList<QPersistentModelIndex> GetSelected () const;
	private slots:
		void invalidateMergeMode ();
		void on_ActionHideReadItems__triggered ();
		void on_ActionShowAsTape__triggered ();
		void on_ActionMarkItemAsUnread__triggered ();
		void on_ActionMarkItemAsRead__triggered ();
		void on_ActionMarkItemAsImportant__triggered ();
		void on_ActionDeleteItem__triggered ();

		void on_ActionPrevUnreadItem__triggered ();
		void on_ActionPrevItem__triggered ();
		void on_ActionNextItem__triggered ();
		void on_ActionNextUnreadItem__triggered ();

		void on_CaseSensitiveSearch__stateChanged (int);
		void on_ActionItemCommentsSubscribe__triggered ();
		void on_ActionItemLinkOpen__triggered ();
		void on_ActionItemLinkCopy__triggered ();

		void on_CategoriesSplitter__splitterMoved ();
		void currentItemChanged ();
		void checkSelected ();
		void makeCurrentItemVisible ();
		void updateItemsFilter ();
		void selectorVisiblityChanged ();
		void navBarVisibilityChanged ();
	signals:
		void movedToChannel (const QModelIndex&);
	};
}
}
