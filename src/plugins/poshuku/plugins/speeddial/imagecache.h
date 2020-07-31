/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDir>
#include <QMap>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>

class QSize;
class QWebPage;
class QImage;

namespace LC::Poshuku::SpeedDial
{
	class ImageCache : public QObject
	{
		Q_OBJECT

		const QDir CacheDir_;
		const ICoreProxy_ptr Proxy_;

		QMap<QWebPage*, QUrl> Page2Url_;
		QMap<QUrl, QWebPage*> Url2Page_;

		QList<QWebPage*> PendingLoads_;
	public:
		explicit ImageCache (const ICoreProxy_ptr&);

		QImage GetSnapshot (const QUrl&);
		QSize GetThumbSize () const;
	private:
		void Render (QWebPage*);
	private slots:
		void handleLoadFinished ();
	signals:
		void gotSnapshot (const QUrl&, const QImage&);
	};
}
