/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_CORE_H
#define PLUGINS_LACKMAN_CORE_H
#include <QObject>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include "repoinfo.h"

class QAbstractItemModel;
class QStandardItemModel;
class QDir;

namespace LeechCraft
{
namespace LackMan
{
	class RepoInfoFetcher;
	class ExternalResourceManager;
	class Storage;
	class PackagesModel;
	class PendingManager;
	class PackageProcessor;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		RepoInfoFetcher *RepoInfoFetcher_;
		ExternalResourceManager *ExternalResourceManager_;
		Storage *Storage_;
		PackagesModel *PackagesModel_;
		PendingManager *PendingManager_;
		PackageProcessor *PackageProcessor_;
		QStandardItemModel *ReposModel_;

		enum ReposColumns
		{
			RCURL
		};

		Core ();
	public:
		static Core& Instance ();
		void FinishInitialization ();
		void Release ();

		void SecondInit ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		QAbstractItemModel* GetPluginsModel () const;
		PendingManager* GetPendingManager () const;
		ExternalResourceManager* GetExtResourceManager () const;
		Storage* GetStorage () const;
		QAbstractItemModel* GetRepositoryModel () const;

		DependencyList GetDependencies (int) const;
		QList<ListPackageInfo> GetDependencyFulfillers (const Dependency&) const;
		bool IsVersionOk (const QString& candidate, QString refVer) const;
		bool IsFulfilled (const Dependency&) const;
		QIcon GetIconForLPI (const ListPackageInfo&);
		QList<QUrl> GetPackageURLs (int) const;
		ListPackageInfo GetListPackageInfo (int);
		QDir GetPackageDir (int) const;

		void AddRepo (const QUrl&);
		void UpdateRepo (const QUrl&, const QStringList&);

		QString NormalizePackageName (const QString&) const;

		QStringList GetAllTags () const;
	private:
		InstalledDependencyInfoList GetSystemInstalledPackages () const;
		InstalledDependencyInfoList GetLackManInstalledPackages () const;
		InstalledDependencyInfoList GetAllInstalledPackages () const;
		void PopulatePluginsModel ();
		void HandleNewPackages (const PackageShortInfoList& shorts,
				int componentId, const QString& component, const QUrl& repoUrl);
		void PerformRemoval (int);
		void UpdateRowFor (int);
		bool RecordInstalled (int);
		bool RecordUninstalled (int);
		int GetPackageRow (int packageId) const;
		void ReadSettings ();
		void WriteSettings ();
	public slots:
		void updateAllRequested ();
		void timeredUpdateAllRequested ();
		void upgradeAllRequested ();
		void cancelPending ();
		void acceptPending ();
		void removeRequested (const QString&, const QModelIndexList&);
		void addRequested (const QString&, const QVariantList&);
	private slots:
		void handleInfoFetched (const RepoInfo&);
		void handleComponentFetched (const PackageShortInfoList&,
				const QString&, int);
		void handlePackageFetched (const PackageInfo&, int);
		void handlePackageInstallError (int, const QString&);
		void handlePackageInstalled (int);
		void handlePackageUpdated (int from, int to);
		void handlePackageRemoved (int);
	signals:
		void delegateEntity (const LeechCraft::Entity&,
				int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
		void tagsUpdated (const QStringList&);
		void packageRowActionFinished (int row);
	};
}
}

#endif
