/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
#define PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
#include <QObject>
#include <QUrl>
#include <QWebFrame>
#include <QWebElementCollection>
#include <interfaces/structures.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/core/icoreproxyfwd.h>

namespace LC
{
namespace Otzerkalu
{
	struct DownloadParams
	{
		QUrl DownloadUrl_;
		QString DestDir_;
		int RecLevel_ = 0;
		bool FromOtherSite_ = false;
	};

	struct FileData
	{
		QUrl Url_;
		QString Filename_;
		int RecLevel_ = 0;
	};

	class OtzerkaluDownloader : public QObject
	{
		Q_OBJECT

		const DownloadParams Param_;
		const ICoreProxy_ptr Proxy_;
		QSet<QString> DownloadedFiles_;
		int UrlCount_ = 0;
	public:
		OtzerkaluDownloader (const DownloadParams& param, const ICoreProxy_ptr&, QObject *parent = 0);
		void Begin ();
	private:
		QString Download (const QUrl&, int);
		QList<QUrl> CSSParser (const QString&) const;
		QString CSSUrlReplace (QString, const FileData&);
		bool HTMLReplace (QWebElement element, const FileData& data);
		bool WriteData (const QString& filename, const QString& data);
		void HandleJobFinished (const FileData& data);
	signals:
		void fileDownloaded (int count);
		void mirroringFinished ();
	};
};
};

#endif // PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
