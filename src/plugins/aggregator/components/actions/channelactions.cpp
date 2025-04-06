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
#include "components/models/channelsmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "actiondefshelpers.h"
#include "dbupdatethread.h"
#include "feedsettings.h"
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

	void ChannelActions::Update (const QList<ChannelShort>& channels)
	{
		for (const auto& channel : channels)
			Deps_.UpdatesManager_.UpdateFeed (channel.FeedID_);
	}

	void ChannelActions::Rename (const ChannelShort& channel)
	{
		const auto& newName = QInputDialog::getText (nullptr,
				tr ("Rename feed"),
				tr ("New feed name:"),
				QLineEdit::Normal,
				channel.Title_);
		if (newName.isEmpty () || newName == channel.Title_)
			return;

		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->SetChannelDisplayTitle (channel.ChannelID_, newName);
	}

	namespace
	{
		QString GetUserString (const QList<ChannelShort>& channels,
				const QString& single,
				const QString& multiple)
		{
			return channels.size () > 1 ?
					multiple :
					single.arg (Util::FormatName (channels.value (0).Title_));
		}

		bool Confirm (const QList<ChannelShort>& channels,
				const QString& single,
				const QString& multiple)
		{
			return QMessageBox::Yes == QMessageBox::question (nullptr,
					MessageBoxTitle,
					GetUserString (channels, single, multiple),
					QMessageBox::Yes | QMessageBox::No);
		}

		auto InvokeHandler (ChannelActions *pThis, const ChannelActions::Deps& deps,
				std::invocable<ChannelActions*, ChannelShort> auto handler)
		{
			return [=]
			{
				if (const auto& channel = deps.GetCurrentChannel_ ())
					std::invoke (handler, pThis, *channel);
			};
		}

		auto InvokeHandler (ChannelActions *pThis, const ChannelActions::Deps& deps,
				std::invocable<ChannelActions*, QList<ChannelShort>> auto handler)
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

	void ChannelActions::MarkAsRead (const QList<ChannelShort>& channels)
	{
		const auto& userString = GetUserString (channels,
				tr ("Are you sure you want to mark channel %1 as read?"),
				tr ("Are you sure you want to mark %n channel(s) as read?", nullptr, channels.size ()));
		if (!ConfirmWithPersistence ("ConfirmMarkChannelAsRead", userString))
			return;

		for (const auto& channel : channels)
			Deps_.DBUpThread_.ToggleChannelUnread (channel.ChannelID_, false);
	}

	void ChannelActions::MarkAsUnread (const QList<ChannelShort>& channels)
	{
		if (!Confirm (channels,
				tr ("Are you sure you want to mark channel %1 as unread?"),
				tr ("Are you sure you want to mark %n channel(s) as unread?", nullptr, channels.size ())))
			return;

		for (const auto& channel : channels)
			Deps_.DBUpThread_.ToggleChannelUnread (channel.ChannelID_, true);
	}

	void ChannelActions::RemoveFeed (const QList<ChannelShort>& channels)
	{
		if (!Confirm (channels,
				tr ("Are you sure you want to delete feed %1?"),
				tr ("Are you sure you want to delete %n feed(s)?", nullptr, channels.size ())))
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& channel : channels)
			sb->RemoveFeed (channel.FeedID_);
	}

	void ChannelActions::RemoveChannel (const QList<ChannelShort>& channels)
	{
		if (!Confirm (channels,
				tr ("Are you sure you want to delete channel %1?"),
				tr ("Are you sure you want to delete %n channel(s)?", nullptr, channels.size ())))
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& channel : channels)
			sb->RemoveChannel (channel.ChannelID_);
	}

	void ChannelActions::Settings (const ChannelShort& channel)
	{
		FeedSettings dia { channel };
		dia.exec ();
	}
}
