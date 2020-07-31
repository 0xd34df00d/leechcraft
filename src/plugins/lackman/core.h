/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
{
namespace LackMan
{
	class RepoInfoFetcher;
	class ExternalResourceManager;
	class Storage;
	class PackagesModel;
	class PendingManager;
	class PackageProcessor;
	class UpdatesNotificationManager;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		RepoInfoFetcher *RepoInfoFetcher_ = nullptr;
		ExternalResourceManager *ExternalResourceManager_;
		Storage *Storage_;
		PackagesModel *PackagesModel_;
		PendingManager *PendingManager_;
		PackageProcessor *PackageProcessor_;
		QStandardItemModel *ReposModel_;
		UpdatesNotificationManager *UpdatesNotificationManager_ = nullptr;
		bool UpdatesEnabled_;

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
		UpdatesNotificationManager* GetUpdatesNotificationManager () const;

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

		QStringList GetAllTags () const;
	private:
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
		void handleUpdatesIntervalChanged ();
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
		void gotEntity (const LC::Entity&);
		void tagsUpdated (const QStringList&);
		void packageRowActionFinished (int row);

		void openLackmanRequested ();
	};
}
}

#endif
