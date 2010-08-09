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

#ifndef PLUGINS_LACKMAN_STORAGE_H
#define PLUGINS_LACKMAN_STORAGE_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "repoinfo.h"

class QUrl;

namespace LeechCraft
{
	namespace Plugins
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
				QSqlQuery QueryAddRepoComponent_;
				QSqlQuery QueryGetRepoComponents_;
				QSqlQuery QueryFindComponent_;
				QSqlQuery QueryFindPackage_;
				QSqlQuery QueryFindInstalledPackage_;
				QSqlQuery QueryAddPackage_;
				QSqlQuery QueryGetPackage_;
				QSqlQuery QueryRemovePackage_;
				QSqlQuery QueryHasLocation_;
				QSqlQuery QueryAddLocation_;
				QSqlQuery QueryRemovePackageFromLocations_;
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
				QSqlQuery QueryGetDependencies_;
				QSqlQuery QueryGetFulfillerCandidates_;
				QSqlQuery QueryGetPackageLocations_;
				QSqlQuery QueryAddToInstalled_;
				QSqlQuery QueryRemoveFromInstalled_;
			public:
				Storage (QObject* = 0);

				int CountPackages (const QUrl& repoUrl);

				InstalledDependencyInfoList GetInstalledPackages ();

				int FindRepo (const QUrl& repoUrl);
				int AddRepo (const RepoInfo& ri);
				RepoInfo GetRepo (int);

				QStringList GetComponents (int repoId);
				int FindComponent (int repoId, const QString& component);
				int AddComponent (int repoId, const QString& component, bool = true);
				void RemoveComponent (int repoId, const QString& component);

				int FindPackage (const QString& name, const QString& version);
				int FindInstalledPackage (int packageId);
				PackageShortInfo GetPackage (int packageId);
				void RemovePackage (int packageId);
				void AddPackages (const PackageInfo&);

				QMap<int, QList<QString> > GetPackageLocations (int);
				QList<int> GetPackagesInComponent (int);
				QMap<QString, QList<ListPackageInfo> > GetListPackageInfos ();
				ListPackageInfo GetSingleListPackageInfo (int);
				DependencyList GetDependencies (int);
				QList<ListPackageInfo> GetFulfillers (const Dependency&);

				QStringList GetPackageTags (int);
				QStringList GetAllTags ();

				bool HasLocation (int packageId, int componentId);
				void AddLocation (int packageId, int componentId);

				void AddToInstalled (int);
				void RemoveFromInstalled (int);
			private:
				void InitTables ();
				void InitQueries ();
			};
		};
	};
};

#endif
