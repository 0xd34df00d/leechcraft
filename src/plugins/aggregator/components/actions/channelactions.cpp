/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelactions.h"
#include <concepts>
#include <QMessageBox>
#include <QModelIndex>
#include <QInputDialog>
#include <interfaces/core/iiconthememanager.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/gui/util.h>
#include <util/util.h>
#include "actiondefshelpers.h"
#include "channelsmodel.h"
#include "dbupdatethread.h"
#include "feedsettings.h"
#include "resourcesfetcher.h"
#include "storagebackendmanager.h"
#include "updatesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	MAKE_ACTIONS (ChannelActions,
		MarkChannelAsRead,
		MarkChannelAsUnread,
		RemoveFeed,
		UpdateSelectedFeed,
		RenameFeed,
		RemoveChannel,
		ChannelSettings
	);

	namespace
	{
		ActionInfo MakeInfo (const QString& text, const QByteArray& icon)
		{
			return ActionInfo { .Text_ = text, .Icon_ = icon };
		}

		ActionInfo GetActionInfo (ChannelActions::ActionId action)
		{
			using enum ChannelActions::ActionId;
			switch (action)
			{
			case MarkChannelAsRead:
				return MakeInfo (ChannelActions::tr ("Mark channel as read"), "mail-mark-read");
			case MarkChannelAsUnread:
				return MakeInfo (ChannelActions::tr ("Mark channel as unread"), "mail-mark-unread");
			case RemoveFeed:
				return MakeInfo (ChannelActions::tr ("Remove feed"), "list-remove");
			case UpdateSelectedFeed:
				return MakeInfo (ChannelActions::tr ("Update selected feed"), "view-refresh");
			case RenameFeed:
				return MakeInfo (ChannelActions::tr ("Rename feed"), "edit-rename");
			case RemoveChannel:
				return MakeInfo (ChannelActions::tr ("Remove channel"), {});
			case ChannelSettings:
				return MakeInfo (ChannelActions::tr ("Settings..."), "configure");
			}

			qWarning () << "unknown action" << static_cast<int> (action);
			return {};
		}
	}

	void ChannelActions::RegisterActions (Util::ShortcutManager& sm)
	{
		for (const auto actionId : AllActionIds ())
			sm.RegisterActionInfo (ToString (actionId), GetActionInfo (actionId));
	}

	ChannelActions::ChannelActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, Deps_ { deps }
	{
		const auto mkSep = [this] { AllActions_ << Util::CreateSeparator (this); };

		using enum ActionId;
		MakeAction (MarkChannelAsRead, &ChannelActions::MarkAsRead);
		MakeAction (MarkChannelAsUnread, &ChannelActions::MarkAsUnread);
		mkSep ();
		ToolbarActions_ << MakeAction (RemoveFeed, &ChannelActions::RemoveFeed);
		ToolbarActions_ << MakeAction (UpdateSelectedFeed, &ChannelActions::Update);
		MakeAction (RenameFeed, &ChannelActions::Rename);
		mkSep ();
		MakeAction (RemoveChannel, &ChannelActions::RemoveChannel);
		mkSep ();
		MakeAction (ChannelSettings, &ChannelActions::Settings);

		GetProxyHolder ()->GetIconThemeManager ()->UpdateIconset (AllActions_);
	}

	QList<QAction*> ChannelActions::GetAllActions () const
	{
		return AllActions_;
	}

	QList<QAction*> ChannelActions::GetToolbarActions () const
	{
		return ToolbarActions_;
	}

	void ChannelActions::Update (const QModelIndexList& idxes)
	{
		for (const auto& idx : idxes)
			Deps_.UpdatesManager_.UpdateFeed (idx.data (ChannelRoles::FeedID).value<IDType_t> ());
	}

	void ChannelActions::Rename (const QModelIndex& idx)
	{
		const auto& current = idx.data (ChannelRoles::ChannelTitle).toString ();
		const auto& newName = QInputDialog::getText (nullptr,
				tr ("Rename feed"),
				tr ("New feed name:"),
				QLineEdit::Normal,
				current);
		if (newName.isEmpty ())
			return;

		auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetChannelDisplayTitle (idx.data (ChannelRoles::ChannelID).value<IDType_t> (), newName);
	}

	namespace
	{
		QString GetUserString (const QModelIndexList& idxes,
				const QString& single,
				const QString& multiple)
		{
			if (idxes.size () > 1)
				return multiple;

			const auto& name = idxes.value (0).data (ChannelRoles::ChannelTitle).toString ();
			return single.arg (Util::FormatName (name));
		}

		bool Confirm (const QModelIndexList& idxes,
				const QString& single,
				const QString& multiple)
		{
			return QMessageBox::Yes == QMessageBox::question (nullptr,
					MessageBoxTitle,
					GetUserString (idxes, single, multiple),
					QMessageBox::Yes | QMessageBox::No);
		}

		auto InvokeHandler (ChannelActions *pThis, const ChannelActions::Deps& deps,
				std::invocable<ChannelActions*, QModelIndex> auto handler)
		{
			return [=]
			{
				if (const auto& idx = deps.GetCurrentChannel_ ();
						idx.isValid ())
					std::invoke (handler, pThis, idx);
			};
		}

		auto InvokeHandler (ChannelActions *pThis, const ChannelActions::Deps& deps,
				std::invocable<ChannelActions*, QModelIndexList> auto handler)
		{
			return [=]
			{
				if (const auto& chans = deps.GetAllSelectedChannels_ ();
						!chans.isEmpty ())
					std::invoke (handler, pThis, chans);
			};
		}
	}

	QAction* ChannelActions::MakeAction (ActionId actionId, auto handler)
	{
		const auto action = new QAction { parent () };
		Deps_.ShortcutManager_.RegisterAction (ToString (actionId), action);

		connect (action,
				&QAction::triggered,
				this,
				InvokeHandler (this, Deps_, handler));

		AllActions_ << action;

		return action;
	}

	void ChannelActions::MarkAsRead (const QModelIndexList& idxes)
	{
		const auto& userString = GetUserString (idxes,
				tr ("Are you sure you want to mark channel %1 as read?"),
				tr ("Are you sure you want to mark %n channel(s) as read?", nullptr, idxes.size ()));
		if (!ConfirmWithPersistence ("ConfirmMarkChannelAsRead", userString))
			return;

		for (const auto& idx : idxes)
			Deps_.DBUpThread_.ToggleChannelUnread (idx.data (ChannelRoles::ChannelID).value<IDType_t> (), false);
	}

	void ChannelActions::MarkAsUnread (const QModelIndexList& idxes)
	{
		if (!Confirm (idxes,
				tr ("Are you sure you want to mark channel %1 as unread?"),
				tr ("Are you sure you want to mark %n channel(s) as unread?", nullptr, idxes.size ())))
			return;

		for (const auto& idx : idxes)
			Deps_.DBUpThread_.ToggleChannelUnread (idx.data (ChannelRoles::ChannelID).value<IDType_t> (), true);
	}

	void ChannelActions::RemoveFeed (const QModelIndexList& idxes)
	{
		if (!Confirm (idxes,
				tr ("Are you sure you want to delete feed %1?"),
				tr ("Are you sure you want to delete %n feed(s)?", nullptr, idxes.size ())))
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& idx : idxes)
			sb->RemoveFeed (idx.data (ChannelRoles::FeedID).value<IDType_t> ());
	}

	void ChannelActions::RemoveChannel (const QModelIndexList& idxes)
	{
		if (!Confirm (idxes,
				tr ("Are you sure you want to delete channel %1?"),
				tr ("Are you sure you want to delete %n channel(s)?", nullptr, idxes.size ())))
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& idx : idxes)
			sb->RemoveChannel (idx.data (ChannelRoles::ChannelID).value<IDType_t> ());
	}

	void ChannelActions::Settings (const QModelIndex& idx)
	{
		FeedSettings dia { idx.data (ChannelShortStruct).value<ChannelShort> () };
		connect (&dia,
				&FeedSettings::faviconRequested,
				&Deps_.ResourcesFetcher_,
				&ResourcesFetcher::FetchFavicon);
		dia.exec ();
	}
}
