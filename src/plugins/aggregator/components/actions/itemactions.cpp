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
	ItemActions::ItemActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, Deps_ { deps }
	{
		auto& xsm = *XmlSettingsManager::Instance ();

		struct ActionInfo
		{
			QKeySequence Shortcut_ {};
			std::optional<bool> Checked_ {};

			bool AddToAllActions_ = true;
		};

		const auto mkAction = [&] (const QString& name,
				const QByteArray& icon,
				const QString& objectNameSuffix,
				auto handler,
				ActionInfo info = {})
		{
			const auto action = new QAction { name, parent };
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

			deps.ShortcutsMgr_.RegisterAction ("Action" + objectNameSuffix + "_", action);

			if (info.AddToAllActions_)
				AllActions_ << action;

			return action;
		};

		const auto mkSep = [&] { AllActions_ << Util::CreateSeparator (parent); };

		ToolbarActions_ << mkAction (tr ("Hide read items"), "mail-mark-unread", "HideReadItems",
				deps.SetHideRead_,
				{ .Checked_ = xsm.Property ("HideReadItems", false).toBool () });
		ToolbarActions_ << mkAction (tr ("Show items as tape"), "format-list-unordered", "ActionShowAsTape",
				deps.SetShowTape_,
				{ .Checked_ = xsm.Property ("ShowAsTape", false).toBool () });

		mkSep ();

		MarkUnread_ = mkAction (tr ("Mark as unread"), {}, "MarkItemAsUnread",
				[this] { MarkSelectedReadStatus (false); },
				{ .Shortcut_ = { "U" } });
		MarkRead_ = mkAction (tr ("Mark as read"), {}, "MarkItemAsRead",
				[this] { MarkSelectedReadStatus (true); },
				{ .Shortcut_ = { "R" } });
		mkSep ();
		MarkUnimportant_ = mkAction (tr ("Unimportant"), "rating-unrated", "MarkItemAsUnimportant",
				[this] { MarkSelectedAsImportant (false); },
				{ .Shortcut_ = { "I" } });
		MarkImportant_ = mkAction (tr ("Important"), "rating", "MarkItemAsImportant",
				[this] { MarkSelectedAsImportant (true); },
				{ .Shortcut_ = { "Shift+I" } });
		mkSep ();
		Delete_ = mkAction (tr ("Delete"), "remove", "DeleteItem",
				[this] { DeleteSelected (); },
				{ .Shortcut_ = { "Delete" } });
		mkSep ();
		SubComments_ = mkAction (tr ("Subscribe to comments"), "news-subscribe", "ItemCommentsSubscribe",
				[this] { SubscribeComments (); });
		LinkOpen_ = mkAction (tr ("Open in new tab"), "internet-web-browser", "ItemLinkOpen",
				[this] { LinkOpen (); },
				{ .Shortcut_ = { "O" } });
		LinkCopy_ = mkAction (tr ("Copy news item link"), "edit-copy", "ItemLinkCopy",
				[this] { LinkCopy (); },
				{ .Shortcut_ = { "C" } });

		InvisibleActions_ << mkAction (tr ("Previous unread item"), "go-first", "PrevUnreadItem",
				[deps] { deps.ItemNavigator_.MoveToPrevUnread (); },
				{ .Shortcut_ = { "Shift+K" }, .AddToAllActions_ = false });
		InvisibleActions_ << mkAction (tr ("Next unread item"), "go-last", "NextUnreadItem",
				[deps] { deps.ItemNavigator_.MoveToNextUnread (); },
				{ .Shortcut_ = { "Shift+J" }, .AddToAllActions_ = false });
		InvisibleActions_ << mkAction (tr ("Previous item"), "go-previous", "PrevItem",
				[deps] { deps.ItemNavigator_.MoveToPrev (); },
				{ .Shortcut_ = { "K" }, .AddToAllActions_ = false });
		InvisibleActions_ << mkAction (tr ("Next item"), "go-next", "NextItem",
				[deps] { deps.ItemNavigator_.MoveToNext (); },
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
