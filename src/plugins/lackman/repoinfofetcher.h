/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_REPOINFOFETCHER_H
#define PLUGINS_LACKMAN_REPOINFOFETCHER_H
#include <QObject>
#include <QUrl>
#include <QProcess>
#include <QHash>
#include <interfaces/idownload.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "repoinfo.h"

namespace LC
{
namespace LackMan
{
	class RepoInfoFetcher : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		struct ScheduledPackageFetch
		{
			QUrl BaseUrl_;
			QString PackageName_;
			QList<QString> NewVersions_;
			int ComponentId_;
		};
		QList<ScheduledPackageFetch> ScheduledPackages_;

	public:
		struct PendingPackage
		{
			QUrl URL_;
			QUrl BaseURL_;
			QString Location_;
			QString PackageName_;
			QList<QString> NewVersions_;
			int ComponentId_;
		};

		RepoInfoFetcher (const ICoreProxy_ptr& proxy, QObject*);

		void FetchFor (QUrl);
		void FetchComponent (QUrl, int, const QString& component);
		void ScheduleFetchPackageInfo (const QUrl& url,
				const QString& name,
				const QList<QString>& newVers,
				int componentId);
	private:
		void FetchPackageInfo (const QUrl& url,
				const QString& name,
				const QList<QString>& newVers,
				int componentId);

		void HandleRIFinished (const QString&, const QUrl&);
		void HandleComponentFinished (const QUrl&, const QString&, const QString&, int);
		void HandlePackageFinished (const PendingPackage&);
	private slots:
		void rotatePackageFetchQueue ();
	signals:
		void infoFetched (const RepoInfo&);
		void componentFetched (const PackageShortInfoList& packages,
				const QString& component, int repoId);
		void packageFetched (const PackageInfo&, int componentId);
	};
}
}

#endif
