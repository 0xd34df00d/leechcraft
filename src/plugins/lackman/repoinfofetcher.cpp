/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "repoinfofetcher.h"
#include <QTimer>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "xmlparsers.h"
#include "lackmanutil.h"

namespace LC
{
namespace LackMan
{
	RepoInfoFetcher::RepoInfoFetcher (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	{
	}

	namespace
	{
		template<typename SuccessF>
		void FetchImpl (const QUrl& url, const ICoreProxy_ptr& proxy, QObject *object,
				const QString& failureHeading, SuccessF&& successFun)
		{
			const auto& location = Util::GetTemporaryName ("lackman_XXXXXX.gz");

			const auto iem = proxy->GetEntityManager ();

			const auto& e = Util::MakeEntity (url,
					location,
					Internal |
						DoNotNotifyUser |
						DoNotSaveInHistory |
						NotPersistent |
						DoNotAnnounceEntity);
			const auto& result = iem->DelegateEntity (e);
			if (!result)
			{
				iem->HandleEntity (Util::MakeNotification (RepoInfoFetcher::tr ("Error fetching repository"),
						RepoInfoFetcher::tr ("Could not find any plugins to fetch %1.")
							.arg ("<em>" + url.toString () + "</em>"),
						Priority::Critical));
				return;
			}

			Util::Sequence (object, result.DownloadResult_) >>
					Util::Visitor
					{
						[successFun, location] (IDownload::Success) { successFun (location); },
						[proxy, url, failureHeading, location] (const IDownload::Error&)
						{
							proxy->GetEntityManager ()->HandleEntity (Util::MakeNotification (failureHeading,
									RepoInfoFetcher::tr ("Error downloading file from %1.")
											.arg (url.toString ()),
									Priority::Critical));
							QFile::remove (location);
						}
					};
		}
	}

	void RepoInfoFetcher::FetchFor (QUrl url)
	{
		QString path = url.path ();
		if (!path.endsWith ("/Repo.xml.gz"))
		{
			path.append ("/Repo.xml.gz");
			url.setPath (path);
		}

		QUrl baseUrl = url;
		baseUrl.setPath (baseUrl.path ().remove ("/Repo.xml.gz"));

		FetchImpl (url, Proxy_, this, tr ("Error fetching repository"),
				[this, baseUrl] (const QString& location) { HandleRIFinished (location, baseUrl); });
	}

	void RepoInfoFetcher::FetchComponent (QUrl url, int repoId, const QString& component)
	{
		if (!url.path ().endsWith ("/Packages.xml.gz"))
			url.setPath (url.path () + "/Packages.xml.gz");

		FetchImpl (url, Proxy_, this, tr ("Error fetching component"),
				[=, this] (const QString& location) { HandleComponentFinished (url, location, component, repoId); });
	}

	void RepoInfoFetcher::ScheduleFetchPackageInfo (const QUrl& url,
			const QString& name,
			const QList<QString>& newVers,
			int componentId)
	{
		ScheduledPackageFetch f =
		{
			url,
			name,
			newVers,
			componentId
		};

		if (ScheduledPackages_.isEmpty ())
			QTimer::singleShot (0,
					this,
					SLOT (rotatePackageFetchQueue ()));

		ScheduledPackages_ << f;
	}

	void RepoInfoFetcher::FetchPackageInfo (const QUrl& baseUrl,
			const QString& packageName,
			const QList<QString>& newVersions,
			int componentId)
	{
		auto packageUrl = baseUrl;
		packageUrl.setPath (packageUrl.path () +
				LackManUtil::NormalizePackageName (packageName) + ".xml.gz");

		FetchImpl (packageUrl, Proxy_, this, tr ("Error fetching package info"),
				[=, this] (const QString& location)
				{
					HandlePackageFinished ({ packageUrl, baseUrl, location, packageName, newVersions, componentId });
				});
	}

	void RepoInfoFetcher::rotatePackageFetchQueue ()
	{
		if (ScheduledPackages_.isEmpty ())
			return;

		const auto& f = ScheduledPackages_.takeFirst ();
		FetchPackageInfo (f.BaseUrl_, f.PackageName_, f.NewVersions_, f.ComponentId_);

		if (!ScheduledPackages_.isEmpty ())
			QTimer::singleShot (50, this, SLOT (rotatePackageFetchQueue ()));
	}

	namespace
	{
		void HandleUnarchError (QProcess *proc, IEntityManager *iem, const QUrl& url, const QString& filename)
		{
			proc->deleteLater ();

			auto error = proc->error ();

			qWarning () << Q_FUNC_INFO
					<< "unable to unpack for"
					<< url
					<< filename
					<< "with"
					<< error
					<< proc->readAllStandardError ();
			const auto& notification = Util::MakeNotification (RepoInfoFetcher::tr ("Component unpack error"),
					RepoInfoFetcher::tr ("Unable to unpack file. Exit code: %1. Problematic file is at %2.")
						.arg (error)
						.arg (filename),
					Priority::Critical);
			iem->HandleEntity (notification);
		}

		template<typename Handler>
		void HandleUnarch (QObject *parent,
				const ICoreProxy_ptr& proxy, const QUrl& url, const QString& location, Handler&& handler)
		{
			auto iem = proxy->GetEntityManager ();

			auto unarch = new QProcess { parent };
			QObject::connect (unarch,
					qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
					parent,
					[=] (int exitCode)
					{
						unarch->deleteLater ();

						if (exitCode)
						{
							iem->HandleEntity (Util::MakeNotification (RepoInfoFetcher::tr ("Repository unpack error"),
									RepoInfoFetcher::tr ("Unable to unpack the repository file. gunzip error: %1. "
										"Problematic file is at %2.")
											.arg (exitCode)
											.arg (location),
									Priority::Critical));
							return;
						}

						QFile::remove (location);

						std::invoke (handler, unarch->readAllStandardOutput ());
					});

			QObject::connect (unarch,
					&QProcess::errorOccurred,
					[=] { HandleUnarchError (unarch, iem, url, location); });

#ifdef Q_OS_WIN32
			unarch->start ("7za", { "e", "-so", location });
#else
			unarch->start ("gunzip", { "-c", location });
#endif
		}
	}

	void RepoInfoFetcher::HandleRIFinished (const QString& location, const QUrl& url)
	{
		HandleUnarch (this, Proxy_, url, location,
				[=, this] (const QByteArray& data)
				{
					try
					{
						emit infoFetched (ParseRepoInfo (url, { data }));
					}
					catch (const QString& error)
					{
						qWarning () << Q_FUNC_INFO
								<< error;
						Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (tr ("Repository parse error"),
								tr ("Unable to parse repository description: %1.")
										.arg (error),
								Priority::Critical));
					}
				});
	}

	void RepoInfoFetcher::HandleComponentFinished (const QUrl& url,
			const QString& location, const QString& component, int repoId)
	{
		HandleUnarch (this, Proxy_, url, location,
				[=, this] (const QByteArray& data)
				{
					try
					{
						emit componentFetched (ParseComponent (data), component, repoId);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< e.what ();
						Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (tr ("Component parse error"),
								tr ("Unable to parse component %1 description file.")
										.arg (component),
								Priority::Critical));
					}
				});
	}

	void RepoInfoFetcher::HandlePackageFinished (const PendingPackage& pp)
	{
		HandleUnarch (this, Proxy_, pp.URL_, pp.Location_,
				[=, this] (const QByteArray& data)
				{
					try
					{
						emit packageFetched (ParsePackage (data, pp.BaseURL_, pp.PackageName_, pp.NewVersions_), pp.ComponentId_);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< e.what ();
						Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (tr ("Package parse error"),
								tr ("Unable to parse package description file."),
								Priority::Critical));
					}
				});
	}
}
}
