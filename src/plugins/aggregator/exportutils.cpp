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
#include "export.h"
#include "opmlwriter.h"
#include "storagebackendmanager.h"
#include "storagebackend.h"
#include "channelutils.h"

namespace LC::Aggregator::ExportUtils
{
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
		const auto& allChannels = ChannelUtils::GetAllChannels ();
		Export exportDialog (QObject::tr ("Export to OPML"),
				QObject::tr ("Select save file"),
				QObject::tr ("OPML files (*.opml);;"
					"XML files (*.xml);;"
					"All files (*.*)"),
				parent);
		exportDialog.SetFeeds (allChannels);
		if (exportDialog.exec () == QDialog::Rejected)
			return;

		auto channels = FilterChannels (allChannels, exportDialog.GetSelectedFeeds ());

		OPMLWriter writer { GetProxyHolder ()->GetTagsManager () };
		auto data = writer.Write (channels,
				exportDialog.GetTitle (), exportDialog.GetOwner (), exportDialog.GetOwnerEmail ());

		QFile f { exportDialog.GetDestination () };
		if (!f.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (parent,
					QObject::tr ("OPML export error"),
					QObject::tr ("Could not open file %1 for write.")
							.arg (f.fileName ()));
			return;
		}
		f.write (data.toUtf8 ());
	}
}
