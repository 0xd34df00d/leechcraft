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
#include <interfaces/core/icoreproxy.h>
#include "dbutils.h"
#include "feedsexportdialog.h"
#include "itemsexportdialog.h"
#include "writeopml.h"

namespace LC::Aggregator::ExportUtils
{
	void RunExportFB2 (ChannelsModel& channelsModel, QWidget *parent)
	{
		// TODO the dialog shouldn't depend on the exact ChannelsModel
		const auto dialog = new ItemsExportDialog (channelsModel, parent);
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		dialog->show ();
	}

	namespace
	{
		auto FilterChannels (channels_shorts_t channels, const QSet<IDType_t>& selected)
		{
			const auto removedPos = std::remove_if (channels.begin (), channels.end (),
					[&selected] (const ChannelShort& ch) { return !selected.contains (ch.ChannelID_); });
			channels.erase (removedPos, channels.end ());
			return channels;
		}
	}

	void RunExportOPML (QWidget *parent)
	{
		const auto& allChannels = GetAllChannels ();
		FeedsExportDialog exportDialog { parent };
		exportDialog.SetFeeds (allChannels);
		if (exportDialog.exec () == QDialog::Rejected)
			return;

		auto channels = FilterChannels (allChannels, exportDialog.GetSelectedFeeds ());

		auto data = OPML::Write (channels,
				exportDialog.GetTitle (), exportDialog.GetOwner (), exportDialog.GetOwnerEmail ());

		QFile f { exportDialog.GetDestination () };
		if (!f.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (parent,
					MessageBoxTitle,
					QObject::tr ("OPML export error: could not open file %1 for write.")
							.arg (f.fileName ()));
			return;
		}
		f.write (data);
	}
}
