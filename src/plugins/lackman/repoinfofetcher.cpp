/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "repoinfofetcher.h"
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			RepoInfoFetcher::RepoInfoFetcher (QObject *parent)
			: QObject (parent)
			{
			}

			void RepoInfoFetcher::FetchFor (QUrl url)
			{
				QString path = url.path ();
				if (!path.endsWith ("/Repo.xml.xz"))
				{
					path.append ("/Repo.xml.xz");
					url.setPath (path);
				}

				QString location = Util::GetTemporaryName ("lackman_XXXXXX");

				PendingRI pri =
				{
					url,
					location
				};

				Entity e = Util::MakeEntity (url,
						location,
						LeechCraft::Internal |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit gotEntity (Util::MakeNotification (tr ("Error fetching repository"),
							tr ("Could not find plugin to fetch repository information for %1.")
								.arg (url.toString ()),
							PCritical_));
					return;
				}

				PendingRIs_ [id] = pri;

				connect (pr,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handleJobFinished (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobRemoved (int)),
						this,
						SLOT (handleJobRemoved (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleJobError (int, IDownload::Error)),
						Qt::UniqueConnection);
			}

			void RepoInfoFetcher::handleJobFinished (int id)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRI pri = PendingRIs_.take (id);

				QString name = pri.Location_;
				qDebug () << Q_FUNC_INFO << name;
			}

			void RepoInfoFetcher::handleJobRemoved (int id)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRIs_.remove (id);
			}

			void RepoInfoFetcher::handleJobError (int id, IDownload::Error error)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRI pri = PendingRIs_.take (id);

				emit gotEntity (Util::MakeNotification (tr ("Error fetching repository"),
						tr ("Error downloading file from %1.")
							.arg (pri.URL_.toString ()),
						PCritical_));
			}
		}
	}
}
