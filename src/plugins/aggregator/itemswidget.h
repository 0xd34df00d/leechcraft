/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "common.h"

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
	class ChannelActions;
	class AppWideActions;
	class Aggregator;
	class ItemsFilterModel;
	class UpdatesManager;
	class ItemActions;
	class ItemSelectionTracker;

	struct ItemsWidgetDependencies
	{
		Util::ShortcutManager& ShortcutsMgr_;
		QAbstractItemModel& ChannelsModel_;
		const AppWideActions& AppWideActions_;
		const ChannelActions& ChannelActions_;
		UpdatesManager& UpdatesManager_;
	};

	class ItemsWidget : public QWidget
	{
		Q_OBJECT

		friend class Aggregator;
		ItemsWidget_Impl *Impl_;

		std::unique_ptr<ItemActions> Actions_;
		std::unique_ptr<ItemSelectionTracker> SelectionTracker_;
	public:
		using Dependencies = ItemsWidgetDependencies;

		explicit ItemsWidget (const Dependencies&, QWidget* = nullptr);
		~ItemsWidget () override;

		QToolBar* GetToolBar () const;
		void SetTapeMode (bool);

		QModelIndex GetUnfilteredSelectedIndex () const;

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
		void Selected (const QModelIndex&);
		void CurrentChannelChanged (const QModelIndex&);

		void ConstructBrowser ();
	private:
		void ClearSupplementaryModels ();
		void AddSupplementaryModelFor (IDType_t);
	private slots:
		void invalidateMergeMode ();

		void on_CaseSensitiveSearch__stateChanged (int);

		void currentItemChanged ();
		void checkSelected ();
		void makeCurrentItemVisible ();
		void updateItemsFilter ();
	signals:
		void movedToChannel (const QModelIndex&);
	};
}
}
