/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>
#include <QList>

class QXmlStreamWriter;
class QWebView;
class QUrl;
class QImage;

namespace LC::Poshuku
{
class IProxyObject;
class IWebView;
class IBrowserWidget;

namespace SpeedDial
{
	class ImageCache;
	class CustomSitesManager;

	typedef QList<QPair<QUrl, QString>> TopList_t;

	struct LoadResult;

	class ViewHandler : public QObject
	{
		Q_OBJECT

		IWebView * const View_;
		IBrowserWidget * const BrowserWidget_;
		ImageCache * const ImageCache_;
		IProxyObject * const PoshukuProxy_;

		bool IsLoading_ = false;

		int PendingImages_ = 0;
	public:
		ViewHandler (IBrowserWidget*, ImageCache*, CustomSitesManager*, IProxyObject*);
	private:
		void LoadStatistics ();
		void WriteTables (const QList<QPair<QString, TopList_t>>&);
		void WriteTable (QXmlStreamWriter&, const TopList_t&, size_t, size_t, const QString&);
	private slots:
		void handleLoadStarted ();
	};
}
}
