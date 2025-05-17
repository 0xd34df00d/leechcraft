/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exportutils.h"
#include <algorithm>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QMessageBox>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "components/storage/storagebackend.h"
#include "components/storage/storagebackendmanager.h"
#include "dbutils.h"
#include "feedsexportdialog.h"
#include "itemsexportdialog.h"
#include "writefb2.h"
#include "writeopml.h"
#include "writepdf.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		void ExportItems (const ItemsExportDialog& dialog, QWidget *parent)
		{
			QFile file { dialog.GetFilename () };
			if (!file.open (QIODevice::WriteOnly))
			{
				QMessageBox::critical (parent,
						MessageBoxTitle,
						QObject::tr ("Cannot open %1 for writing: %2.")
							.arg (dialog.GetFilename (), file.errorString ()));
				return;
			}

			const auto& itemsExportInfo = dialog.GetItemsExportInfo ();

			QMap<ChannelShort, QList<Item>> items2write;

			qDebug () << 1;
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			for (const auto& channelId : itemsExportInfo.Channels_)
			{
				const auto& cs = sb->GetChannel (channelId).ToShort ();
				auto& items = items2write [cs];
				for (const auto& item : sb->GetItems (channelId))
				{
					if (itemsExportInfo.UnreadOnly_ && !item.Unread_)
						continue;

					if (!item.Categories_.isEmpty ())
					{
						const auto matchesCategories = std::ranges::any_of (itemsExportInfo.Categories_,
								[&item] (const QString& cat) { return item.Categories_.contains (cat); });
						if (!matchesCategories)
							continue;
					}

					if (const auto& fullItem = sb->GetItem (item.ItemID_))
						items.prepend (*fullItem);
				}
			}
			qDebug () << 2;

			Util::Visit (dialog.GetFormat (),
				[&] (const Fb2Config& config) { WriteFB2 (config, items2write, file); },
				[&] (const PdfConfig& config) { WritePDF (config, items2write); });
			qDebug () << 3;

			const auto iem = GetProxyHolder ()->GetEntityManager ();
			iem->HandleEntity (Util::MakeNotification (NotificationTitle,
						QObject::tr ("Finished exporting items."),
						Priority::Info));
		}
	}

	void RunExportItems (ChannelsModel& channelsModel, QWidget *parent)
	{
		// TODO the dialog shouldn't depend on the exact ChannelsModel
		const auto dialog = new ItemsExportDialog { channelsModel, parent };
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		dialog->show ();
		QObject::connect (dialog,
				&QDialog::accepted,
				[=] { ExportItems (*dialog, parent); });
	}

	namespace
	{
		auto FilterChannels (channels_shorts_t channels, const QSet<IDType_t>& selected)
		{
			const auto removedPos = std::ranges::remove_if (channels,
					[&selected] (const ChannelShort& ch) { return !selected.contains (ch.ChannelID_); }).begin ();
			channels.erase (removedPos, channels.end ());
			return channels;
		}
	}

	void RunExportChannels (QAbstractItemModel& channelsModel, QWidget *parent)
	{
		const auto& allChannels = GetAllChannels ();
		FeedsExportDialog exportDialog { channelsModel, parent };
		if (exportDialog.exec () == QDialog::Rejected)
			return;

		const auto channels = FilterChannels (allChannels, exportDialog.GetSelectedFeeds ());
		const auto data = OPML::Write (channels,
				exportDialog.GetTitle (), exportDialog.GetOwner (), exportDialog.GetOwnerEmail ());

		QFile file { exportDialog.GetDestination () };
		if (!file.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (parent,
					MessageBoxTitle,
					QObject::tr ("Cannot open %1 for writing: %2.")
							.arg (file.fileName (), file.errorString ()));
			return;
		}
		file.write (data);
	}

	void ManageLastPath (const LastPathParams& params)
	{
		auto& edit = params.Edit_;
		auto name = params.SettingName_;
		edit.setText (XmlSettingsManager::Instance ().Property (name, params.DefaultPath_).toString ());

		QObject::connect (&params.Parent_,
				&QDialog::accepted,
				[name, &edit] { XmlSettingsManager::Instance ().setProperty (name, edit.text ()); });
	}
}
