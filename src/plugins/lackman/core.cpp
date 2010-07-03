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
#include <boost/bind.hpp>
#include <QString>
#include <QStandardItemModel>
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
			, PluginsModel_ (new QStandardItemModel (this))
			{
				QStringList labels;
				labels << tr ("Name")
						<< tr ("Type")
						<< tr ("Tags")
						<< tr ("Last version");
				PluginsModel_->setHorizontalHeaderLabels (labels);

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
						SIGNAL (componentFetched (const PackageShortInfoList&,
								const QString&, int)),
						this,
						SLOT (handleComponentFetched (const PackageShortInfoList&,
								const QString&, int)));
				connect (RepoInfoFetcher_,
						SIGNAL (packageFetched (const PackageInfo&, int)),
						this,
						SLOT (handlePackageFetched (const PackageInfo&, int)));
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

			QAbstractItemModel* Core::GetPluginsModel () const
			{
				return PluginsModel_;
			}

			void Core::AddRepo (const QUrl& url)
			{
				RepoInfoFetcher_->FetchFor (url);
			}

			void Core::UpdateRepo (const QUrl& url, const QStringList& components)
			{
				int id = -1;
				QStringList ourComponents;
				try
				{
					id = Storage_->FindRepo (url);
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
					ourComponents = Storage_->GetComponents (id);
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

				Q_FOREACH (const QString& oc, ourComponents)
				{
					if (!components.contains (oc))
					{
						qDebug () << Q_FUNC_INFO
								<< "orphaned component"
								<< oc;
						try
						{
							Storage_->RemoveComponent (id, oc);
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to remove component"
									<< oc
									<< "not present in freshly obtained description of"
									<< id
									<< url;
							emit gotEntity (Util::MakeNotification (tr ("Error updating repository"),
									tr ("Unable to remove the component `%1` which "
										"disappeared from the list of components for repo %2.")
										.arg (oc)
										.arg (url.toString ()),
									PCritical_));
							return;
						}
					}
				}

				Q_FOREACH (const QString& component, components)
				{
					QUrl compUrl = url;
					compUrl.setPath ((compUrl.path () + "/dists/%1/all/").arg (component));
					RepoInfoFetcher_->FetchComponent (compUrl, id, component);
				}
			}

			void Core::PopulatePluginsModel ()
			{
				QMap<QString, QList<ListPackageInfo> > infos;
				try
				{
					infos = Storage_->GetListPackageInfos ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to get package infos"
							<< e.what ();
					return;
				}

				Q_FOREACH (const QString& packageName, infos.keys ())
				{
					QList<ListPackageInfo> list = infos [packageName];
					std::sort (list.begin (), list.end (),
							boost::bind (IsVersionLess,
									boost::bind (&ListPackageInfo::Version_, _1),
									boost::bind (&ListPackageInfo::Version_, _2)));
				}
			}

			void Core::HandleNewPackages (const PackageShortInfoList& shortInfos,
					int componentId, const QString& component, const QUrl& repoUrl)
			{
				QMap<QString, QList<QString> > PackageName2NewVersions_;

				Q_FOREACH (const PackageShortInfo& info, shortInfos)
					Q_FOREACH (const QString& version, info.Versions_)
					{
						int packageId = -1;
						try
						{
							packageId = Storage_->FindPackage (info.Name_, version);
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to get package ID for"
									<< info.Name_
									<< version
									<< e.what ();
							emit gotEntity (Util::MakeNotification (tr ("Error parsing component"),
									tr ("Unable to load package ID for package `%1`-%2")
										.arg (info.Name_)
										.arg (version),
									PCritical_));
							return;
						}

						try
						{
							if (packageId == -1)
								PackageName2NewVersions_ [info.Name_] << version;
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to get save package"
									<< info.Name_
									<< version
									<< e.what ();
							emit gotEntity (Util::MakeNotification (tr ("Error parsing component"),
									tr ("Unable to save package `%1`-%2")
										.arg (info.Name_)
										.arg (version),
									PCritical_));
							return;
						}

						try
						{
							if (packageId != -1 &&
									!Storage_->HasLocation (packageId, componentId))
								Storage_->AddLocation (packageId, componentId);
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to add package location"
									<< info.Name_
									<< version
									<< e.what ();
							emit gotEntity (Util::MakeNotification (tr ("Error parsing component"),
									tr ("Unable to save package location for "
										"package `%1`-%2 and component %3")
										.arg (info.Name_)
										.arg (version)
										.arg (component),
									PCritical_));
							return;
						}
					}


				Q_FOREACH (QString packageName, PackageName2NewVersions_.keys ())
				{
					QUrl packageUrl = repoUrl;
					QString normalized = packageName.toLower ().simplified ();
					normalized.remove (' ');
					normalized.remove ('\t');
					packageUrl.setPath (packageUrl.path () +
							"/dists/" + component + "/all" +
							'/' + normalized +
							'/' + normalized +
							".xml.xz");
					RepoInfoFetcher_->FetchPackageInfo (packageUrl,
							packageName,
							PackageName2NewVersions_ [packageName],
							componentId);
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

			void Core::handleComponentFetched (const PackageShortInfoList& shortInfos,
					const QString& component, int repoId)
			{
				int componentId = -1;
				QUrl repoUrl;
				try
				{
					componentId = Storage_->FindComponent (repoId, component);
					if (componentId == -1)
						componentId = Storage_->AddComponent (repoId, component);

					repoUrl = Storage_->GetRepo (repoId).GetUrl ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to get component ID for"
							<< component
							<< "of"
							<< repoId
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Error parsing component"),
							tr ("Unable to load component ID for component %1.")
								.arg (component),
							PCritical_));
					return;
				}

				QList<int> presentPackages;
				try
				{
					presentPackages = Storage_->GetPackagesInComponent (componentId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to get present packages in component:"
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Error handling component"),
							tr ("Unable to load packages already present in the component %1.")
								.arg (component),
							PCritical_));
					return;
				}

				Q_FOREACH (int presentPId, presentPackages)
				{
					PackageShortInfo psi;
					try
					{
						psi = Storage_->GetPackage (presentPId);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to get present package:"
								<< e.what ();
						emit gotEntity (Util::MakeNotification (tr ("Error handling component"),
								tr ("Unable to load package already present in the component %1.")
									.arg (component),
								PCritical_));
						return;
					}

					bool found = false;
					Q_FOREACH (const PackageShortInfo& candidate, shortInfos)
					{
						if (candidate.Name_ != psi.Name_)
							continue;
						if (candidate.Versions_.contains (psi.Versions_.at (0)))
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						try
						{
							Storage_->RemovePackage (presentPId);
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to remove package for:"
									<< component
									<< presentPId
									<< e.what ();
							emit gotEntity (Util::MakeNotification (tr ("Error handling component"),
									tr ("Unable to remove package which has been removed upstream from %1.")
										.arg (component),
									PCritical_));
							return;
						}
					}
				}

				HandleNewPackages (shortInfos, componentId, component, repoUrl);
			}

			void Core::handlePackageFetched (const PackageInfo& pInfo,
					int componentId)
			{
				try
				{
					Storage_->AddPackages (pInfo);
					Q_FOREACH (const QString& version, pInfo.Versions_)
					{
						int packageId =
								Storage_->FindPackage (pInfo.Name_, version);
						Storage_->AddLocation (packageId, componentId);
					}
				}
				catch (const std::runtime_error& e)
				{
					pInfo.Dump ();
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Error retrieving package"),
							tr ("Unable to save package %1.")
								.arg (pInfo.Name_),
							PCritical_));
				}
			}
		}
	}
}
