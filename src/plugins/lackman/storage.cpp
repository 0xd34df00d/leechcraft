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

#include "storage.h"
#include <stdexcept>
#include <QDir>
#include <QSqlError>
#include <plugininterface/dblock.h>
#include <plugininterface/util.h>
#include "repoinfo.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			namespace
			{
				QString LoadQuery (const QString& name)
				{
					QFile file (QString (":/resources/sql/%1.sql").arg (name));
					if (!file.open (QIODevice::ReadOnly))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to open file"
								<< name
								<< "for reading";
						return QString ();
					}
					return file.readAll ();
				}

				QUrl Slashize (const QUrl& url)
				{
					if (url.path ().endsWith ('/'))
						return url;
					else
					{
						QUrl tmp = url;
						tmp.setPath (tmp.path () + '/');
						return tmp;
					}
				}
			}

			Storage::Storage (QObject *parent)
			: QObject (parent)
			, DB_ (QSqlDatabase::addDatabase ("QSQLITE", "LackManConnectionAvailable"))
			{
				QDir dir;
				try
				{
					dir = Util::CreateIfNotExists ("lackman");
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					throw;
				}

				DB_.setDatabaseName (dir.filePath ("availablepackages.db"));

				if (!DB_.open ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
								.arg (DB_.lastError ().text ())));
				}

				InitTables ();
				InitQueries ();
			}

			int Storage::CountPackages (const QUrl& repoUrl)
			{
				QueryCountPackages_.bindValue (":repo_url",
						Slashize (repoUrl).toEncoded ());
				if (!QueryCountPackages_.exec ())
				{
					Util::DBLock::DumpError (QueryCountPackages_);
					throw std::runtime_error ("Query execution failed.");
				}

				int value = 0;
				if (!QueryCountPackages_.next ())
					qWarning () << Q_FUNC_INFO
							<< "strange, next() returns false.";
				else
					value = QueryCountPackages_.value (0).toInt ();

				QueryCountPackages_.finish ();

				return value;
			}

			InstalledDependencyInfoList Storage::GetInstalledPackages ()
			{
				if (!QueryGetInstalledPackages_.exec ())
				{
					Util::DBLock::DumpError (QueryGetInstalledPackages_);
					throw std::runtime_error ("Query execution failed.");
				}

				InstalledDependencyInfoList result;

				while (QueryGetInstalledPackages_.next ())
				{
					Dependency dep =
					{
						Dependency::TProvides,
						QueryGetInstalledPackages_.value (0).toString (),
						QueryGetInstalledPackages_.value (1).toString ()
					};
					InstalledDependencyInfo info =
					{
						dep,
						InstalledDependencyInfo::SLackMan
					};

					result << info;
				}

				return result;
			}

			int Storage::FindRepo (const QUrl& repoUrl)
			{
				QueryFindRepo_.bindValue (":repo_url",
						Slashize (repoUrl).toEncoded ());
				if (!QueryFindRepo_.exec ())
				{
					Util::DBLock::DumpError (QueryFindRepo_);
					throw std::runtime_error ("Query execution failed.");
				}

				int value = -1;
				if (QueryFindRepo_.next ())
					value = QueryFindRepo_.value (0).toInt ();

				QueryFindRepo_.finish ();

				return value;
			}

			int Storage::AddRepo (const RepoInfo& ri)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "could not acquire DB lock";
					throw;
				}

				QueryAddRepo_.bindValue (":url", Slashize (ri.GetUrl ()).toEncoded ());
				QueryAddRepo_.bindValue (":name", ri.GetName ());
				QueryAddRepo_.bindValue (":description", ri.GetShortDescr ());
				QueryAddRepo_.bindValue (":longdescr", ri.GetLongDescr ());
				QueryAddRepo_.bindValue (":maint_name", ri.GetMaintainer ().Name_);
				QueryAddRepo_.bindValue (":maint_email", ri.GetMaintainer ().Email_);
				if (!QueryAddRepo_.exec ())
				{
					Util::DBLock::DumpError (QueryAddRepo_);
					throw std::runtime_error ("Query execution failed.");
				}

				QueryAddRepo_.finish ();

				int repoId = FindRepo (Slashize (ri.GetUrl ()));
				if (repoId == -1)
				{
					qWarning () << Q_FUNC_INFO
							<< "OH SHI~, just inserted repo cannot be found!";
					throw std::runtime_error ("Just inserted repo cannot be found.");
				}

				Q_FOREACH (const QString& component, ri.GetComponents ())
					AddComponent (repoId, component);

				lock.Good ();

				return repoId;
			}

			RepoInfo Storage::GetRepo (int repoId)
			{
				QueryGetRepo_.bindValue (":repo_id", repoId);
				if (!QueryGetRepo_.exec ())
				{
					Util::DBLock::DumpError (QueryGetRepo_);
					throw std::runtime_error ("Query execution failed.");
				}
				if (!QueryGetRepo_.next ())
				{
					qWarning () << Q_FUNC_INFO
							<< "could not position on next record";
					throw std::runtime_error ("Could not position on next record");
				}
				RepoInfo result (QUrl::fromEncoded (QueryGetRepo_.value (0).toString ().toUtf8 ()));

				result.SetName (QueryGetRepo_.value (1).toString ());
				result.SetShortDescr (QueryGetRepo_.value (2).toString ());
				result.SetLongDescr (QueryGetRepo_.value (3).toString ());
				MaintainerInfo info =
				{
					QueryGetRepo_.value (4).toString (),
					QueryGetRepo_.value (5).toString ()
				};
				result.SetMaintainer (info);

				QueryGetRepo_.finish ();

				result.SetComponents (GetComponents (repoId));

				return result;
			}

			QStringList Storage::GetComponents (int repoId)
			{
				QueryGetRepoComponents_.bindValue (":repo_id", repoId);
				if (!QueryGetRepoComponents_.exec ())
				{
					Util::DBLock::DumpError (QueryGetRepoComponents_);
					throw std::runtime_error ("Query execution failed");
				}

				QStringList result;
				while (QueryGetRepoComponents_.next ())
					result << QueryGetRepoComponents_.value (0).toString ();

				QueryGetRepoComponents_.finish ();

				return result;
			}

			int Storage::FindComponent (int repoId, const QString& component)
			{
				QueryFindComponent_.bindValue (":repo_id", repoId);
				QueryFindComponent_.bindValue (":component", component);
				if (!QueryFindComponent_.exec ())
				{
					Util::DBLock::DumpError (QueryFindComponent_);
					throw std::runtime_error ("Query execution failed");
				}

				int result = -1;
				if (QueryFindComponent_.next ())
					result = QueryFindComponent_.value (0).toInt ();

				QueryFindComponent_.finish ();

				return result;
			}

			int Storage::AddComponent (int repoId, const QString& component, bool returnId)
			{
				QueryAddRepoComponent_.bindValue (":repo_id", repoId);
				QueryAddRepoComponent_.bindValue (":component", component);
				if (!QueryAddRepoComponent_.exec ())
				{
					Util::DBLock::DumpError (QueryAddRepoComponent_);
					throw std::runtime_error ("Query execution failed.");
				}

				QueryAddRepoComponent_.finish ();

				if (!returnId)
					 return 0;

				return FindComponent (repoId, component);
			}

			void Storage::RemoveComponent (int repoId, const QString& component)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to start transaction";
					throw std::runtime_error ("Unable to start transaction");
				}

				int compId = FindComponent (repoId, component);
				if (compId == -1)
				{
					qWarning () << Q_FUNC_INFO
							<< "component"
							<< component
							<< "not found.";
					throw std::runtime_error ("Requested component not found");
				}

				QSqlQuery idsSelector (DB_);
				idsSelector.prepare ("SELECT DISTINCT package_id "
						"FROM locations WHERE component_id = :component_id");
				idsSelector.bindValue (":component_id", compId);
				if (!idsSelector.exec ())
				{
					Util::DBLock::DumpError (idsSelector);
					throw std::runtime_error ("Fetching of possibly affected packages failed.");
				}

				QList<int> possiblyAffected;
				while (idsSelector.next ())
					possiblyAffected << idsSelector.value (0).toInt ();

				idsSelector.finish ();

				QSqlQuery remover (DB_);
				remover.prepare ("DELETE FROM locations WHERE component_id = :component_id;");
				remover.bindValue (":component_id", compId);
				if (!remover.exec ())
				{
					Util::DBLock::DumpError (remover);
					throw std::runtime_error ("Unable to remove component from locations.");
				}
				remover.prepare ("DELETE FROM components WHERE component_id = :component_id;");
				remover.bindValue (":component_id", compId);
				if (!remover.exec ())
				{
					Util::DBLock::DumpError (remover);
					throw std::runtime_error ("Unable to remove component from components.");
				}

				remover.finish ();

				QSqlQuery checker (DB_);
				checker.prepare ("SELECT COUNT (package_id) FROM locations WHERE package_id = :package_id;");
				Q_FOREACH (int packageId, possiblyAffected)
				{
					checker.bindValue (":package_id", packageId);
					if (!checker.exec ())
					{
						Util::DBLock::DumpError (checker);
						throw std::runtime_error ("Unable to remove check affected.");
					}

					if (!checker.next ())
					{
						qWarning () << Q_FUNC_INFO
								<< "zarroo rows";
						throw std::runtime_error ("Unable to move to the next row");
					}

					if (checker.value (0).toInt ())
						continue;

					checker.finish ();

					remover.prepare ("DELETE FROM packages WHERE package_id = :package_id;");
					remover.bindValue (":package_id", packageId);
					if (!remover.exec ())
					{
						Util::DBLock::DumpError (remover);
						throw std::runtime_error ("Unable to remove orphaned package.");
					}

					remover.finish ();
				}

				lock.Good ();
			}

			int Storage::FindPackage (const QString& name, const QString& version)
			{
				QueryFindPackage_.bindValue (":name", name);
				QueryFindPackage_.bindValue (":version", version);
				if (!QueryFindPackage_.exec ())
				{
					Util::DBLock::DumpError (QueryFindPackage_);
					throw std::runtime_error ("Query execution failed");
				}

				int result = -1;
				if (QueryFindPackage_.next ())
					result = QueryFindPackage_.value (0).toInt ();

				QueryFindPackage_.finish ();

				return result;
			}

			PackageShortInfo Storage::GetPackage (int packageId)
			{
				QueryGetPackage_.bindValue (":package_id", packageId);
				if (!QueryGetPackage_.exec ())
				{
					Util::DBLock::DumpError (QueryGetPackage_);
					throw std::runtime_error ("Query execution failed");
				}

				if (!QueryGetPackage_.next ())
				{
					QString str = QString ("package with id %1 not found")
							.arg (packageId);
					qWarning () << Q_FUNC_INFO
							<< str;
					throw std::runtime_error (qPrintable (str));
				}

				PackageShortInfo info =
				{
					QueryGetPackage_.value (0).toString (),
					QStringList (QueryGetPackage_.value (1).toString ())
				};
				QueryGetPackage_.finish ();

				return info;
			}

			void Storage::RemovePackage (int packageId)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to begin transaction:"
							<< e.what ();
					throw;
				}

				QString name = GetPackage (packageId).Name_;

				QueryClearTags_.bindValue (":name", name);
				if (!QueryClearTags_.exec ())
				{
					Util::DBLock::DumpError (QueryClearTags_);
					throw std::runtime_error ("Query execution failed");
				}

				QueryClearPackageInfos_.bindValue (":name", name);
				if (!QueryClearPackageInfos_.exec ())
				{
					Util::DBLock::DumpError (QueryClearPackageInfos_);
					throw std::runtime_error ("Query execution failed");
				}

				QueryClearImages_.bindValue (":name", name);
				if (!QueryClearImages_.exec ())
				{
					Util::DBLock::DumpError (QueryClearImages_);
					throw std::runtime_error ("Query execution failed");
				}

				QueryRemovePackageFromLocations_.bindValue (":package_id", packageId);
				if (!QueryRemovePackageFromLocations_.exec ())
				{
					Util::DBLock::DumpError (QueryRemovePackageFromLocations_);
					throw std::runtime_error ("Query execution failed");
				}

				QueryRemovePackage_.bindValue (":package_id", packageId);
				if (!QueryRemovePackage_.exec ())
				{
					Util::DBLock::DumpError (QueryRemovePackage_);
					throw std::runtime_error ("Query execution failed");
				}

				lock.Good ();
			}

			void Storage::AddPackages (const PackageInfo& pInfo)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ()
							<< "while acquiring lock";
					throw;
				}

				Q_FOREACH (const QString& version, pInfo.Versions_)
				{
					QueryAddPackage_.bindValue (":name", pInfo.Name_);
					QueryAddPackage_.bindValue (":version", version);
					if (!QueryAddPackage_.exec ())
					{
						Util::DBLock::DumpError (QueryAddPackage_);
						throw std::runtime_error ("Query execution failed");
					}
				}
				QueryAddPackage_.finish ();

				QueryClearPackageInfos_.bindValue (":name", pInfo.Name_);
				if (!QueryClearPackageInfos_.exec ())
				{
					Util::DBLock::DumpError (QueryAddPackage_);
					throw std::runtime_error ("Query execution failed");
				}
				QueryClearPackageInfos_.finish ();

				QueryAddPackageInfo_.bindValue (":name", pInfo.Name_);
				QueryAddPackageInfo_.bindValue (":short_descr", pInfo.Description_);
				QueryAddPackageInfo_.bindValue (":long_descr", pInfo.LongDescription_);
				QueryAddPackageInfo_.bindValue (":type", pInfo.Type_);
				QueryAddPackageInfo_.bindValue (":language", pInfo.Language_);
				QueryAddPackageInfo_.bindValue (":maint_name", pInfo.MaintName_);
				QueryAddPackageInfo_.bindValue (":maint_email", pInfo.MaintEmail_);
				QueryAddPackageInfo_.bindValue (":icon_url", pInfo.IconURL_);
				if (!QueryAddPackageInfo_.exec ())
				{
					Util::DBLock::DumpError (QueryAddPackageInfo_);
					throw std::runtime_error ("Query execution failed");
				}
				QueryAddPackageInfo_.finish ();

				QueryClearTags_.bindValue (":name", pInfo.Name_);
				if (!QueryClearTags_.exec ())
				{
					Util::DBLock::DumpError (QueryClearTags_);
					throw std::runtime_error ("Query execution failed");
				}
				QueryClearTags_.finish ();

				Q_FOREACH (const QString& tag, pInfo.Tags_)
				{
					QueryAddTag_.bindValue (":name", pInfo.Name_);
					QueryAddTag_.bindValue (":tag", tag);
					if (!QueryAddTag_.exec ())
					{
						Util::DBLock::DumpError (QueryAddTag_);
						throw std::runtime_error ("Query execution failed");
					}
				}
				QueryAddTag_.finish ();

				QueryClearImages_.bindValue (":name", pInfo.Name_);
				if (!QueryClearImages_.exec ())
				{
					Util::DBLock::DumpError (QueryClearImages_);
					throw std::runtime_error ("Query execution failed");
				}
				QueryClearImages_.finish ();

				Q_FOREACH (const Image& img, pInfo.Images_)
				{
					QueryAddImage_.bindValue (":name", pInfo.Name_);
					QueryAddImage_.bindValue (":url", img.URL_);
					QueryAddImage_.bindValue (":type", img.Type_);
					if (!QueryAddImage_.exec ())
					{
						Util::DBLock::DumpError (QueryAddImage_);
						throw std::runtime_error ("Query execution failed");
					}
				}
				QueryAddImage_.finish ();

				Q_FOREACH (const QString& thisVersion, pInfo.Deps_.keys ())
				{
					int packageId = -1;
					try
					{
						packageId = FindPackage (pInfo.Name_, thisVersion);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to get package ID for"
								<< pInfo.Name_
								<< thisVersion
								<< "with error:"
								<< e.what ();
						throw;
					}

					if (packageId == -1)
					{
						qWarning () << Q_FUNC_INFO
								<< "package ID for"
								<< pInfo.Name_
								<< thisVersion
								<< "not found";
						throw std::runtime_error (qPrintable (QString ("package ID for %1 "
								"version %2 not found.")
									.arg (pInfo.Name_)
									.arg (thisVersion)));
					}

					QueryClearDeps_.bindValue (":package_id", packageId);
					if (!QueryClearDeps_.exec ())
					{
						Util::DBLock::DumpError (QueryClearDeps_);
						throw std::runtime_error ("Query execution failed");
					}
					QueryClearDeps_.finish ();

					Q_FOREACH (const Dependency& dep, pInfo.Deps_ [thisVersion])
					{
						QueryAddDep_.bindValue (":package_id", packageId);
						QueryAddDep_.bindValue (":name", dep.Name_);
						QueryAddDep_.bindValue (":version", dep.Version_);
						QueryAddDep_.bindValue (":type", dep.Type_);
						if (!QueryAddDep_.exec ())
						{
							Util::DBLock::DumpError (QueryAddDep_);
							throw std::runtime_error ("Query execution failed");
						}
					}
					QueryAddDep_.finish ();
				}

				lock.Good ();
			}

			QList<int> Storage::GetPackagesInComponent (int componentId)
			{
				QueryGetPackagesInComponent_.bindValue (":component_id", componentId);
				if (!QueryGetPackagesInComponent_.exec ())
				{
					Util::DBLock::DumpError (QueryGetPackagesInComponent_);
					throw std::runtime_error ("Query execution failed");
				}

				QList<int> result;
				while (QueryGetPackagesInComponent_.next ())
					result << QueryGetPackagesInComponent_.value (0).toInt ();

				QueryGetPackagesInComponent_.finish ();
				return result;
			}

			QMap<QString, QList<ListPackageInfo> > Storage::GetListPackageInfos ()
			{
				if (!QueryGetListPackageInfos_.exec ())
				{
					Util::DBLock::DumpError (QueryGetListPackageInfos_);
					throw std::runtime_error ("Query execution failed");
				}

				QMap<QString, QList<ListPackageInfo> > result;
				while (QueryGetListPackageInfos_.next ())
				{
					int packageId = QueryGetListPackageInfos_.value (0).toInt ();
					QString name = QueryGetListPackageInfos_.value (1).toString ();

					ListPackageInfo info =
					{
						packageId,
						name,
						QueryGetListPackageInfos_.value (2).toString (),
						QueryGetListPackageInfos_.value (3).toString (),
						QueryGetListPackageInfos_.value (4).toString (),
						static_cast<PackageInfo::Type> (QueryGetListPackageInfos_.value (5).toInt ()),
						QUrl::fromEncoded (QueryGetListPackageInfos_.value (6).toString ().toUtf8 ()),
						GetPackageTags (packageId)
					};

					result [name] << info;
				}

				QueryGetListPackageInfos_.finish ();

				return result;
			}

			ListPackageInfo Storage::GetSingleListPackageInfo (int packageId)
			{
				QueryGetSingleListPackageInfo_.bindValue (":package_id", packageId);
				if (!QueryGetSingleListPackageInfo_.exec ())
				{
					Util::DBLock::DumpError (QueryGetSingleListPackageInfo_);
					throw std::runtime_error ("Query execution failed");
				}

				QMap<QString, QList<ListPackageInfo> > result;
				if (!QueryGetSingleListPackageInfo_.next ())
				{
					qWarning () << Q_FUNC_INFO
							<< "package with package ID"
							<< packageId
							<< "not found;";
					QString str = tr ("Package with ID %1 not found.")
							.arg (packageId);
					throw std::runtime_error (qPrintable (str));
				}

				QString name = QueryGetSingleListPackageInfo_.value (1).toString ();

				ListPackageInfo info =
				{
					packageId,
					name,
					QueryGetSingleListPackageInfo_.value (2).toString (),
					QueryGetSingleListPackageInfo_.value (3).toString (),
					QueryGetSingleListPackageInfo_.value (4).toString (),
					static_cast<PackageInfo::Type> (QueryGetSingleListPackageInfo_.value (5).toInt ()),
					QUrl::fromEncoded (QueryGetSingleListPackageInfo_.value (6).toString ().toUtf8 ()),
					GetPackageTags (packageId)
				};

				QueryGetSingleListPackageInfo_.finish ();

				return info;
			}

			DependencyList Storage::GetDependencies (int packageId)
			{
				QueryGetDependencies_.bindValue (":package_id", packageId);
				if (!QueryGetDependencies_.exec ())
				{
					Util::DBLock::DumpError (QueryGetDependencies_);
					throw std::runtime_error ("Query execution failed");
				}

				DependencyList result;

				while (QueryGetDependencies_.next ())
				{
					Dependency::Type type;
					QString typeString = QueryGetDependencies_.value (0).toString ();
					if (typeString == "depends")
						type = Dependency::TRequires;
					else if (typeString == "requires")
						type = Dependency::TProvides;
					else
					{
						qWarning () << Q_FUNC_INFO
								<< "unknown type"
								<< typeString;
						QString err = tr ("Unknown dependency type `%1`.")
								.arg (typeString);
						throw std::runtime_error (qPrintable (err));
					}

					Dependency dep =
					{
						type,
						QueryGetDependencies_.value (1).toString (),
						QueryGetDependencies_.value (2).toString ()
					};
					result << dep;
				}

				QueryGetDependencies_.finish ();

				return result;
			}

			QList<ListPackageInfo> Storage::GetFulfillers (const Dependency& dep)
			{
				QueryGetFulfillerCandidates_.bindValue (":name", dep.Name_);
				if (!QueryGetFulfillerCandidates_.exec ())
				{
					Util::DBLock::DumpError (QueryGetPackageTags_);
					throw std::runtime_error ("Query execution failed");
				}

				QList<ListPackageInfo> result;

				while (QueryGetFulfillerCandidates_.next ())
				{
					int packageId = QueryGetFulfillerCandidates_.value (0).toInt ();
					QString version = QueryGetFulfillerCandidates_.value (1).toString ();

					if (Core::Instance ().IsVersionOk (version, dep.Version_))
						result << GetSingleListPackageInfo (packageId);
				}

				return result;
			}

			QStringList Storage::GetPackageTags (int packageId)
			{
				QueryGetPackageTags_.bindValue (":package_id", packageId);
				if (!QueryGetPackageTags_.exec ())
				{
					Util::DBLock::DumpError (QueryGetPackageTags_);
					throw std::runtime_error ("Query execution failed");
				}

				QStringList result;

				while (QueryGetPackageTags_.next ())
					result << QueryGetPackageTags_.value (0).toString ();

				QueryGetPackageTags_.finish ();

				return result;
			}

			bool Storage::HasLocation (int packageId, int componentId)
			{
				QueryHasLocation_.bindValue (":package_id", packageId);
				QueryHasLocation_.bindValue (":component_id", componentId);
				if (!QueryHasLocation_.exec ())
				{
					Util::DBLock::DumpError (QueryHasLocation_);
					throw std::runtime_error ("Query execution failed");
				}

				if (!QueryHasLocation_.next ())
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to position on first record.";
					throw std::runtime_error ("Unable to position on first record.");
				}

				bool result = QueryHasLocation_.value (0).toInt () != 0;
				QueryHasLocation_.finish ();
				return result;
			}

			void Storage::AddLocation (int packageId, int componentId)
			{
				QueryAddLocation_.bindValue (":package_id", packageId);
				QueryAddLocation_.bindValue (":component_id", componentId);
				if (!QueryAddLocation_.exec ())
				{
					Util::DBLock::DumpError (QueryAddLocation_);
					throw std::runtime_error ("Query execution failed");
				}

				QueryAddLocation_.finish ();
			}

			void Storage::InitTables ()
			{
				QSqlQuery query (DB_);
				QStringList names;
				names << "packages"
						<< "deps"
						<< "infos"
						<< "locations"
						<< "images"
						<< "tags"
						<< "repos"
						<< "components"
						<< "installed";
				Q_FOREACH (const QString& name, names)
					if (!DB_.tables ().contains (name))
						if (!query.exec (LoadQuery (QString ("create_table_%1").arg (name))))
						{
							Util::DBLock::DumpError (query);
							throw std::runtime_error ("Query execution failed.");
						}
			}

			void Storage::InitQueries ()
			{
				QueryCountPackages_ = QSqlQuery (DB_);
				QueryCountPackages_.prepare ("SELECT COUNT (package_id) "
						"FROM locations WHERE repo_url = :repo_url;");

				QueryFindRepo_ = QSqlQuery (DB_);
				QueryFindRepo_.prepare ("SELECT repo_id "
						"FROM repos WHERE url = :repo_url");

				QueryAddRepo_ = QSqlQuery (DB_);
				QueryAddRepo_.prepare (LoadQuery ("insert_repo"));

				QueryGetRepo_ = QSqlQuery (DB_);
				QueryGetRepo_.prepare ("SELECT url, name, description, "
						"longdescr, maint_name, maint_email FROM repos WHERE repo_id = :repo_id;");

				QueryAddRepoComponent_ = QSqlQuery (DB_);
				QueryAddRepoComponent_.prepare ("INSERT INTO components (repo_id, component) "
						"VALUES (:repo_id, :component);");

				QueryGetRepoComponents_ = QSqlQuery (DB_);
				QueryGetRepoComponents_.prepare ("SELECT component "
						"FROM components WHERE repo_id = :repo_id;");

				QueryFindComponent_ = QSqlQuery (DB_);
				QueryFindComponent_.prepare ("SELECT component_id "
						"FROM components WHERE repo_id = :repo_id AND component = :component;");

				QueryFindPackage_ = QSqlQuery (DB_);
				QueryFindPackage_.prepare ("SELECT package_id "
						"FROM packages WHERE name = :name AND version = :version;");

				QueryAddPackage_ = QSqlQuery (DB_);
				QueryAddPackage_.prepare ("INSERT INTO packages (name, version) "
						"VALUES (:name, :version);");

				QueryGetPackage_ = QSqlQuery (DB_);
				QueryGetPackage_.prepare ("SELECT name, version FROM packages WHERE package_id = :package_id;");

				QueryRemovePackage_ = QSqlQuery (DB_);
				QueryRemovePackage_.prepare ("DELETE FROM packages WHERE package_id = :package_id;");

				QueryHasLocation_ = QSqlQuery (DB_);
				QueryHasLocation_.prepare ("SELECT COUNT (package_id) "
						"FROM locations WHERE package_id = :package_id AND component_id = :component_id;");

				QueryAddLocation_ = QSqlQuery (DB_);
				QueryAddLocation_.prepare ("INSERT INTO locations (package_id, component_id) "
						"VALUES (:package_id, :component_id);");

				QueryRemovePackageFromLocations_ = QSqlQuery (DB_);
				QueryRemovePackageFromLocations_.prepare ("DELETE FROM locations WHERE package_id = :package_id;");

				QueryClearTags_ = QSqlQuery (DB_);
				QueryClearTags_.prepare ("DELETE FROM tags WHERE name = :name;");

				QueryAddTag_ = QSqlQuery (DB_);
				QueryAddTag_.prepare ("INSERT INTO tags (name, tag) VALUES (:name, :tag);");

				QueryAddPackageInfo_ = QSqlQuery (DB_);
				QueryAddPackageInfo_.prepare ("INSERT OR REPLACE INTO infos "
						"(name, short_descr, long_descr, type, language, maint_name, maint_email, icon_url) "
						"VALUES "
						"(:name, :short_descr, :long_descr, :type, :language, :maint_name, :maint_email, :icon_url);");

				QueryClearPackageInfos_ = QSqlQuery (DB_);
				QueryClearPackageInfos_.prepare ("DELETE FROM infos WHERE name = :name;");

				QueryClearImages_ = QSqlQuery (DB_);
				QueryClearImages_.prepare ("DELETE FROM images WHERE name = :name;");

				QueryAddImage_ = QSqlQuery (DB_);
				QueryAddImage_.prepare ("INSERT INTO images (name, url, type) "
						"VALUES (:name, :url, :type);");

				QueryClearDeps_ = QSqlQuery (DB_);
				QueryClearDeps_.prepare ("DELETE FROM deps WHERE package_id = :package_id;");

				QueryAddDep_ = QSqlQuery (DB_);
				QueryAddDep_.prepare ("INSERT INTO deps (package_id, name, version, type) "
						"VALUES (:package_id, :name, :version, :type);");

				QueryGetPackagesInComponent_ = QSqlQuery (DB_);
				QueryGetPackagesInComponent_.prepare ("SELECT package_id FROM locations WHERE component_id = :component_id;");

				QueryGetListPackageInfos_ = QSqlQuery (DB_);
				QueryGetListPackageInfos_.prepare ("SELECT DISTINCT packages.package_id, packages.name, packages.version, "
						"infos.short_descr, infos.long_descr, infos.type, infos.icon_url FROM packages, infos WHERE packages.name = infos.name;");

				QueryGetSingleListPackageInfo_ = QSqlQuery (DB_);
				QueryGetSingleListPackageInfo_.prepare ("SELECT DISTINCT packages.package_id, packages.name, packages.version, "
						"infos.short_descr, infos.long_descr, infos.type, infos.icon_url FROM packages, infos WHERE packages.name = infos.name AND packages.package_id = :package_id;");

				QueryGetPackageTags_ = QSqlQuery (DB_);
				QueryGetPackageTags_.prepare ("SELECT tag FROM tags, packages WHERE tags.name = packages.name AND package_id = :package_id;");

				QueryGetInstalledPackages_ = QSqlQuery (DB_);
				QueryGetInstalledPackages_.prepare ("SELECT name, version FROM installed;");

				QueryGetDependencies_ = QSqlQuery (DB_);
				QueryGetDependencies_.prepare ("SELECT name, version, type FROM deps WHERE package_id = :package_id;");

				QueryGetFulfillerCandidates_ = QSqlQuery (DB_);
				QueryGetFulfillerCandidates_.prepare ("SELECT package_id, version FROM deps WHERE name = :name;");
			}
		}
	}
}
