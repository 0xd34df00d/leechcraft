/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDir>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>

class QSize;
class QImage;

namespace LC::Poshuku
{
	class IProxyObject;
	class IBrowserWidget;
}

namespace LC::Poshuku::SpeedDial
{
	class ImageCache : public QObject
	{
		Q_OBJECT

		const QDir CacheDir_;
		IProxyObject& Proxy_;

		std::unordered_map<QWidget*, std::pair<QUrl, std::unique_ptr<IBrowserWidget>>> CurrentLoads_;
		QList<QUrl> QueuedLoads_;
	public:
		explicit ImageCache (IProxyObject&);
		~ImageCache () override;

		QImage GetSnapshot (const QUrl&);
		QSize GetThumbSize () const;
	private:
		void EnsureEnqueued (const QUrl&);
		void StartNextLoad ();
		void Render (QWidget*);
	private slots:
		void handleLoadFinished ();
	signals:
		void gotSnapshot (const QUrl&, const QImage&);
	};
}
