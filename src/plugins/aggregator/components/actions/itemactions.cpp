/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemactions.h"
#include <optional>
#include <QAction>
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>
#include <QModelIndex>
#include <interfaces/core/ientitymanager.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include "interfaces/aggregator/iitemsmodel.h"
#include "../../dbutils.h"
#include "storagebackendmanager.h"
#include "xmlsettingsmanager.h"
#include "actiondefshelpers.h"

namespace LC::Aggregator
{
	struct ItemActions::ActionInfo
	{
		std::optional<bool> Checked_ {};
	};

	MAKE_ACTIONS (ItemActions,
			MarkItemAsUnread,
			MarkItemAsRead,
			MarkItemAsUnimportant,
			MarkItemAsImportant,
			DeleteItem,
			ItemCommentsSubscribe,
			ItemLinkOpen,
			ItemLinkCopy,
			HideReadItems,
			ShowAsTape,
			PrevUnreadItem,
			NextUnreadItem,
			PrevItem,
			NextItem
	);

	namespace
	{
		ActionInfo MakeInfo (const QString& text, const QByteArray& icon, const QString& shortcut = {})
		{
			return { .Text_ = text, .Seq_ = shortcut, .Icon_ = icon };
		}

		ActionInfo GetActionInfo (ItemActions::ActionId action)
		{
			using enum ItemActions::ActionId;
			switch (action)
			{
			case MarkItemAsUnread:
				return MakeInfo (ItemActions::tr ("Mark as unread"), {}, "U"_qs);
			case MarkItemAsRead:
				return MakeInfo (ItemActions::tr ("Mark as read"), {}, "R"_qs);
			case MarkItemAsUnimportant:
				return MakeInfo (ItemActions::tr ("Unimportant"), "rating-unrated", "I"_qs);
			case MarkItemAsImportant:
				return MakeInfo (ItemActions::tr ("Important"), "rating", "Shift+I"_qs);
			case DeleteItem:
				return MakeInfo (ItemActions::tr ("Delete"), "remove", "Delete"_qs);
			case ItemCommentsSubscribe:
				return MakeInfo (ItemActions::tr ("Subscribe to comments"), "news-subscribe");
			case ItemLinkOpen:
				return MakeInfo (ItemActions::tr ("Open in new tab"), "internet-web-browser", "O"_qs);
			case ItemLinkCopy:
				return MakeInfo (ItemActions::tr ("Copy news item link"), "edit-copy", "C"_qs);
			case HideReadItems:
				return MakeInfo (ItemActions::tr ("Hide read items"), "mail-mark-unread");
			case ShowAsTape:
				return MakeInfo (ItemActions::tr ("Show items as tape"), "format-list-unordered");
			case PrevUnreadItem:
				return MakeInfo (ItemActions::tr ("Previous unread item"), "go-first", "Shift+K"_qs);
			case NextUnreadItem:
				return MakeInfo (ItemActions::tr ("Next unread item"), "go-last", "Shift+J"_qs);
			case PrevItem:
				return MakeInfo (ItemActions::tr ("Previous item"), "go-previous", "K"_qs);
			case NextItem:
				return MakeInfo (ItemActions::tr ("Next item"), "go-next", "J"_qs);
			}

			qWarning () << "unknown action" << static_cast<int> (action);
			return {};
		}
	}

	void ItemActions::RegisterActions (Util::ShortcutManager& sm)
	{
		for (const auto actionId : AllActionIds ())
			sm.RegisterActionInfo (ToString (actionId), GetActionInfo (actionId));
	}

	ItemActions::ItemActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, Deps_ { deps }
	{
		auto& xsm = XmlSettingsManager::Instance ();

		const auto mkSep = [&] { AllActions_ << Util::CreateSeparator (parent); };

		using enum ActionId;
		MarkUnread_ = MakeAction (MarkItemAsUnread, [this] { MarkSelectedReadStatus (false); });
		MarkRead_ = MakeAction (MarkItemAsRead, [this] { MarkSelectedReadStatus (true); });
		mkSep ();
		MarkUnimportant_ = MakeAction (MarkItemAsUnimportant, [this] { MarkSelectedAsImportant (false); });
		MarkImportant_ = MakeAction (MarkItemAsImportant, [this] { MarkSelectedAsImportant (true); });
		mkSep ();
		Delete_ = MakeAction (DeleteItem, [this] { DeleteSelected (); });
		mkSep ();
		SubComments_ = MakeAction (ItemCommentsSubscribe, [this] { SubscribeComments (); });
		LinkOpen_ = MakeAction (ItemLinkOpen, [this] { LinkOpen (); });
		LinkCopy_ = MakeAction (ItemLinkCopy, [this] { LinkCopy (); });

		mkSep ();

		ToolbarActions_ << MakeAction (HideReadItems,
				Deps_.SetHideRead_,
				{ .Checked_ = xsm.property ("HideReadItems").toBool () });
		ToolbarActions_ << MakeAction (ShowAsTape, deps.SetShowTape_,
				{ .Checked_ = xsm.Property ("ShowAsTape", false).toBool () });

		auto& nav = Deps_.ItemNavigator_;
		InvisibleActions_ << MakeAction (PrevUnreadItem, [&nav] { nav.MoveToPrevUnread (); });
		InvisibleActions_ << MakeAction (NextUnreadItem, [&nav] { nav.MoveToNextUnread (); });
		InvisibleActions_ << MakeAction (PrevItem, [&nav] { nav.MoveToPrev (); });
		InvisibleActions_ << MakeAction (NextItem, [&nav] { nav.MoveToNext (); });
		for (auto act : InvisibleActions_)
			AllActions_.removeOne (act);
	}

