/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "externalresourcemanager.h"
#include <stdexcept>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/sys/paths.h>

namespace LeechCraft
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

		Q_FOREACH (const PendingResource& pr, PendingResources_.values ())
			if (pr.URL_ == url)
				return;

		QString location = ResourcesDir_.filePath (fileName);

		Entity e = Util::MakeEntity (url,
				location,
				Internal |
					DoNotNotifyUser |
					DoNotSaveInHistory |
					NotPersistent |
					DoNotAnnounceEntity);

		int id = -1;
		QObject *pr;
		emit delegateEntity (e, &id, &pr);
		if (id == -1)
		{
			QString errorString = QString ("Could not find "
						"plugin to download %1 to %2.")
					.arg (url.toString ())
					.arg (location);
			qWarning () << Q_FUNC_INFO
					<< errorString;
			throw std::runtime_error (qPrintable (errorString));
		}

		PendingResource prdata =
		{
			url
		};

		PendingResources_ [id] = prdata;

		connect (pr,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleResourceFinished (int)),
				Qt::UniqueConnection);
		connect (pr,
				SIGNAL (jobRemoved (int)),
				this,
				SLOT (handleResourceRemoved (int)),
				Qt::UniqueConnection);
		connect (pr,
				SIGNAL (jobError (int, IDownload::Error::Type)),
				this,
				SLOT (handleResourceError (int, IDownload::Error::Type)),
				Qt::UniqueConnection);
	}

	QString ExternalResourceManager::GetResourcePath (const QUrl& url) const
	{
		return ResourcesDir_.filePath (URLToFileName (url));
	}

	void ExternalResourceManager::ClearCaches ()
	{
		Q_FOREACH (const QString& fname, ResourcesDir_.entryList ())
			ResourcesDir_.remove (fname);
	}

	void ExternalResourceManager::ClearCachedResource (const QUrl& url)
	{
		ResourcesDir_.remove (URLToFileName (url));
	}

	void ExternalResourceManager::handleResourceFinished (int id)
	{
		if (!PendingResources_.contains (id))
			return;

		PendingResource pr = PendingResources_.take (id);

		ResourcesDir_.refresh ();

		emit resourceFetched (pr.URL_);
	}

	void ExternalResourceManager::handleResourceRemoved (int id)
	{
		if (!PendingResources_.contains (id))
			return;

		PendingResources_.remove (id);
	}

	void ExternalResourceManager::handleResourceError (int id, IDownload::Error::Type error)
	{
		if (!PendingResources_.contains (id))
			return;

		qWarning () << Q_FUNC_INFO
				<< "got error"
				<< static_cast<int> (error)
				<< "for PendingResource"
				<< id
				<< PendingResources_ [id].URL_;
		PendingResources_.remove (id);
	}
}
}
