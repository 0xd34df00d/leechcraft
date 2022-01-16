/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagecache.h"
#include <QImage>
#include <QUrl>
#include <QDateTime>
#include <QPainter>
#include <QWidget>
#include <QTimer>
#include <QShowEvent>
#include <QCoreApplication>
#include <QtDebug>
#include <util/sys/paths.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <interfaces/poshuku/iproxyobject.h>
#include <interfaces/poshuku/iwebview.h>
#include "xmlsettingsmanager.h"

namespace LC::Poshuku::SpeedDial
{
	const QSize RenderSize { 1280, 720 };
	const QSize ThumbSize = RenderSize / 8;

	ImageCache::ImageCache (IProxyObject& proxy)
	: CacheDir_ { Util::GetUserDir (Util::UserDir::Cache, "poshuku/speeddial/snapshots") }
	, Proxy_ { proxy }
	{
	}

	ImageCache::~ImageCache () = default;

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

		if (QueuedLoads_.contains (url))
			return {};

		if (std::any_of (CurrentLoads_.begin (), CurrentLoads_.end (),
				[&url] (const auto& pair) { return pair.second.first == url; }))
			return {};

		QueuedLoads_ << url;
		StartNextLoad ();

		return {};
	}

	QSize ImageCache::GetThumbSize () const
	{
		return ThumbSize;
	}

	void ImageCache::StartNextLoad ()
	{
		if (QueuedLoads_.isEmpty ())
			return;

		if (CurrentLoads_.size () >= 2)
			return;

		const auto& url = QueuedLoads_.takeFirst ();

		auto page = Proxy_.CreateBrowserWidget ();

		const auto view = page->GetWebView ();
		const auto viewWidget = view->GetQWidget ();
		viewWidget->setFixedSize (RenderSize);
		viewWidget->setWindowFlags (Qt::Window
				| Qt::BypassWindowManagerHint
				| Qt::WindowDoesNotAcceptFocus
				| Qt::WindowTransparentForInput);

		// Move + show is required for QtWebEngine to actually start displaying the contents.
		// It's unfortunate this implementation detail leaks into another plugin,
		// but there seems to be no better option that'd be general enough.
		viewWidget->move (-RenderSize.width (), -RenderSize.height ());
		viewWidget->show ();

		viewWidget->setStyleSheet ("QScrollBar { width: 0px; height: 0px; }");

		using enum IWebView::Attribute;
		view->SetAttribute (PluginsEnabled, false);
		view->SetAttribute (XSSAuditingEnabled, false);
		view->SetAttribute (JavascriptEnabled,
				XmlSettingsManager::Instance ().property ("EnableJS").toBool ());

		view->Load (url);

		connect (viewWidget,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleLoadFinished ()));
		CurrentLoads_.try_emplace (viewWidget, url, std::move (page));
	}

	void ImageCache::Render (QWidget *webWidget)
	{
		const auto pos = CurrentLoads_.find (webWidget);
		if (pos == CurrentLoads_.end ())
		{
			qWarning () << "unable to find"
					<< webWidget;

			CurrentLoads_.clear ();
			StartNextLoad ();
			return;
		}

		auto [url, page] = std::move (pos->second);
		CurrentLoads_.erase (pos);
		StartNextLoad ();

		QImage image { RenderSize, QImage::Format_ARGB32 };
		QPainter painter { &image };

		webWidget->render (&painter);
		painter.end ();

		const auto& thumb = image.scaled (ThumbSize,
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
		thumb.save (CacheDir_.filePath (QString::number (qHash (url))) + ".png");

		emit gotSnapshot (url, thumb);
	}

	void ImageCache::handleLoadFinished ()
	{
		Render (qobject_cast<QWidget*> (sender ()));
	}
}
