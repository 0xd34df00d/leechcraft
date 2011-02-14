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
#include <QApplication>
#include <QString>
#include <QStandardItemModel>
#include <QDir>
#include <QtDebug>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/datasourceroles.h>
#include "repoinfofetcher.h"
#include "storage.h"
#include "packagesmodel.h"
#include "externalresourcemanager.h"
#include "pendingmanager.h"
#include "packageprocessor.h"
#include "versioncomparator.h"
#include "xmlsettingsmanager.h"

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
			, PackagesModel_ (new PackagesModel (this))
			, PendingManager_ (new PendingManager (this))
			, PackageProcessor_ (new PackageProcessor (this))
			, ReposModel_ (new QStandardItemModel (this))
			{
				Relation2comparator [Dependency::L] = IsVersionLess;
				Relation2comparator [Dependency::G] = boost::bind (Relation2comparator [Dependency::L], _2, _1);
				Relation2comparator [Dependency::GE] = boost::bind (std::logical_not<bool> (),
						boost::bind (Relation2comparator [Dependency::L], _1, _2));
				Relation2comparator [Dependency::E] = std::equal_to<QString> ();
				Relation2comparator [Dependency::LE] = boost::bind (std::logical_not<bool> (),
						boost::bind (Relation2comparator [Dependency::G], _1, _2));

				connect (Storage_,
						SIGNAL (packageRemoved (int)),
						this,
						SLOT (handlePackageRemoved (int)));
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
				connect (PackageProcessor_,
						SIGNAL (packageInstallError (int, const QString&)),
						this,
						SLOT (handlePackageInstallError (int, const QString&)));
				connect (PackageProcessor_,
						SIGNAL (packageInstalled (int)),
						this,
						SLOT (handlePackageInstalled (int)));
				connect (PackageProcessor_,
						SIGNAL (packageUpdated (int, int)),
						this,
						SLOT (handlePackageUpdated (int, int)));

				QStandardItem *item = new QStandardItem (tr ("URL"));
				item->setData (DataSources::DFTUrl, DataSources::DSRFieldType);
				ReposModel_->setHorizontalHeaderItem (0, item);
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::FinishInitialization ()
			{
				PendingManager_->Reset ();
				PopulatePluginsModel ();
			}

			void Core::Release ()
			{
				delete RepoInfoFetcher_;
				RepoInfoFetcher_ = 0;

				delete Storage_;
				Storage_ = 0;
			}

			void Core::SecondInit ()
			{
				ReadSettings ();
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
				return PackagesModel_;
			}

			PendingManager* Core::GetPendingManager () const
			{
				return PendingManager_;
			}

			ExternalResourceManager* Core::GetExtResourceManager () const
			{
				return ExternalResourceManager_;
			}

			Storage* Core::GetStorage () const
			{
				return Storage_;
			}

			QAbstractItemModel* Core::GetRepositoryModel () const
			{
				return ReposModel_;
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
				case PackageInfo::TData:
					result = Proxy_->GetIcon ("lackman_data");
					break;
				case PackageInfo::TTheme:
					result = Proxy_->GetIcon ("lackman_theme");
					break;
				}
				return result;
			}

			ListPackageInfo Core::GetListPackageInfo (int packageId)
			{
				return Storage_->GetSingleListPackageInfo (packageId);
			}

			QList<QUrl> Core::GetPackageURLs (int packageId) const
			{
				QList<QUrl> result;

				QMap<int, QList<QString> > repo2cmpt = Storage_->GetPackageLocations (packageId);

				PackageShortInfo info = Storage_->GetPackage (packageId);
				QString pathAddition = QString ("dists/%1/all/");
				QString normalized = NormalizePackageName (info.Name_);
				pathAddition += QString ("%1/%1-%2.tar.gz")
						.arg (normalized)
						.arg (info.Versions_.at (0));

				Q_FOREACH (int repoId, repo2cmpt.keys ())
				{
					RepoInfo ri = Storage_->GetRepo (repoId);
					QUrl url = ri.GetUrl ();
					QString path = url.path ();
					if (!path.endsWith ('/'))
						path += '/';

					Q_FOREACH (const QString& component, repo2cmpt [repoId])
					{
						QUrl tmp = url;
						tmp.setPath (path + pathAddition.arg (component));
						result << tmp;
					}
				}

				return result;
			}

			namespace
			{
				void SafeCD (QDir& dir, const QString& subdir)
				{
					if (!dir.exists (subdir))
						dir.mkdir (subdir);
					if (!dir.cd (subdir))
						throw std::runtime_error (QObject::tr ("Unable to cd into %1.")
								.arg (subdir)
								.toUtf8 ().constData ());
				}
			}

			QDir Core::GetPackageDir (int packageId) const
			{
				ListPackageInfo info = Storage_->GetSingleListPackageInfo (packageId);
				QDir dir = QDir::home ();
				dir.cd (".leechcraft");
				switch (info.Type_)
				{
				case PackageInfo::TPlugin:
					SafeCD (dir, "plugins");
					SafeCD (dir, "scriptable");
					SafeCD (dir, info.Language_);
					break;
				case PackageInfo::TIconset:
					SafeCD (dir, "icons");
					break;
				case PackageInfo::TTranslation:
					SafeCD (dir, "translations");
					break;
				case PackageInfo::TData:
				case PackageInfo::TTheme:
					SafeCD (dir, "data");
					break;
				}
				return dir;
			}

			void Core::AddRepo (const QUrl& url)
			{
				QStandardItem *item = new QStandardItem (url.toString ());
				item->setData (url);
				ReposModel_->appendRow (item);
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

			void Core::cancelPending ()
			{
				PendingManager_->Reset ();
			}

			void Core::acceptPending ()
			{
				QSet<int> toInstall = PendingManager_->GetPendingInstall ();
				QSet<int> toRemove = PendingManager_->GetPendingRemove ();
				QSet<int> toUpdate = PendingManager_->GetPendingUpdate ();

				Q_FOREACH (int packageId, toRemove)
					PerformRemoval (packageId);

				Q_FOREACH (int packageId, toInstall)
				{
					try
					{
						PackageProcessor_->Install (packageId);
					}
					catch (const std::exception& e)
					{
						QString str = Util::FromStdString (e.what ());
						qWarning () << Q_FUNC_INFO
								<< "got"
								<< str
								<< "while installing"
								<< packageId;
						emit gotEntity (Util::MakeNotification (tr ("Unable to install package"),
									str,
									PCritical_));
						continue;
					}
				}

				Q_FOREACH (int packageId, toUpdate)
				{
					try
					{
						PackageProcessor_->Update (packageId);
					}
					catch (const std::exception& e)
					{
						QString str = Util::FromStdString (e.what ());
						qWarning () << Q_FUNC_INFO
								<< "got"
								<< str
								<< "while updating to"
								<< packageId;
						emit gotEntity (Util::MakeNotification (tr ("Unable to update package"),
									str,
									PCritical_));
						continue;
					}
				}
			}

			QString Core::NormalizePackageName (const QString& packageName) const
			{
				QString normalized = packageName.toLower ().simplified ();
				normalized.remove (' ');
				normalized.remove ('\t');
				return normalized;
			}

			QStringList Core::GetAllTags () const
			{
				return Storage_->GetAllTags ();
			}

			InstalledDependencyInfoList Core::GetSystemInstalledPackages () const
			{
				InstalledDependencyInfoList result;

				QFileInfoList infoEntries;
#if defined(Q_WS_X11)
				infoEntries += QDir ("/usr/share/leechcraft/installed")
						.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
				infoEntries += QDir ("/usr/local/share/leechcraft/installed")
						.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#elif defined(Q_WS_WIN)
				infoEntries += QDir (QApplication::applicationDirPath () + "/share/installed")
						.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#elif defined(Q_WS_MAC)
				infoEntries += QDir (QCoreApplication::applicationDirPath () + "/../installed")
						.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

#endif

				QStringList entries;

				Q_FOREACH (const QFileInfo& info, infoEntries)
				{
					QString path = info.absoluteFilePath ();
					if (info.isFile ())
						entries << path;
					else if (info.isDir ())
						Q_FOREACH (const QFileInfo& subInfo,
								QDir (path).entryInfoList (QDir::Files))
							entries << subInfo.absoluteFilePath ();
				}

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
								<< Proxy_->GetVersion ();
						info.Dep_.Version_ = Proxy_->GetVersion ();
					}

					result << info;
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

					PackagesModel_->AddRow (last);
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
					QString normalized = NormalizePackageName (packageName);
					packageUrl.setPath (packageUrl.path () +
							"/dists/" + component + "/all" +
							'/' + normalized +
							'/');
					RepoInfoFetcher_->FetchPackageInfo (packageUrl,
							packageName,
							PackageName2NewVersions_ [packageName],
							componentId);
				}
			}

			void Core::PerformRemoval (int packageId)
			{
				try
				{
					PackageProcessor_->Remove (packageId);
				}
				catch (const std::exception& e)
				{
					QString str = Util::FromStdString (e.what ());
					qWarning () << Q_FUNC_INFO
							<< "got"
							<< str
							<< "while removing"
							<< packageId;
					emit gotEntity (Util::MakeNotification (tr ("Unable to remove package"),
								str,
								PCritical_));
					return;
				}

				if (!RecordUninstalled (packageId))
					return;

				UpdateRowFor (packageId);

				PendingManager_->SuccessfullyRemoved (packageId);

				emit packageRowActionFinished (GetPackageRow (packageId));
			}

			void Core::UpdateRowFor (int packageId)
			{
				try
				{
					ListPackageInfo info = Storage_->GetSingleListPackageInfo (packageId);
					PackagesModel_->UpdateRow (info);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while updating row for package"
							<< packageId
							<< "got this exception:"
							<< e.what ();
				}
			}

			bool Core::RecordInstalled (int packageId)
			{
				try
				{
					Storage_->AddToInstalled (packageId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while trying to record installed package"
							<< e.what ();
					emit gotEntity (Util::MakeNotification (tr ("Error installing package"),
								tr ("Error recording package to the package DB."),
								PCritical_));

					try
					{
						PackageProcessor_->Remove (packageId);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "while trying to cleanup partially installed package"
								<< e.what ();
					}
					return false;
				}

				return true;
			}

			bool Core::RecordUninstalled (int packageId)
			{
				try
				{
					Storage_->RemoveFromInstalled (packageId);
				}
				catch (const std::exception& e)
				{
					QString str = Util::FromStdString (e.what ());
					qWarning () << Q_FUNC_INFO
							<< "unable to remove from installed"
							<< packageId
							<< str;
					emit gotEntity (Util::MakeNotification (tr ("Unable to remove package"),
								str,
								PCritical_));
					return false;
				}

				return true;
			}

			int Core::GetPackageRow (int packageId) const
			{
				return PackagesModel_->GetRow (packageId);
			}

			void Core::ReadSettings ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_LackMan");
				int size = settings.beginReadArray ("Repos");
				for (int i = 0; i < size; ++i)
				{
					settings.setArrayIndex (i);
					QUrl url = settings.value ("URL").value<QUrl> ();
					AddRepo (url);
				}
				settings.endArray ();

				if (settings.value ("AddDefault", true).toBool ())
				{
					AddRepo (QUrl ("http://leechcraft.org/repo/"));
					settings.setValue ("AddDefault", false);
					WriteSettings ();
				}
			}

			void Core::WriteSettings ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_LackMan");
				settings.beginWriteArray ("Repos");
				for (int i = 0, size = ReposModel_->rowCount ();
						i < size; ++i)
				{
					settings.setArrayIndex (i);
					QStandardItem *item = ReposModel_->item (i);
					QUrl url = item->data ().value<QUrl> ();
					settings.setValue ("URL", url);
				}
				settings.endArray ();
			}

			void Core::updateAllRequested ()
			{
				for (int i = 0, size = ReposModel_->rowCount ();
						i < size; ++i)
				{
					QStandardItem *item = ReposModel_->item (i);
					QUrl url = item->data ().value<QUrl> ();
					QStringList components;

					try
					{
						int id = Storage_->FindRepo (url);
						components = Storage_->GetRepo (id).GetComponents ();
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "while trying to get repo components for"
								<< url
								<< e.what ();
						continue;
					}
					catch (...)
					{
						qWarning () << Q_FUNC_INFO
								<< "general error while trying to get repo components for"
								<< url;
						continue;
					}

					qDebug () << "would update" << url << components;
					UpdateRepo (url, components);
				}
			}

			void Core::upgradeAllRequested ()
			{
				for (int i = 0, rows = PackagesModel_->rowCount ();
						i < rows; ++i)
				{
					QModelIndex index = PackagesModel_->index (i, 0);
					int packageId = PackagesModel_->
							data (index, PackagesModel::PMRPackageID).toInt ();
					bool isUpgr = PackagesModel_->
							data (index, PackagesModel::PMRUpgradable).toBool ();

					if (isUpgr)
						PendingManager_->ToggleUpdate (packageId, true);
				}
			}

			void Core::removeRequested (const QString&, const QModelIndexList& list)
			{
				QList<int> rows;
				Q_FOREACH (const QModelIndex& index, list)
					rows << index.row ();

				std::sort (rows.begin (), rows.end ());
				std::reverse (rows.begin (), rows.end ());

				Q_FOREACH (int row, rows)
				{
					QList<QStandardItem*> items = ReposModel_->takeRow (row);
					QUrl url = items.at (RCURL)->data ().value<QUrl> ();

					try
					{
						int repoId = Storage_->FindRepo (url);
						Storage_->RemoveRepo (repoId);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to remove repo"
								<< url
								<< e.what ();
					}

					qDeleteAll (items);
				}

				WriteSettings ();
			}

			void Core::addRequested (const QString&, const QVariantList& list)
			{
				if (!list.size ())
				{
					qWarning () << Q_FUNC_INFO
							<< "too small list";
					return;
				}

				QString str = list.at (0).toString ();
				QUrl url = QUrl (str);
				if (!url.isValid ())
				{
					qWarning () << Q_FUNC_INFO
							<< "incorrect url"
							<< str;
					emit gotEntity (Util::MakeNotification (tr ("Repository addition error"),
							tr ("Incorrect URL %1.")
								.arg (str),
							PCritical_));
					return;
				}

				AddRepo (url);

				WriteSettings ();
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
							QString existing = PackagesModel_->
									FindPackage (pInfo.Name_).Version_;
							if (existing.isEmpty ())
								PackagesModel_->AddRow (Storage_->
										GetSingleListPackageInfo (packageId));
							else if (IsVersionLess (existing, greatest))
							{
								ListPackageInfo info = Storage_->GetSingleListPackageInfo (packageId);
								info.HasNewVersion_ = info.IsInstalled_;
								PackagesModel_->UpdateRow (info);
							}
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

			void Core::handlePackageInstallError (int packageId, const QString& error)
			{
				QString packageName;
				try
				{
					packageName = Storage_->GetPackage (packageId).Name_;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while trying to get erroneous package name"
							<< e.what ();
				}

				QString prepared = tr ("Error while fetching package %1: %2.");
				QString msg;
				if (packageName.size ())
					msg = prepared.arg (packageName).arg (error);
				else
					msg = prepared.arg (packageId).arg (error);

				emit gotEntity (Util::MakeNotification (tr ("Error installing package"),
							msg,
							PCritical_));
			}

			void Core::handlePackageInstalled (int packageId)
			{
				if (!RecordInstalled (packageId))
					return;

				UpdateRowFor (packageId);

				PendingManager_->SuccessfullyInstalled (packageId);

				QString packageName;
				try
				{
					packageName = Storage_->GetPackage (packageId).Name_;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while trying to get installed package name"
							<< e.what ();
					return;
				}

				emit gotEntity (Util::MakeNotification (tr ("Package installed"),
							tr ("Package %1 installed successfully.")
								.arg (packageName),
							PInfo_));

				emit packageRowActionFinished (GetPackageRow (packageId));
			}

			void Core::handlePackageUpdated (int fromId, int packageId)
			{
				if (!RecordUninstalled (fromId) ||
						!RecordInstalled (packageId))
					return;

				UpdateRowFor (packageId);

				PendingManager_->SuccessfullyUpdated (packageId);

				QString packageName;
				try
				{
					packageName = Storage_->GetPackage (packageId).Name_;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while trying to get installed package name"
							<< e.what ();
					return;
				}

				emit gotEntity (Util::MakeNotification (tr ("Package updated"),
							tr ("Package %1 updated successfully.")
								.arg (packageName),
							PInfo_));

				emit packageRowActionFinished (GetPackageRow (packageId));
			}

			void Core::handlePackageRemoved (int packageId)
			{
				PackagesModel_->RemovePackage (packageId);
			}
		}
	}
}
