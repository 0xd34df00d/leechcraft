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
#include <QDir>
#include <QtDebug>
#include <plugininterface/util.h>
#include <config.h>
#include "repoinfofetcher.h"
#include "storage.h"
#include "packagesmodel.h"
#include "externalresourcemanager.h"
#include "pendingmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			QMap<Dependency::Relation, Comparator_t> Relation2comparator;

			Core::Core ()
			: RepoInfoFetcher_ (new RepoInfoFetcher (this))
			, ExternalResourceManager_ (new ExternalResourceManager (this))
			, Storage_ (new Storage (this))
			, PluginsModel_ (new PackagesModel (this))
			, PendingManager_ (new PendingManager (this))
			{
				Relation2comparator [Dependency::L] = IsVersionLess;
				Relation2comparator [Dependency::GE] = boost::bind (std::logical_not<bool> (),
						Relation2comparator [Dependency::L]);
				Relation2comparator [Dependency::E] = std::equal_to<QString> ();
				Relation2comparator [Dependency::LE] = boost::bind (std::logical_or<bool> (),
						Relation2comparator [Dependency::L], Relation2comparator [Dependency::E]);
				Relation2comparator [Dependency::G] = boost::bind (std::logical_not<bool> (),
						Relation2comparator [Dependency::LE]);
				Relation2comparator [Dependency::G] = boost::bind (Relation2comparator [Dependency::L], _2, _1);

				connect (ExternalResourceManager_,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
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

				PopulatePluginsModel ();
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

			PendingManager* Core::GetPendingManager () const
			{
				return PendingManager_;
			}

			DependencyList Core::GetDependencies (int packageId) const
			{
				DependencyList result;
				Q_FOREACH (const Dependency& dep, Storage_->GetDependencies (packageId))
					if (dep.Type_ == Dependency::TRequires)
						result << dep;
				return result;
			}

			QList<ListPackageInfo> Core::GetDependencyFulfillers (const Dependency& dep) const
			{
				return Storage_->GetFulfillers (dep);
			}

			bool Core::IsVersionOk (const QString& candidate, QString refVer) const
			{
				Dependency::Relation relation;

				if (refVer.startsWith (">="))
				{
					relation = Dependency::GE;
					refVer = refVer.mid (2);
				}
				else if (refVer.startsWith ("<="))
				{
					relation = Dependency::LE;
					refVer = refVer.mid (2);
				}
				else if (refVer.startsWith ('>'))
				{
					relation = Dependency::G;
					refVer = refVer.mid (1);
				}
				else if (refVer.startsWith ('<'))
				{
					relation = Dependency::L;
					refVer = refVer.mid (1);
				}
				else
				{
					relation = Dependency::E;
					if (refVer.startsWith ('='))
						refVer = refVer.mid (1);
				}
				refVer = refVer.trimmed ();

				return Relation2comparator [relation] (candidate, refVer);
			}

			bool Core::IsFulfilled (const Dependency& dep) const
			{
				Q_FOREACH (const InstalledDependencyInfo& info, GetAllInstalledPackages ())
					if (info.Dep_.Name_ == dep.Name_ &&
							IsVersionOk (info.Dep_.Version_, dep.Version_))
						return true;

				return false;
			}

			QIcon Core::GetIconForLPI (const ListPackageInfo& packageInfo)
			{
				QIcon result;

				boost::optional<QByteArray> data;
				if (!packageInfo.IconURL_.isEmpty ())
				{
					try
					{
						data = ExternalResourceManager_->
								GetResourceData (packageInfo.IconURL_);
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "could not download icon at"
								<< packageInfo.IconURL_
								<< e.what ();
					}
				}

				if (data)
				{
					QPixmap px;
					if (px.loadFromData (*data) &&
							!px.isNull ())
					{
						result = QIcon (px);
						return result;
					}
				}

				switch (packageInfo.Type_)
				{
				case PackageInfo::TPlugin:
					result = Proxy_->GetIcon ("lackman_plugin");
					break;
				case PackageInfo::TIconset:
					result = Proxy_->GetIcon ("lackman_iconset");
					break;
				case PackageInfo::TTranslation:
					result = Proxy_->GetIcon ("lackman_translation");
					break;
				}
				return result;
			}

			ListPackageInfo Core::GetListPackageInfo (int packageId)
			{
				return Storage_->GetSingleListPackageInfo (packageId);
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

			void Core::CancelPending ()
			{
				PendingManager_->Reset ();
			}

			void Core::AcceptPending ()
			{
				QSet<int> toInstall = PendingManager_->GetPendingInstall ();
				QSet<int> toRemove = PendingManager_->GetPendingRemove ();
				QSet<int> toUpdate = PendingManager_->GetPendingUpdate ();
				PendingManager_->Reset ();
			}

			QStringList Core::GetAllTags () const
			{
				return Storage_->GetAllTags ();
			}

			InstalledDependencyInfoList Core::GetSystemInstalledPackages () const
			{
				InstalledDependencyInfoList result;

				QStringList entries;
#if defined(Q_WS_X11)
				entries += QDir ("/usr/share/leechcraft/installed").entryList ();
				entries += QDir ("/usr/local/share/leechcraft/installed").entryList ();
#endif

				QString nameStart ("Name: ");
				QString versionStart ("Version: ");

				Q_FOREACH (const QString& entry, entries)
				{
					QFile file (entry);
					if (!file.open (QIODevice::ReadOnly))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to open"
								<< entry
								<< "for reading, skipping it.";
						continue;
					}

					InstalledDependencyInfo info;
					info.Source_ = InstalledDependencyInfo::SSystem;

					QStringList lines = QString (file.readAll ()).split ('\n', QString::SkipEmptyParts);
					Q_FOREACH (const QString& untrimmed, lines)
					{
						QString string = untrimmed.trimmed ();
						if (string.startsWith (nameStart))
							info.Dep_.Name_ = string.mid (nameStart.length ());
						else if (string.startsWith (versionStart))
							info.Dep_.Version_ = string.mid (versionStart.length ());
					}

					if (info.Dep_.Version_.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "dependency version for"
								<< info.Dep_.Name_
								<< "not filled, defaulting to"
								<< LEECHCRAFT_VERSION;
						info.Dep_.Version_ = LEECHCRAFT_VERSION;
					}
				}

				return result;
			}

			InstalledDependencyInfoList Core::GetLackManInstalledPackages () const
			{
				return Storage_->GetInstalledPackages ();
			}

			InstalledDependencyInfoList Core::GetAllInstalledPackages () const
			{
				return GetSystemInstalledPackages () + GetLackManInstalledPackages ();
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

				InstalledDependencyInfoList instedAll = GetLackManInstalledPackages ();

				Q_FOREACH (const QString& packageName, infos.keys ())
				{
					QList<ListPackageInfo> list = infos [packageName];
					std::sort (list.begin (), list.end (),
							boost::bind (IsVersionLess,
									boost::bind (&ListPackageInfo::Version_, _1),
									boost::bind (&ListPackageInfo::Version_, _2)));
					ListPackageInfo last = list.last ();

					Q_FOREACH (const InstalledDependencyInfo& idi,
							instedAll)
						if (last.Name_ == idi.Dep_.Name_)
						{
							last.IsInstalled_ = true;

							if (idi.Source_ == InstalledDependencyInfo::SLackMan &&
									IsVersionLess (idi.Dep_.Version_, last.Version_))
								last.HasNewVersion_ = true;

							break;
						}

					QString type;
					switch (last.Type_)
					{
					case PackageInfo::TPlugin:
						type = tr ("plugin");
						break;
					case PackageInfo::TIconset:
						type = tr ("iconset");
						break;
					case PackageInfo::TTranslation:
						type = tr ("translation");
						break;
					}

					PluginsModel_->AddRow (last);
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

					QStringList versions = pInfo.Versions_;
					std::sort (versions.begin (), versions.end (), IsVersionLess);
					QString greatest = versions.last ();

					Q_FOREACH (const QString& version, pInfo.Versions_)
					{
						int packageId =
								Storage_->FindPackage (pInfo.Name_, version);
						Storage_->AddLocation (packageId, componentId);

						if (version == greatest)
						{
							QString existing = PluginsModel_->FindPackage (pInfo.Name_).Version_;
							if (existing.isEmpty ())
								PluginsModel_->AddRow (Storage_->
										GetSingleListPackageInfo (packageId));
							else if (IsVersionLess (existing, greatest))
								PluginsModel_->UpdateRow (Storage_->
										GetSingleListPackageInfo (packageId));
						}
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

				if (pInfo.IconURL_.isValid ())
				{
					try
					{
						ExternalResourceManager_->GetResourceData (pInfo.IconURL_);
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "error fetching icon from"
								<< pInfo.IconURL_
								<< e.what ();
						emit gotEntity (Util::MakeNotification (tr ("Error retrieving package icon"),
								tr ("Unable to retrieve icon for package %1.")
									.arg (pInfo.Name_),
								PCritical_));
					}
				}

				Q_FOREACH (const Image& image, pInfo.Images_)
					try
					{
						ExternalResourceManager_->
								GetResourceData (QUrl::fromEncoded (image.URL_.toUtf8 ()));
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "error fetching"
								<< image.URL_
								<< e.what ();
						emit gotEntity (Util::MakeNotification (tr ("Error retrieving image"),
								tr ("Unable to retrieve image for package %1 from %2.")
									.arg (pInfo.Name_)
									.arg (image.URL_),
								PCritical_));
					}
			}
		}
	}
}