	QList<QAction*> ItemActions::GetAllActions () const
	{
		return AllActions_;
	}

	QList<QAction *> ItemActions::GetToolbarActions () const
	{
		return ToolbarActions_;
	}

	QList<QAction*> ItemActions::GetInvisibleActions () const
	{
		return InvisibleActions_;
	}

	void ItemActions::HandleSelectionChanged (const QList<QModelIndex>& selected)
	{
		bool hasImportant = false;
		bool hasUnimportant = false;
		bool hasRead = false;
		bool hasUnread = false;
		bool hasCommentsRss = false;

		for (const auto& idx : selected)
		{
			if (idx.data (IItemsModel::ItemRole::ItemImportant).toBool ())
				hasImportant = true;
			else
				hasUnimportant = true;

			if (idx.data (IItemsModel::ItemRole::IsRead).toBool ())
				hasRead = true;
			else
				hasUnread = true;

			if (hasImportant && hasUnimportant && hasRead && hasUnread && hasCommentsRss)
				break;

			const auto& item = idx.data (IItemsModel::ItemRole::FullItem).value<Item> ();
			if (item.CommentsLink_.isEmpty ())
				hasCommentsRss = true;
		}

		MarkUnimportant_->setEnabled (hasImportant);
		MarkImportant_->setEnabled (hasUnimportant);

		MarkUnread_->setEnabled (hasRead);
		MarkRead_->setEnabled (hasUnread);

		Delete_->setEnabled (!selected.isEmpty ());
		LinkOpen_->setEnabled (!selected.isEmpty ());
		LinkCopy_->setEnabled (!selected.isEmpty ());
		SubComments_->setEnabled (hasCommentsRss);
	}

	QAction* ItemActions::MakeAction (ActionId actionId,
			auto handler)
	{
		return MakeAction (actionId, handler, {});
	}

	QAction* ItemActions::MakeAction (ActionId actionId,
			auto handler,
			const ActionInfo& info)
	{
		const auto action = new QAction { parent () };
		if (info.Checked_)
		{
			action->setCheckable (true);
			action->setChecked (*info.Checked_);
		}
		connect (action,
				&QAction::triggered,
				this,
				handler);

		Deps_.ShortcutsMgr_.RegisterAction (ToString (actionId), action);
		AllActions_ << action;
		return action;
	}

	void ItemActions::MarkSelectedReadStatus (bool read)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& idx : Deps_.GetSelection_ ())
		{
			const auto channelId = idx.data (IItemsModel::ItemRole::ItemChannelId).value<IDType_t> ();
			const auto itemId = idx.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
			sb->SetItemUnread (channelId, itemId, !read);
		}
	}

	void ItemActions::MarkSelectedAsImportant (bool important)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const ITagsManager::tag_id impId = "_important"_qs;
		for (const auto itemId : GetSelectedIds ())
		{
			auto tags = sb->GetItemTags (itemId);
			if (important && !tags.contains (impId))
				sb->SetItemTags (itemId, tags + QStringList { impId });
			else if (!important && tags.removeAll (impId))
				sb->SetItemTags (itemId, tags);
		}
	}

	void ItemActions::DeleteSelected ()
	{
		const auto& ids = GetSelectedIds ();

		if (QMessageBox::warning (Deps_.Parent_,
					MessageBoxTitle,
					tr ("Are you sure you want to remove %n items?", nullptr, ids.size ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->RemoveItems ({ ids.begin (), ids.end () });
	}

	namespace
	{
		QStringList GetCommentTags ()
		{
			const auto& prop = XmlSettingsManager::Instance ().property ("CommentsTags").toString ();
			return GetProxyHolder ()->GetTagsManager ()->Split (prop);
		}
	}

	void ItemActions::SubscribeComments ()
	{
		const auto& commentTags = GetCommentTags ();

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& itemId : GetSelectedIds ())
		{
			const auto& item = sb->GetItem (itemId);
			if (!item)
			{
				qCritical () << "unable to get item by ID" << itemId;
				continue;
			}

			const auto& commentRss = item->CommentsLink_;
			if (commentRss.isEmpty ())
				continue;

			AddFeed ({
					.URL_ = commentRss,
					.Tags_ = item->Categories_ + commentTags,
					.UpdatesManager_ = Deps_.UpdatesManager_
				});
		}
	}

	namespace
	{
		QList<QUrl> GetSelectedLinks (const QModelIndexList& selection)
		{
			QList<QUrl> result;
			for (const auto& idx : selection)
				result << QUrl { idx.data (IItemsModel::ItemRole::ItemShortDescr).value<ItemShort> ().URL_ };
			return result;
		}
	}

	void ItemActions::LinkOpen ()
	{
		const auto iem = GetProxyHolder ()->GetEntityManager ();
		for (const auto& link : GetSelectedLinks (Deps_.GetSelection_ ()))
			iem->HandleEntity (Util::MakeEntity (link,
						{},
						FromUserInitiated | OnlyHandle));
	}

	void ItemActions::LinkCopy ()
	{
		const auto& links = GetSelectedLinks (Deps_.GetSelection_ ());

		const auto mime = new QMimeData;
		mime->setUrls (links);
		QGuiApplication::clipboard ()->setMimeData (mime);
	}

	QVector<IDType_t> ItemActions::GetSelectedIds () const
	{
		QVector<IDType_t> result;
		for (const auto& idx : Deps_.GetSelection_ ())
			result << idx.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
		return result;
	}
}
