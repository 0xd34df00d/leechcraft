/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storage.h"
#include <stdexcept>
#include <QDir>
#include <QSqlError>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/sll/containerconversions.h>
#include <util/sys/paths.h>
#include "repoinfo.h"
#include "core.h"

namespace LC
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
		DB_.setDatabaseName (Util::CreateIfNotExists ("lackman").filePath ("availablepackages.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO;
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
						.arg (DB_.lastError ().text ())));
		}

		QSqlQuery query (DB_);
		query.exec ("PRAGMA foreign_keys = ON;");
		query.exec ("PRAGMA synchronous = OFF;");

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

	QSet<int> Storage::GetInstalledPackagesIDs ()
	{
		if (!QueryGetInstalledPackages_.exec ())
		{
			Util::DBLock::DumpError (QueryGetInstalledPackages_);
			throw std::runtime_error ("Query execution failed.");
		}

		QSet<int> result;
		while (QueryGetInstalledPackages_.next ())
			result << QueryGetInstalledPackages_.value (0).toInt ();
		return result;
	}

	InstalledDependencyInfoList Storage::GetInstalledPackages ()
	{
		InstalledDependencyInfoList result;

		for (int id : GetInstalledPackagesIDs ())
		{
			PackageShortInfo psi;
			try
			{
				psi = GetPackage (id);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to get installed package info"
						<< id
						<< e.what ();
				continue;
			}

			Dependency dep =
			{
				Dependency::TProvides,
				psi.Name_,
				psi.Versions_.at (0)
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
					<< "could not acquire DB lock"
					<< e.what ();
			throw;
		}

		QueryAddRepo_.bindValue (":url", Slashize (ri.URL_).toEncoded ());
		QueryAddRepo_.bindValue (":name", ri.Name_);
		QueryAddRepo_.bindValue (":description", ri.ShortDescr_);
		QueryAddRepo_.bindValue (":longdescr", ri.LongDescr_);
		QueryAddRepo_.bindValue (":maint_name", ri.Maintainer_.Name_);
		QueryAddRepo_.bindValue (":maint_email", ri.Maintainer_.Email_);
		if (!QueryAddRepo_.exec ())
		{
			Util::DBLock::DumpError (QueryAddRepo_);
			throw std::runtime_error ("Query execution failed.");
		}

		QueryAddRepo_.finish ();

		int repoId = FindRepo (Slashize (ri.URL_));
		if (repoId == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< "OH SHI~, just inserted repo cannot be found!";
			throw std::runtime_error ("Just inserted repo cannot be found.");
		}

		for (const auto& component : ri.Components_)
			AddComponent (repoId, component);

		lock.Good ();

		return repoId;
	}

	void Storage::RemoveRepo (int repoId)
	{
		QStringList components = GetComponents (repoId);
		for (const auto& component : components)
			RemoveComponent (repoId, component);

		QueryRemoveRepo_.bindValue (":repo_id", repoId);
		if (!QueryRemoveRepo_.exec ())
		{
			Util::DBLock::DumpError (QueryRemoveRepo_);
			throw std::runtime_error ("Query execution failed");
		}
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

		result.Name_ = QueryGetRepo_.value (1).toString ();
		result.ShortDescr_ = QueryGetRepo_.value (2).toString ();
		result.LongDescr_ = QueryGetRepo_.value (3).toString ();
		result.Maintainer_.Name_ = QueryGetRepo_.value (4).toString ();
		result.Maintainer_.Email_ = QueryGetRepo_.value (5).toString ();

		QueryGetRepo_.finish ();

		result.Components_ = GetComponents (repoId);

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
					<< "unable to start transaction"
					<< e.what ();
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

		const auto& packs = Util::AsSet (GetPackagesInComponent (compId));
		const auto& toRemove = packs - GetInstalledPackagesIDs ();

		QSqlQuery remover (DB_);
		remover.prepare ("DELETE FROM components WHERE component_id = :component_id;");
		remover.bindValue (":component_id", compId);
		if (!remover.exec ())
		{
			Util::DBLock::DumpError (remover);
			throw std::runtime_error ("Unable to remove component from components.");
		}
		remover.finish ();

		for (int packageId : toRemove)
		{
			emit packageRemoved (packageId);
			RemovePackage (packageId);
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

	QStringList Storage::GetPackageVersions (const QString& name)
	{
		QueryGetPackageVersions_.bindValue (":name", name);
		if (!QueryGetPackageVersions_.exec ())
		{
			Util::DBLock::DumpError (QueryGetPackageVersions_);
			throw std::runtime_error ("Query execution failed");
		}

		QStringList result;
		while (QueryGetPackageVersions_.next ())
			result << QueryGetPackageVersions_.value (0).toString ();

		QueryGetPackageVersions_.finish ();

		return result;
	}

	int Storage::FindInstalledPackage (int packageId)
	{
		QueryFindInstalledPackage_.bindValue (":package_id", packageId);
		if (!QueryFindInstalledPackage_.exec ())
		{
			Util::DBLock::DumpError (QueryFindInstalledPackage_);
			throw std::runtime_error ("Query execution failed");
		}

		int result = -1;
		if (QueryFindInstalledPackage_.next ())
			result = QueryFindInstalledPackage_.value (0).toInt ();

		QueryFindInstalledPackage_.finish ();

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

		const auto& version = QueryGetPackage_.value (1).toString ();
		PackageShortInfo info =
		{
			QueryGetPackage_.value (0).toString (),
			QStringList (version),
			QMap<QString, QString> ()
		};
		QueryGetPackage_.finish ();

		QueryGetPackageArchiver_.bindValue (":package_id", packageId);
		if (!QueryGetPackageArchiver_.exec ())
		{
			Util::DBLock::DumpError (QueryGetPackageArchiver_);
			throw std::runtime_error ("archiver query execution failed");
		}
		info.VersionArchivers_ [version] = QueryGetPackageArchiver_.next () ?
				QueryGetPackageArchiver_.value (0).toString () :
				"gz";

		return info;
	}

	qint64 Storage::GetPackageSize (int packageId)
	{
		QueryGetPackageSize_.bindValue (":package_id", packageId);
		if (!QueryGetPackageSize_.exec ())
		{
			Util::DBLock::DumpError (QueryGetPackageSize_);
			throw std::runtime_error ("Query execution failed");
		}

		if (!QueryGetPackageSize_.next ())
			return -1;

		const qint64 result = QueryGetPackageSize_.value (0).toLongLong ();
		QueryGetPackageSize_.finish ();

		return result;
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

		const auto& name = GetPackage (packageId).Name_;

		QueryRemovePackage_.bindValue (":package_id", packageId);
		if (!QueryRemovePackage_.exec ())
		{
			Util::DBLock::DumpError (QueryRemovePackage_);
			throw std::runtime_error ("Query execution failed");
		}

		QueryRemovePackageSize_.bindValue (":package_id", packageId);
		if (!QueryRemovePackageSize_.exec ())
		{
			Util::DBLock::DumpError (QueryRemovePackageSize_);
			throw std::runtime_error ("Query execution failed");
		}

		QueryRemovePackageArchiver_.bindValue (":package_id", packageId);
		if (!QueryRemovePackageArchiver_.exec ())
		{
			Util::DBLock::DumpError (QueryRemovePackageArchiver_);
			throw std::runtime_error ("Query execution failed");
		}

		QSqlQuery others (DB_);
		others.prepare ("SELECT COUNT(1) FROM packages WHERE name = :name;");
		others.bindValue (":name", name);
		if (!others.exec ())
		{
			Util::DBLock::DumpError (others);
			throw std::runtime_error ("Query execution failed");
		}

		if (!others.next () || !others.value (0).toInt ())
		{
			qDebug () << Q_FUNC_INFO
					<< "no other packages"
					<< name;

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
		}

		others.finish ();

		lock.Good ();
	}

	namespace
	{
		void Exec (QSqlQuery& query)
		{
			if (!query.exec ())
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("Query execution failed");
			}
		}
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

		for (const auto& version : pInfo.Versions_)
		{
			if (FindPackage (pInfo.Name_, version) != -1)
			{
				qWarning () << Q_FUNC_INFO
						<< "package"
						<< pInfo.Name_
						<< version
						<< "is already present, skipping...";
				continue;
			}

			QueryAddPackage_.bindValue (":name", pInfo.Name_);
			QueryAddPackage_.bindValue (":version", version);
			Exec (QueryAddPackage_);

			const int packageId = FindPackage (pInfo.Name_, version);
			qDebug () << Q_FUNC_INFO << pInfo.Name_ << version << packageId;

			QueryAddPackageArchiver_.bindValue (":package_id", packageId);
			QueryAddPackageArchiver_.bindValue (":archiver",
					pInfo.VersionArchivers_.value (version, "gz"));
			Exec (QueryAddPackageArchiver_);

			const qint64 size = pInfo.PackageSizes_.value (version, -1);
			if (size == -1)
				continue;

			QueryAddPackageSize_.bindValue (":package_id", packageId);
			QueryAddPackageSize_.bindValue (":size", size);
			Exec (QueryAddPackageSize_);
		}
		QueryAddPackage_.finish ();
		QueryAddPackageSize_.finish ();
		QueryAddPackageArchiver_.finish ();

		QueryClearPackageInfos_.bindValue (":name", pInfo.Name_);
		Exec (QueryClearPackageInfos_);
		QueryClearPackageInfos_.finish ();

		QueryAddPackageInfo_.bindValue (":name", pInfo.Name_);
		QueryAddPackageInfo_.bindValue (":short_descr", pInfo.Description_);
		QueryAddPackageInfo_.bindValue (":long_descr", pInfo.LongDescription_);
		QueryAddPackageInfo_.bindValue (":type", pInfo.Type_);
		QueryAddPackageInfo_.bindValue (":language", pInfo.Language_);
		QueryAddPackageInfo_.bindValue (":maint_name", pInfo.MaintName_);
		QueryAddPackageInfo_.bindValue (":maint_email", pInfo.MaintEmail_);
		QueryAddPackageInfo_.bindValue (":icon_url", pInfo.IconURL_);
		Exec (QueryAddPackageInfo_);
		QueryAddPackageInfo_.finish ();

		QueryClearTags_.bindValue (":name", pInfo.Name_);
		Exec (QueryClearTags_);
		QueryClearTags_.finish ();

		for (const auto& tag : pInfo.Tags_)
		{
			QueryAddTag_.bindValue (":name", pInfo.Name_);
			QueryAddTag_.bindValue (":tag", tag);
			Exec (QueryAddTag_);
		}
		QueryAddTag_.finish ();

		QueryClearImages_.bindValue (":name", pInfo.Name_);
		Exec (QueryClearImages_);
		QueryClearImages_.finish ();

		for (const auto& img : pInfo.Images_)
		{
			QueryAddImage_.bindValue (":name", pInfo.Name_);
			QueryAddImage_.bindValue (":url", img.URL_);
			QueryAddImage_.bindValue (":type", img.Type_);
			Exec (QueryAddImage_);
		}
		QueryAddImage_.finish ();

		for (const auto& thisVersion : pInfo.Deps_.keys ())
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
			Exec (QueryClearDeps_);
			QueryClearDeps_.finish ();

			for (const auto& dep : pInfo.Deps_ [thisVersion])
			{
				QueryAddDep_.bindValue (":package_id", packageId);
				QueryAddDep_.bindValue (":name", dep.Name_);
				QueryAddDep_.bindValue (":version", dep.Version_);
				QueryAddDep_.bindValue (":type", dep.Type_);
				Exec (QueryAddDep_);
			}
			QueryAddDep_.finish ();
		}

		lock.Good ();
	}

	QMap<int, QList<QString>> Storage::GetPackageLocations (int packageId)
	{
		QueryGetPackageLocations_.bindValue (":package_id", packageId);
		if (!QueryGetPackageLocations_.exec ())
		{
			Util::DBLock::DumpError (QueryGetPackageLocations_);
			throw std::runtime_error ("Query execution failed");
		}

		QMap<int, QList<QString>> result;
		while (QueryGetPackageLocations_.next ())
		{
			int repoId = QueryGetPackageLocations_.value (0).toInt ();
			QString component = QueryGetPackageLocations_.value (1).toString ();

			result [repoId] << component;
		}

		QueryGetPackageLocations_.finish ();

		return result;
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

	QMap<QString, QList<ListPackageInfo>> Storage::GetListPackageInfos ()
	{
		if (!QueryGetListPackageInfos_.exec ())
		{
			Util::DBLock::DumpError (QueryGetListPackageInfos_);
			throw std::runtime_error ("Query execution failed");
		}

		QMap<QString, QList<ListPackageInfo>> result;
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
				QueryGetListPackageInfos_.value (6).toString (),
				QUrl::fromEncoded (QueryGetListPackageInfos_.value (7).toString ().toUtf8 ()),
				GetPackageTags (packageId),
				false,
				false
			};

			result [name] << info;
		}

		QueryGetListPackageInfos_.finish ();

		return result;
	}

	QList<Image> Storage::GetImages (const QString& name)
	{
		QueryGetImages_.bindValue (":name", name);
		if (!QueryGetImages_.exec ())
		{
			Util::DBLock::DumpError (QueryGetImages_);
			return QList<Image> ();
		}

		QList<Image> result;
		while (QueryGetImages_.next ())
		{
			Image img =
			{
				static_cast<Image::Type> (QueryGetImages_.value (1).toInt ()),
				QueryGetImages_.value (0).toString ()
			};
			result << img;
		}
		QueryGetImages_.finish ();
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
			QueryGetSingleListPackageInfo_.value (6).toString (),
			QUrl::fromEncoded (QueryGetSingleListPackageInfo_.value (7).toString ().toUtf8 ()),
			GetPackageTags (packageId),
			false,
			false
		};

		QSqlQuery query (DB_);
		query.prepare ("SELECT COUNT (installed.package_id) FROM installed, packages "
				"WHERE installed.package_id = packages.package_id AND packages.name = :name;");
		query.bindValue (":name", name);
		if (!query.exec ())
		{
			Util::DBLock::DumpError (query);
			qWarning () << Q_FUNC_INFO
					<< "unable to get installed status";
			throw std::runtime_error ("Query execution failed");
		}

		if (!query.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to navigate to next record in installed status";
			throw std::runtime_error ("Unexpected query behavior");
		}

		info.IsInstalled_ = query.value (0).toInt ();

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
			int typeInt = QueryGetDependencies_.value (2).toInt ();
			Dependency::Type type;
			if (typeInt < Dependency::TMAX)
				type = static_cast<Dependency::Type> (typeInt);
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown type"
						<< typeInt;
				QString err = tr ("Unknown dependency type `%1`.")
						.arg (typeInt);
				throw std::runtime_error (qPrintable (err));
			}

			Dependency dep =
			{
				type,
				QueryGetDependencies_.value (0).toString (),
				QueryGetDependencies_.value (1).toString ()
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

	QStringList Storage::GetAllTags ()
	{
		QSqlQuery query ("SELECT DISTINCT tag FROM tags;", DB_);
		if (!query.exec ())
		{
			Util::DBLock::DumpError (query);
			throw std::runtime_error ("Query execution failed");
		}

		QStringList result;

		while (query.next ())
			result << query.value (0).toString ();

		query.finish ();

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

	void Storage::RemoveLocation (int packageId, int componentId)
	{
		QueryRemovePackageFromLocation_.bindValue (":package_id", packageId);
		QueryRemovePackageFromLocation_.bindValue (":component_id", componentId);
		if (!QueryRemovePackageFromLocation_.exec ())
		{
			Util::DBLock::DumpError (QueryRemovePackageFromLocation_);
			throw std::runtime_error ("Query execution failed");
		}

		QueryRemovePackageFromLocation_.finish ();
	}

	void Storage::AddToInstalled (int packageId)
	{
		QueryAddToInstalled_.bindValue (":package_id", packageId);
		if (!QueryAddToInstalled_.exec ())
		{
			Util::DBLock::DumpError (QueryAddToInstalled_);
			throw std::runtime_error ("Query execution failed");
		}

		QueryAddToInstalled_.finish ();
	}

	void Storage::RemoveFromInstalled (int packageId)
	{
		QueryRemoveFromInstalled_.bindValue (":package_id", packageId);
		if (!QueryRemoveFromInstalled_.exec ())
		{
			Util::DBLock::DumpError (QueryRemoveFromInstalled_);
			throw std::runtime_error ("Query execution failed");
		}

		QueryRemoveFromInstalled_.finish ();

		if (GetPackageLocations (packageId).isEmpty ())
		{
			emit packageRemoved (packageId);
			RemovePackage (packageId);
		}
	}

	void Storage::InitTables ()
	{
		QSqlQuery query (DB_);
		const QStringList names
		{
			"packages",
			"packagesizes",
			"packagearchivers",
			"deps",
			"infos",
			"locations",
			"images",
			"tags",
			"repos",
			"components",
			"installed"
		};
		for (const auto& name : names)
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

		QueryRemoveRepo_ = QSqlQuery (DB_);
		QueryRemoveRepo_.prepare ("DELETE FROM repos WHERE repo_id = :repo_id;");

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

		QueryGetPackageVersions_ = QSqlQuery (DB_);
		QueryGetPackageVersions_.prepare ("SELECT version "
				"FROM packages WHERE name = :name;");

		QueryFindInstalledPackage_ = QSqlQuery (DB_);
		QueryFindInstalledPackage_.prepare ("SELECT installed.package_id FROM installed, packages, packages AS tmp "
				"WHERE installed.package_id = packages.package_id "
				"AND packages.name = tmp.name AND tmp.package_id = :package_id;");

		QueryAddPackage_ = QSqlQuery (DB_);
		QueryAddPackage_.prepare ("INSERT INTO packages (name, version) "
				"VALUES (:name, :version);");

		QueryGetPackage_ = QSqlQuery (DB_);
		QueryGetPackage_.prepare ("SELECT name, version FROM packages WHERE package_id = :package_id;");

		QueryRemovePackage_ = QSqlQuery (DB_);
		QueryRemovePackage_.prepare ("DELETE FROM packages WHERE package_id = :package_id;");

		QueryAddPackageSize_ = QSqlQuery (DB_);
		QueryAddPackageSize_.prepare ("INSERT INTO packagesizes (package_id, size) "
				"VALUES (:package_id, :size);");

		QueryGetPackageSize_ = QSqlQuery (DB_);
		QueryGetPackageSize_.prepare ("SELECT size FROM packagesizes WHERE package_id = :package_id;");

		QueryRemovePackageSize_ = QSqlQuery (DB_);
		QueryRemovePackageSize_.prepare ("DELETE from packagesizes WHERE package_id = :package_id;");

		QueryAddPackageArchiver_ = QSqlQuery (DB_);
		QueryAddPackageArchiver_.prepare ("INSERT INTO packagearchivers (package_id, archiver) "
				"VALUES (:package_id, :archiver);");

		QueryGetPackageArchiver_ = QSqlQuery (DB_);
		QueryGetPackageArchiver_.prepare ("SELECT archiver FROM packagearchivers WHERE package_id = :package_id;");

		QueryRemovePackageArchiver_ = QSqlQuery (DB_);
		QueryRemovePackageArchiver_.prepare ("DELETE FROM packagearchivers WHERE package_id = :package_id;");

		QueryHasLocation_ = QSqlQuery (DB_);
		QueryHasLocation_.prepare ("SELECT COUNT (package_id) "
				"FROM locations WHERE package_id = :package_id AND component_id = :component_id;");

		QueryAddLocation_ = QSqlQuery (DB_);
		QueryAddLocation_.prepare ("INSERT INTO locations (package_id, component_id) "
				"VALUES (:package_id, :component_id);");

		QueryRemovePackageFromLocation_ = QSqlQuery (DB_);
		QueryRemovePackageFromLocation_.prepare ("DELETE FROM locations WHERE package_id = :package_id AND component_id = :component_id;");

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
		QueryGetPackagesInComponent_.prepare ("SELECT DISTINCT package_id FROM locations WHERE component_id = :component_id;");

		QueryGetListPackageInfos_ = QSqlQuery (DB_);
		QueryGetListPackageInfos_.prepare ("SELECT DISTINCT packages.package_id, packages.name, packages.version, "
				"infos.short_descr, infos.long_descr, infos.type, infos.language, infos.icon_url FROM packages, infos "
				"WHERE packages.name = infos.name;");

		QueryGetSingleListPackageInfo_ = QSqlQuery (DB_);
		QueryGetSingleListPackageInfo_.prepare ("SELECT DISTINCT packages.package_id, packages.name, packages.version, "
				"infos.short_descr, infos.long_descr, infos.type, infos.language, infos.icon_url FROM packages, infos "
				"WHERE packages.name = infos.name AND packages.package_id = :package_id;");

		QueryGetPackageTags_ = QSqlQuery (DB_);
		QueryGetPackageTags_.prepare ("SELECT tag FROM tags, packages WHERE tags.name = packages.name AND package_id = :package_id;");

		QueryGetInstalledPackages_ = QSqlQuery (DB_);
		QueryGetInstalledPackages_.prepare ("SELECT package_id FROM installed;");

		QueryGetImages_ = QSqlQuery (DB_);
		QueryGetImages_.prepare ("SELECT url, type FROM images WHERE name = :name;");

		QueryGetDependencies_ = QSqlQuery (DB_);
		QueryGetDependencies_.prepare ("SELECT name, version, type FROM deps WHERE package_id = :package_id;");

		QueryGetFulfillerCandidates_ = QSqlQuery (DB_);
		QueryGetFulfillerCandidates_.prepare ("SELECT package_id, version FROM deps WHERE name = :name;");

		QueryGetPackageLocations_ = QSqlQuery (DB_);
		QueryGetPackageLocations_.prepare (LoadQuery ("select_package_locations"));

		QueryAddToInstalled_ = QSqlQuery (DB_);
		QueryAddToInstalled_.prepare (LoadQuery ("insert_installed"));

		QueryRemoveFromInstalled_ = QSqlQuery (DB_);
		QueryRemoveFromInstalled_.prepare ("DELETE FROM installed WHERE package_id = :package_id;");
	}
}
}
