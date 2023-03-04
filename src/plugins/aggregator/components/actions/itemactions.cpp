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
#include <util/xpc/util.h>
#include <util/util.h>
#include "interfaces/aggregator/iitemsmodel.h"
#include "../../dbutils.h"
#include "storagebackendmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	struct ItemActions::ActionInfo
	{
		QKeySequence Shortcut_ {};
		std::optional<bool> Checked_ {};

		bool AddToAllActions_ = true;
	};

	ItemActions::ItemActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, Deps_ { deps }
	{
		auto& xsm = *XmlSettingsManager::Instance ();

		const auto mkSep = [&] { AllActions_ << Util::CreateSeparator (parent); };

		ToolbarActions_ << MakeAction (tr ("Hide read items"), "mail-mark-unread", "HideReadItems",
				deps.SetHideRead_,
				{ .Checked_ = xsm.Property ("HideReadItems", false).toBool () });
		ToolbarActions_ << MakeAction (tr ("Show items as tape"), "format-list-unordered", "ActionShowAsTape",
				deps.SetShowTape_,
				{ .Checked_ = xsm.Property ("ShowAsTape", false).toBool () });

		mkSep ();

		MarkUnread_ = MakeAction (tr ("Mark as unread"), {}, "MarkItemAsUnread",
				[this] { MarkSelectedReadStatus (false); },
				{ .Shortcut_ = { "U" } });
		MarkRead_ = MakeAction (tr ("Mark as read"), {}, "MarkItemAsRead",
				[this] { MarkSelectedReadStatus (true); },
				{ .Shortcut_ = { "R" } });
		mkSep ();
		MarkUnimportant_ = MakeAction (tr ("Unimportant"), "rating-unrated", "MarkItemAsUnimportant",
				[this] { MarkSelectedAsImportant (false); },
				{ .Shortcut_ = { "I" } });
		MarkImportant_ = MakeAction (tr ("Important"), "rating", "MarkItemAsImportant",
				[this] { MarkSelectedAsImportant (true); },
				{ .Shortcut_ = { "Shift+I" } });
		mkSep ();
		Delete_ = MakeAction (tr ("Delete"), "remove", "DeleteItem",
				[this] { DeleteSelected (); },
				{ .Shortcut_ = { "Delete" } });
		mkSep ();
		SubComments_ = MakeAction (tr ("Subscribe to comments"), "news-subscribe", "ItemCommentsSubscribe",
				[this] { SubscribeComments (); },
				{});
		LinkOpen_ = MakeAction (tr ("Open in new tab"), "internet-web-browser", "ItemLinkOpen",
				[this] { LinkOpen (); },
				{ .Shortcut_ = { "O" } });
		LinkCopy_ = MakeAction (tr ("Copy news item link"), "edit-copy", "ItemLinkCopy",
				[this] { LinkCopy (); },
				{ .Shortcut_ = { "C" } });

		auto& nav = deps.ItemNavigator_;
		InvisibleActions_ << MakeAction (tr ("Previous unread item"), "go-first", "PrevUnreadItem",
				[&nav] { nav.MoveToPrevUnread (); },
				{ .Shortcut_ = { "Shift+K" }, .AddToAllActions_ = false });
		InvisibleActions_ << MakeAction (tr ("Next unread item"), "go-last", "NextUnreadItem",
				[&nav] { nav.MoveToNextUnread (); },
				{ .Shortcut_ = { "Shift+J" }, .AddToAllActions_ = false });
		InvisibleActions_ << MakeAction (tr ("Previous item"), "go-previous", "PrevItem",
				[&nav] { nav.MoveToPrev (); },
				{ .Shortcut_ = { "K" }, .AddToAllActions_ = false });
		InvisibleActions_ << MakeAction (tr ("Next item"), "go-next", "NextItem",
				[&nav] { nav.MoveToNext (); },
				{ .Shortcut_ = { "J" }, .AddToAllActions_ = false });
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

	QAction* ItemActions::MakeAction (const QString& name,
			const QByteArray& icon,
			const QByteArray& objectNameSuffix,
			auto handler,
			const ActionInfo& info)
	{
		const auto action = new QAction { name, parent () };
		if (!icon.isEmpty ())
			action->setProperty ("ActionIcon", icon);
		if (!info.Shortcut_.isEmpty ())
			action->setShortcut (info.Shortcut_);
		if (info.Checked_)
		{
			action->setCheckable (true);
			action->setChecked (*info.Checked_);
		}
		connect (action,
				&QAction::triggered,
				this,
				handler);

		Deps_.ShortcutsMgr_.RegisterAction ("Action" + objectNameSuffix + "_", action);

		if (info.AddToAllActions_)
			AllActions_ << action;

		return action;
	}

	void ItemActions::MarkSelectedReadStatus (bool read)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto itemId : GetSelectedIds ())
			sb->SetItemUnread (itemId, !read);
	}

	void ItemActions::MarkSelectedAsImportant (bool important)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const ITagsManager::tag_id impId = "_important";
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
					"LeechCraft",
					tr ("Are you sure you want to remove %n items?", 0, ids.size ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->RemoveItems ({ ids.begin (), ids.end () });
	}

	namespace
	{
		QStringList GetCommentTags ()
		{
			const auto& prop = XmlSettingsManager::Instance ()->property ("CommentsTags").toString ();
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
