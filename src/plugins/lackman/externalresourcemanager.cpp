/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "externalresourcemanager.h"
#include <stdexcept>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LC
{
namespace LackMan
{
	namespace
	{
		QString URLToFileName (const QUrl& url)
		{
			return QString (url.toEncoded ().toBase64 ().replace ('/', '_'));
		}
	}

	ExternalResourceManager::ExternalResourceManager (QObject *parent)
	: QObject (parent)
	, ResourcesDir_ (Util::CreateIfNotExists ("lackman/resources/"))
	{
	}

	void ExternalResourceManager::GetResourceData (const QUrl& url)
	{
		QString fileName = URLToFileName (url);

		if (ResourcesDir_.exists (fileName))
			ResourcesDir_.remove (fileName);

		if (PendingResources_.contains (url))
			return;

		QString location = ResourcesDir_.filePath (fileName);

		auto e = Util::MakeEntity (url,
				location,
				Internal |
					DoNotNotifyUser |
					DoNotSaveInHistory |
					NotPersistent |
					DoNotAnnounceEntity);

		auto delegateResult = Core::Instance ().GetProxy ()->GetEntityManager ()->DelegateEntity (e);
		if (!delegateResult)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find plugin for"
					<< url;
			return;
		}

		PendingResources_ << url;

		Util::Sequence (this, delegateResult.DownloadResult_) >>
				Util::Visitor
				{
					[=] (IDownload::Success) { emit resourceFetched (url); },
					[=] (const IDownload::Error& error)
					{
						qWarning () << Q_FUNC_INFO
								<< "failed to download"
								<< url
								<< error.Message_;
					}
				}.Finally ([=] { PendingResources_.remove (url); });
	}

	QString ExternalResourceManager::GetResourcePath (const QUrl& url) const
	{
		return ResourcesDir_.filePath (URLToFileName (url));
	}

	void ExternalResourceManager::ClearCaches ()
	{
		for (const auto& fname : ResourcesDir_.entryList ())
			ResourcesDir_.remove (fname);
	}

	void ExternalResourceManager::ClearCachedResource (const QUrl& url)
	{
		ResourcesDir_.remove (URLToFileName (url));
	}
}
}
