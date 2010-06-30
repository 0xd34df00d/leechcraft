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
#include <QXmlQuery>
#include <QDomDocument>
#include <QDomElement>
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

				QString location = Util::GetTemporaryName ("lackman_XXXXXX.xz");

				QUrl goodUrl = url;
				goodUrl.setPath (goodUrl.path ().remove ("Repo.xml.xz"));

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
				if (!url.path ().endsWith ("/Packages.xml.xz"))
					url.setPath (url.path () + "/Packages.xml.xz");

				QString location = Util::GetTemporaryName ("lackman_XXXXXX.xz");

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
				unarch->start ("unxz", QStringList ("-c") << name);
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
				unarch->start ("unxz", QStringList ("-c") << pc.Location_);
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

			void RepoInfoFetcher::handleRepoUnarchFinished (int exitCode,
					QProcess::ExitStatus exitStatus)
			{
				sender ()->deleteLater ();

				if (exitCode)
				{
					emit gotEntity (Util::MakeNotification (tr ("Repository unpack error"),
							tr ("Unable to unpack the repository file. unxz error: %1."
								"Problematic file is at %2.")
								.arg (exitCode)
								.arg (sender ()->property ("Filename").toString ()),
							PCritical_));
					return;
				}

				QByteArray data = qobject_cast<QProcess*> (sender ())->readAllStandardOutput ();
				QFile::remove (sender ()->property ("Filename").toString ());

				QXmlQuery query;
				query.setFocus (QString (data));

				RepoInfo info (sender ()->property ("URL").toUrl ());

				try
				{
					QString out;
					query.setQuery ("/repo/name/text()");
					if (!query.evaluateTo (&out))
						throw tr ("Could not get repo name.");
					info.SetName (out.simplified ());

					query.setQuery ("/repo/description/short/text()");
					if (!query.evaluateTo (&out))
						throw tr ("Could not get repo description.");
					info.SetShortDescr (out.simplified ());

					query.setQuery ("/repo/description/long/text()");
					if (!query.evaluateTo (&out))
						throw tr ("Could not get long repo description.");
					info.SetLongDescr (out.simplified ());

					MaintainerInfo maintInfo;
					query.setQuery ("/repo/maintainer/name/text()");
					if (!query.evaluateTo (&out))
						throw tr ("Could not get maintainer name.");
					maintInfo.Name_ = out.simplified ();

					query.setQuery ("/repo/maintainer/email/text()");
					if (!query.evaluateTo (&out))
						throw tr ("Could not get maintainer email.");
					maintInfo.Email_ = out.simplified ();

					info.SetMaintainer (maintInfo);

					QStringList components;
					query.setQuery ("/repo/components/component/text()");
					if (query.evaluateTo (&components))
						info.SetComponents (components);
					else if (query.evaluateTo (&out))
						info.SetComponents (QStringList (out));
					else
						throw tr ("Could not get components.");
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
							tr ("Unable to unpack the component file. unxz error: %1."
								"Problematic file is at %2.")
								.arg (exitCode)
								.arg (sender ()->property ("Filename").toString ()),
							PCritical_));
					return;
				}

				QByteArray data = qobject_cast<QProcess*> (sender ())->readAllStandardOutput ();
				QFile::remove (sender ()->property ("Filename").toString ());

				QDomDocument doc;
				QString msg;
				int line = 0;
				int column = 0;
				if (!doc.setContent (data, &msg, &line, &column))
				{
					qWarning () << Q_FUNC_INFO
							<< "erroneous document with msg"
							<< msg
							<< line
							<< column
							<< data;
					emit gotEntity (Util::MakeNotification (tr ("Component parse error"),
							tr ("Unable to parse component %1 description file. "
								"More information is available in logs.")
								.arg (sender ()->property ("Component").toString ()),
							PCritical_));
					return;
				}

				PackageShortInfoList infos;

				QDomElement root = doc.documentElement ();
				QDomElement package = root.firstChildElement ("package");
				while (!package.isNull ())
				{
					QStringList versionsList;
					QDomElement versions = package.firstChildElement ("versions");
					QDomElement version = versions.firstChildElement ("version");
					while (!version.isNull ())
					{
						versionsList << version.text ();

						version = version.nextSiblingElement ("version");
					}

					PackageShortInfo psi =
					{
						package.firstChildElement ("name").text (),
						versionsList
					};
					infos << psi;

					package = package.nextSiblingElement ("package");
				}

				emit componentFetched (infos);
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
