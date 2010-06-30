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

#include "core.h"
#include <QtDebug>
#include <plugininterface/util.h>
#include "repoinfofetcher.h"
#include "storage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			Core::Core ()
			: RepoInfoFetcher_ (new RepoInfoFetcher (this))
			, Storage_ (new Storage (this))
			{
				connect (RepoInfoFetcher_,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (RepoInfoFetcher_,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (RepoInfoFetcher_,
						SIGNAL (infoFetched (const RepoInfo&)),
						this,
						SLOT (handleInfoFetched (const RepoInfo&)));
				connect (RepoInfoFetcher_,
						SIGNAL (componentFetched (const PackageShortInfoList&)),
						this,
						SLOT (handleComponentFetched (const PackageShortInfoList&)));
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::Release ()
			{
				delete RepoInfoFetcher_;
				RepoInfoFetcher_ = 0;

				delete Storage_;
				Storage_ = 0;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			void Core::AddRepo (const QUrl& url)
			{
				RepoInfoFetcher_->FetchFor (url);
			}

			void Core::UpdateRepo (const QUrl& url, const QStringList& components)
			{
				int id = -1;
				try
				{
					int id = Storage_->FindRepo (url);
					if (id == -1)
					{
						QString str;
						QDebug debug (&str);
						debug << "unable to find repo with URL"
								<< url.toString ();
						qWarning () << Q_FUNC_INFO
								<< str;
						emit gotEntity (Util::MakeNotification (tr ("Error updating repository"),
								tr ("Unable to find repository with URL %1.")
									.arg (url.toString ()),
								PCritical_));
						return;
					}
				}
				catch (const std::exception& e)
				{
					QString str;
					QDebug debug (&str);
					debug << "unable to get ID for repo"
							<< url.toString ()
							<< "with error"
							<< e.what ();
					qWarning () << Q_FUNC_INFO
							<< str;
					emit gotEntity (Util::MakeNotification (tr ("Error updating repository"),
							tr ("While trying to update the repository: %1.")
								.arg (str),
							PCritical_));
					return;
				}

				Q_FOREACH (const QString& component, components)
				{
					QUrl compUrl = url;
					compUrl.setPath ((compUrl.path () + "/dists/%1/all/").arg (component));
					RepoInfoFetcher_->FetchComponent (compUrl, id, component);
				}
			}

			void Core::handleInfoFetched (const RepoInfo& ri)
			{
				int repoId = -1;
				try
				{
					repoId = Storage_->FindRepo (ri.GetUrl ());
					if (repoId == -1)
						repoId = Storage_->AddRepo (ri);
				}
				catch (const std::exception& e)
				{
					QString str;
					QDebug debug (&str);
					debug << "unable to find/add repo"
							<< ri.GetName ()
							<< ri.GetUrl ()
							<< "with error"
							<< e.what ();
					qWarning () << Q_FUNC_INFO
							<< str;
					emit gotEntity (Util::MakeNotification (tr ("Error adding/updating repository"),
							tr ("While trying to add or update the repository: %1.")
								.arg (str),
							PCritical_));
				}

				if (repoId == -1)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to add repo"
							<< ri.GetUrl ()
							<< ri.GetName ();
					return;
				}

				UpdateRepo (ri.GetUrl (), ri.GetComponents ());
			}

			void Core::handleComponentFetched (const PackageShortInfoList& shortInfos)
			{
				qDebug () << Q_FUNC_INFO;
				Q_FOREACH (const PackageShortInfo& info, shortInfos)
					qDebug () << info.Name_ << info.Versions_;
			}
		}
	}
}
