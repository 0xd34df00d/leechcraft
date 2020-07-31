/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagecache.h"
#include <QDesktopServices>
#include <QImage>
#include <QUrl>
#include <QDateTime>
#include <QWebPage>
#include <QWebFrame>
#include <QPainter>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/sll/delayedexecutor.h>
#include <util/sll/util.h>
#include "xmlsettingsmanager.h"

namespace LC::Poshuku::SpeedDial
{
	const QSize RenderSize { 1280, 720 };
	const QSize ThumbSize = RenderSize / 8;

	ImageCache::ImageCache (const ICoreProxy_ptr& proxy)
	: CacheDir_ { Util::GetUserDir (Util::UserDir::Cache, "poshuku/speeddial/snapshots") }
	, Proxy_ { proxy }
	{
	}

	QImage ImageCache::GetSnapshot (const QUrl& url)
	{
		const auto& filename = QString::number (qHash (url)) + ".png";
		const auto& path = CacheDir_.filePath (filename);
		const QFileInfo info { path };
		if (info.exists ())
		{
			const auto validity = XmlSettingsManager::Instance ().property ("ValidFor").toInt ();
			if (info.lastModified ().daysTo (QDateTime::currentDateTime ()) <= validity)
			{
				QImage result { path };
				if (!result.isNull ())
					return result;
			}

			QFile::remove (path);
		}

		if (Url2Page_.contains (url))
			return {};

		const auto page = new QWebPage;
		const auto frame = page->mainFrame ();
		frame->setScrollBarPolicy (Qt::Vertical, Qt::ScrollBarAlwaysOff);
		frame->setScrollBarPolicy (Qt::Horizontal, Qt::ScrollBarAlwaysOff);
		page->setViewportSize (RenderSize);
		page->setNetworkAccessManager (Proxy_->GetNetworkAccessManager ());

		const auto settings = page->settings ();
		settings->setAttribute (QWebSettings::DnsPrefetchEnabled, false);
		settings->setAttribute (QWebSettings::JavaEnabled, false);
		settings->setAttribute (QWebSettings::PluginsEnabled, false);
		settings->setAttribute (QWebSettings::DeveloperExtrasEnabled, false);
		settings->setAttribute (QWebSettings::XSSAuditingEnabled, false);

		settings->setAttribute (QWebSettings::JavascriptEnabled,
				XmlSettingsManager::Instance ().property ("EnableJS").toBool ());

		Page2Url_ [page] = url;
		Url2Page_ [url] = page;
		connect (page,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleLoadFinished ()));

		PendingLoads_ << page;

		if (PendingLoads_.size () <= 2)
			page->mainFrame ()->load (url);

		return {};
	}

	QSize ImageCache::GetThumbSize () const
	{
		return ThumbSize;
	}

	void ImageCache::Render (QWebPage *page)
	{
		const auto pullNextGuard = Util::MakeScopeGuard ([this]
				{
					if (PendingLoads_.isEmpty ())
						return;

					const auto page = PendingLoads_.takeFirst ();
					page->mainFrame ()->load (Page2Url_.value (page));
				});

		PendingLoads_.removeAll (page);

		const auto& url = Page2Url_.take (page);
		if (url.isEmpty ())
			return;

		Url2Page_.remove (url);

		QImage image { page->viewportSize (), QImage::Format_ARGB32 };
		QPainter painter { &image };

		page->mainFrame ()->render (&painter);
		painter.end ();

		page->deleteLater ();

		const auto& thumb = image.scaled (ThumbSize,
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
		thumb.save (CacheDir_.filePath (QString::number (qHash (url))) + ".png");

		emit gotSnapshot (url, thumb);
	}

	void ImageCache::handleLoadFinished ()
	{
		const auto page = qobject_cast<QWebPage*> (sender ());

		new Util::DelayedExecutor
		{
			[this, page] { Render (page); },
			1000
		};
	}
}
