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
#include "xmlparsers.h"

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
				if (!path.endsWith ("/Repo.xml.gz"))
				{
					path.append ("/Repo.xml.gz");
					url.setPath (path);
				}

				QString location = Util::GetTemporaryName ("lackman_XXXXXX.gz");

				QUrl goodUrl = url;
				goodUrl.setPath (goodUrl.path ().remove ("Repo.xml.gz"));

				PendingRI pri =
				{
					goodUrl,
					location
				};

				Entity e = Util::MakeEntity (url,
						location,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
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
						SLOT (handleRIFinished (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobRemoved (int)),
						this,
						SLOT (handleRIRemoved (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleRIError (int, IDownload::Error)),
						Qt::UniqueConnection);
			}

			void RepoInfoFetcher::FetchComponent (QUrl url, int repoId, const QString& component)
			{
				if (!url.path ().endsWith ("/Packages.xml.gz"))
					url.setPath (url.path () + "/Packages.xml.gz");

				QString location = Util::GetTemporaryName ("lackman_XXXXXX.gz");

				PendingComponent pc =
				{
					url,
					location,
					component,
					repoId
				};

				Entity e = Util::MakeEntity (url,
						location,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit gotEntity (Util::MakeNotification (tr ("Error fetching component"),
							tr ("Could not find plugin to fetch component information at %1.")
								.arg (url.toString ()),
							PCritical_));
					return;
				}

				PendingComponents_ [id] = pc;

				connect (pr,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handleComponentFinished (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobRemoved (int)),
						this,
						SLOT (handleComponentRemoved (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleComponentError (int, IDownload::Error)),
						Qt::UniqueConnection);
			}

			void RepoInfoFetcher::FetchPackageInfo (const QUrl& packageUrl,
					const QString& packageName,
					const QList<QString>& newVersions,
					int componentId)
			{
				QString location = Util::GetTemporaryName ("lackman_XXXXXX.gz");

				PendingPackage pp =
				{
					packageUrl,
					location,
					packageName,
					newVersions,
					componentId
				};

				Entity e = Util::MakeEntity (packageUrl,
						location,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit gotEntity (Util::MakeNotification (tr ("Error fetching package information"),
							tr ("Could not find plugin to fetch package information at %1.")
								.arg (packageUrl.toString ()),
							PCritical_));
					return;
				}

				PendingPackages_ [id] = pp;

				connect (pr,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handlePackageFinished (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobRemoved (int)),
						this,
						SLOT (handlePackageRemoved (int)),
						Qt::UniqueConnection);
				connect (pr,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handlePackageError (int, IDownload::Error)),
						Qt::UniqueConnection);
			}

			void RepoInfoFetcher::handleRIFinished (int id)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRI pri = PendingRIs_.take (id);

				QString name = pri.Location_;
				QProcess *unarch = new QProcess (this);
				unarch->setProperty ("URL", pri.URL_);
				unarch->setProperty ("Filename", name);
				connect (unarch,
						SIGNAL (finished (int, QProcess::ExitStatus)),
						this,
						SLOT (handleRepoUnarchFinished (int, QProcess::ExitStatus)));
				connect (unarch,
						SIGNAL (error (QProcess::ProcessError)),
						this,
						SLOT (handleUnarchError (QProcess::ProcessError)));
				unarch->start ("gunzip", QStringList ("-c") << name);
			}

			void RepoInfoFetcher::handleRIRemoved (int id)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRIs_.remove (id);
			}

			void RepoInfoFetcher::handleRIError (int id, IDownload::Error error)
			{
				if (!PendingRIs_.contains (id))
					return;

				PendingRI pri = PendingRIs_.take (id);

				QFile::remove (pri.Location_);

				emit gotEntity (Util::MakeNotification (tr ("Error fetching repository"),
						tr ("Error downloading file from %1.")
							.arg (pri.URL_.toString ()),
						PCritical_));
			}

			void RepoInfoFetcher::handleComponentFinished (int id)
			{
				if (!PendingComponents_.contains (id))
					return;

				PendingComponent pc = PendingComponents_.take (id);

				QProcess *unarch = new QProcess (this);
				unarch->setProperty ("Component", pc.Component_);
				unarch->setProperty ("Filename", pc.Location_);
				unarch->setProperty ("URL", pc.URL_);
				unarch->setProperty ("RepoID", pc.RepoID_);
				connect (unarch,
						SIGNAL (finished (int, QProcess::ExitStatus)),
						this,
						SLOT (handleComponentUnarchFinished (int, QProcess::ExitStatus)));
				connect (unarch,
						SIGNAL (error (QProcess::ProcessError)),
						this,
						SLOT (handleUnarchError (QProcess::ProcessError)));
				unarch->start ("gunzip", QStringList ("-c") << pc.Location_);
			}

			void RepoInfoFetcher::handleComponentRemoved (int id)
			{
				if (!PendingComponents_.contains (id))
					return;

				PendingComponents_.remove (id);
			}

			void RepoInfoFetcher::handleComponentError (int id, IDownload::Error error)
			{
				if (!PendingComponents_.contains (id))
					return;

				PendingComponent pc = PendingComponents_.take (id);

				QFile::remove (pc.Location_);

				emit gotEntity (Util::MakeNotification (tr ("Error fetching component"),
						tr ("Error downloading file from %1.")
							.arg (pc.URL_.toString ()),
						PCritical_));
			}

			void RepoInfoFetcher::handlePackageFinished (int id)
			{
				if (!PendingPackages_.contains (id))
					return;

				PendingPackage pp = PendingPackages_ [id];

				QProcess *unarch = new QProcess (this);
				unarch->setProperty ("Filename", pp.Location_);
				unarch->setProperty ("URL", pp.URL_);
				unarch->setProperty ("TaskID", id);
				connect (unarch,
						SIGNAL (finished (int, QProcess::ExitStatus)),
						this,
						SLOT (handlePackageUnarchFinished (int, QProcess::ExitStatus)));
				connect (unarch,
						SIGNAL (error (QProcess::ProcessError)),
						this,
						SLOT (handleUnarchError (QProcess::ProcessError)));
				unarch->start ("gunzip", QStringList ("-c") << pp.Location_);
			}

			void RepoInfoFetcher::handlePackageRemoved (int id)
			{
				if (!PendingPackages_.contains (id))
					return;

				PendingPackages_.remove (id);
			}

			void RepoInfoFetcher::handlePackageError (int id, IDownload::Error error)
			{
				if (!PendingPackages_.contains (id))
					return;

				PendingPackage pp = PendingPackages_.take (id);

				QFile::remove (pp.Location_);

				emit gotEntity (Util::MakeNotification (tr ("Error fetching package"),
						tr ("Error fetching package from %1.")
							.arg (pp.URL_.toString ()),
						PCritical_));
			}

			void RepoInfoFetcher::handleRepoUnarchFinished (int exitCode,
					QProcess::ExitStatus exitStatus)
			{
				sender ()->deleteLater ();

				if (exitCode)
				{
					emit gotEntity (Util::MakeNotification (tr ("Repository unpack error"),
							tr ("Unable to unpack the repository file. gunzip error: %1. "
								"Problematic file is at %2.")
								.arg (exitCode)
								.arg (sender ()->property ("Filename").toString ()),
							PCritical_));
					return;
				}

				QByteArray data = qobject_cast<QProcess*> (sender ())->readAllStandardOutput ();
				QFile::remove (sender ()->property ("Filename").toString ());

				RepoInfo info;
				try
				{
					info = ParseRepoInfo (sender ()->property ("URL").toUrl (), QString (data));
				}
				catch (const QString& error)
				{
					qWarning () << Q_FUNC_INFO
							<< error;
					emit gotEntity (Util::MakeNotification (tr ("Repository parse error"),
							tr ("Unable to parse repository description: %1.")
								.arg (error),
							PCritical_));
					return;
				}

				emit infoFetched (info);
			}

			void RepoInfoFetcher::handleComponentUnarchFinished (int exitCode,
					QProcess::ExitStatus exitStatus)
			{
				sender ()->deleteLater ();

				if (exitCode)
				{
					emit gotEntity (Util::MakeNotification (tr ("Component unpack error"),
							tr ("Unable to unpack the component file. gunzip error: %1. "
								"Problematic file is at %2.")
								.arg (exitCode)
								.arg (sender ()->property ("Filename").toString ()),
							PCritical_));
					return;
				}

				QByteArray data = qobject_cast<QProcess*> (sender ())->readAllStandardOutput ();
				QFile::remove (sender ()->property ("Filename").toString ());

				PackageShortInfoList infos;
				try
				{
					infos = ParseComponent (data);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Component parse error"),
							tr ("Unable to parse component %1 description file. "
								"More information is available in logs.")
								.arg (sender ()->property ("Component").toString ()),
							PCritical_));
					return;
				}

				emit componentFetched (infos,
						sender ()->property ("Component").toString (),
						sender ()->property ("RepoID").toInt ());
			}

			void RepoInfoFetcher::handlePackageUnarchFinished (int exitCode,
					QProcess::ExitStatus status)
			{
				sender ()->deleteLater ();

				int id = sender ()->property ("TaskID").toInt ();
				PendingPackage pp = PendingPackages_.take (id);

				if (exitCode)
				{
					emit gotEntity (Util::MakeNotification (tr ("Component unpack error"),
							tr ("Unable to unpack the component file. gunzip error: %1. "
								"Problematic file is at %2.")
								.arg (exitCode)
								.arg (sender ()->property ("Filename").toString ()),
							PCritical_));
					return;
				}

				QByteArray data = qobject_cast<QProcess*> (sender ())->readAllStandardOutput ();
				QFile::remove (sender ()->property ("Filename").toString ());

				PackageInfo packageInfo;
				try
				{
					packageInfo = ParsePackage (data, pp.PackageName_, pp.NewVersions_);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Package parse error"),
							tr ("Unable to parse package description file. "
								"More information is available in logs."),
							PCritical_));
					return;
				}

				emit packageFetched (packageInfo, pp.ComponentId_);
			}

			void RepoInfoFetcher::handleUnarchError (QProcess::ProcessError error)
			{
				sender ()->deleteLater ();

				qWarning () << Q_FUNC_INFO
						<< "unable to unpack for"
						<< sender ()->property ("URL").toUrl ()
						<< sender ()->property ("Filename").toString ()
						<< "with"
						<< error
						<< qobject_cast<QProcess*> (sender ())->readAllStandardError ();
			}
		}
	}
}
