/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_STORAGE_H
#define PLUGINS_LACKMAN_STORAGE_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "repoinfo.h"

class QUrl;

namespace LC
{
namespace LackMan
{
	class RepoInfo;
	struct PackageInfo;

	class Storage : public QObject
	{
		Q_OBJECT

		QSqlDatabase DB_;

		QSqlQuery QueryCountPackages_;
		QSqlQuery QueryFindRepo_;
		QSqlQuery QueryAddRepo_;
		QSqlQuery QueryGetRepo_;
		QSqlQuery QueryRemoveRepo_;
		QSqlQuery QueryAddRepoComponent_;
		QSqlQuery QueryGetRepoComponents_;
		QSqlQuery QueryFindComponent_;
		QSqlQuery QueryFindPackage_;
		QSqlQuery QueryGetPackageVersions_;
		QSqlQuery QueryFindInstalledPackage_;
		QSqlQuery QueryAddPackage_;
		QSqlQuery QueryGetPackage_;
		QSqlQuery QueryRemovePackage_;
		QSqlQuery QueryAddPackageSize_;
		QSqlQuery QueryGetPackageSize_;
		QSqlQuery QueryRemovePackageSize_;
		QSqlQuery QueryAddPackageArchiver_;
		QSqlQuery QueryGetPackageArchiver_;
		QSqlQuery QueryRemovePackageArchiver_;
		QSqlQuery QueryHasLocation_;
		QSqlQuery QueryAddLocation_;
		QSqlQuery QueryRemovePackageFromLocation_;
		QSqlQuery QueryClearTags_;
		QSqlQuery QueryAddTag_;
		QSqlQuery QueryClearPackageInfos_;
		QSqlQuery QueryAddPackageInfo_;
		QSqlQuery QueryClearImages_;
		QSqlQuery QueryAddImage_;
		QSqlQuery QueryClearDeps_;
		QSqlQuery QueryAddDep_;
		QSqlQuery QueryGetPackagesInComponent_;
		QSqlQuery QueryGetListPackageInfos_;
		QSqlQuery QueryGetSingleListPackageInfo_;
		QSqlQuery QueryGetPackageTags_;
		QSqlQuery QueryGetInstalledPackages_;
		QSqlQuery QueryGetImages_;
		QSqlQuery QueryGetDependencies_;
		QSqlQuery QueryGetFulfillerCandidates_;
		QSqlQuery QueryGetPackageLocations_;
		QSqlQuery QueryAddToInstalled_;
		QSqlQuery QueryRemoveFromInstalled_;
	public:
		Storage (QObject* = 0);

		int CountPackages (const QUrl& repoUrl);

		QSet<int> GetInstalledPackagesIDs ();
		InstalledDependencyInfoList GetInstalledPackages ();

		int FindRepo (const QUrl& repoUrl);
		int AddRepo (const RepoInfo& ri);
		void RemoveRepo (int);
		RepoInfo GetRepo (int);

		QStringList GetComponents (int repoId);
		int FindComponent (int repoId, const QString& component);
		int AddComponent (int repoId, const QString& component, bool = true);
		void RemoveComponent (int repoId, const QString& component);

		int FindPackage (const QString& name, const QString& version);
		QStringList GetPackageVersions (const QString& name);

		int FindInstalledPackage (int packageId);
		PackageShortInfo GetPackage (int packageId);
		qint64 GetPackageSize (int packageId);
		void RemovePackage (int packageId);
		void AddPackages (const PackageInfo&);

		QMap<int, QList<QString>> GetPackageLocations (int);
		QList<int> GetPackagesInComponent (int);
		QMap<QString, QList<ListPackageInfo>> GetListPackageInfos ();
		QList<Image> GetImages (const QString&);
		ListPackageInfo GetSingleListPackageInfo (int);
		DependencyList GetDependencies (int);
		QList<ListPackageInfo> GetFulfillers (const Dependency&);

		QStringList GetPackageTags (int);
		QStringList GetAllTags ();

		bool HasLocation (int packageId, int componentId);
		void AddLocation (int packageId, int componentId);
		void RemoveLocation (int packageId, int componentId);

		void AddToInstalled (int);
		void RemoveFromInstalled (int);
	private:
		void InitTables ();
		void InitQueries ();
	signals:
		void packageRemoved (int);
	};
}
}

#endif
