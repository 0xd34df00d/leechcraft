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

#ifndef PLUGINS_LACKMAN_REPOINFOFETCHER_H
#define PLUGINS_LACKMAN_REPOINFOFETCHER_H
#include <QObject>
#include <QUrl>
#include <QProcess>
#include <interfaces/idownload.h>
#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class RepoInfoFetcher : public QObject
			{
				Q_OBJECT

				struct PendingRI
				{
					QUrl URL_;
					QString Location_;
				};
				QMap<int, PendingRI> PendingRIs_;

				struct PendingComponent
				{
					QUrl URL_;
					QString Location_;
					QString Component_;
					int RepoID_;
				};
				QMap<int, PendingComponent> PendingComponents_;
			public:
				RepoInfoFetcher (QObject*);

				void FetchFor (QUrl);
				void FetchComponent (QUrl, int, const QString& component);
			private slots:
				void handleRIFinished (int);
				void handleRIRemoved (int);
				void handleRIError (int, IDownload::Error);
				void handleComponentFinished (int);
				void handleComponentRemoved (int);
				void handleComponentError (int, IDownload::Error);

				void handleRepoUnarchFinished (int, QProcess::ExitStatus);
				void handleComponentUnarchFinished (int, QProcess::ExitStatus);
				void handleUnarchError (QProcess::ProcessError);
			signals:
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
				void gotEntity (const LeechCraft::Entity&);

				void infoFetched (const RepoInfo&);
				void componentFetched (const PackageShortInfoList& packages,
						const QString& component, int repoId);
			};
		}
	}
}

#endif
