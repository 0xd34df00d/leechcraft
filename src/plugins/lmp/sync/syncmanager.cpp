/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "syncmanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include <util/util.h>
#include "copymanager.h"
#include "../core.h"
#include "../localfileresolver.h"
#include "../util.h"

namespace LeechCraft
{
namespace LMP
{
	SyncManager::SyncManager (QObject *parent)
	: SyncManagerBase (parent)
	{
	}

	void SyncManager::AddFiles (ISyncPlugin *syncer, const QString& mount,
			const QStringList& files, const TranscodingParams& params)
	{
		std::for_each (files.begin (), files.end (),
				[this, syncer, &mount] (decltype (files.front ()) file)
					{ Source2Params_ [file] = { syncer, mount }; });

		SyncManagerBase::AddFiles (files, params);
	}

	void SyncManager::CreateSyncer (const QString& mount)
	{
		auto mgr = new CopyManager (this);
		connect (mgr,
				SIGNAL (startedCopying (QString)),
				this,
				SLOT (handleStartedCopying (QString)));
		connect (mgr,
				SIGNAL (finishedCopying ()),
				this,
				SLOT (handleFinishedCopying ()));
		Mount2Copiers_ [mount] = mgr;
	}

	namespace
	{
		bool FixMask (QString& mask, const QString& transcoded)
		{
			MediaInfo info;
			try
			{
				info = Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				return false;
			}

			mask = PerformSubstitutions (mask, info);
			const auto& ext = QFileInfo (transcoded).suffix ();
			if (!mask.endsWith (ext))
				mask += "." + ext;

			return true;
		}
	}

	void SyncManager::handleFileTranscoded (const QString& from,
			const QString& transcoded, QString mask)
	{
		SyncManagerBase::HandleFileTranscoded (from, transcoded);

		const auto& syncTo = Source2Params_.take (from);
		if (syncTo.MountPath_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "dumb transcoded file detected"
					<< from
					<< transcoded;
			return;
		}

		emit uploadLog (tr ("File %1 successfully transcoded, adding to copy queue for the device %2...")
				.arg ("<em>" + QFileInfo (from).fileName () + "</em>")
				.arg ("<em>" + syncTo.MountPath_) + "</em>");

		if (!FixMask (mask, transcoded))
			return;

		if (!Mount2Copiers_.contains (syncTo.MountPath_))
			CreateSyncer (syncTo.MountPath_);
		const CopyManager::CopyJob copyJob
		{
			syncTo.Syncer_,
			transcoded,
			from,
			from != transcoded,
			syncTo.MountPath_,
			mask
		};
		Mount2Copiers_ [syncTo.MountPath_]->Copy (copyJob);
	}
}
}
