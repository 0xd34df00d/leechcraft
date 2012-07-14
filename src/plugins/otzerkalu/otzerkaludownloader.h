/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
#define PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
#include <QObject>
#include <QUrl>
#include <QWebFrame>
#include <QWebElementCollection>
#include <interfaces/structures.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
namespace Otzerkalu
{
	struct DownloadParams
	{
		QUrl DownloadUrl_;
		QString DestDir_;
		int RecLevel_;
		bool FromOtherSite_;

		DownloadParams ();
		DownloadParams (const QUrl& downloadUrl, const QString& destDir,
				int recLevel, bool fromOtherSite);
	};

	struct FileData
	{
		QUrl Url_;
		QString Filename_;
		int RecLevel_;

		FileData ();
		FileData (const QUrl& url, const QString& filename, int recLevel);
	};

	class OtzerkaluDownloader : public QObject
	{
		Q_OBJECT
		const DownloadParams Param_;
		QMap<int, FileData> FileMap_;
		QStringList DownloadedFiles_;
		int UrlCount_, ID_;
	public:
		OtzerkaluDownloader (const DownloadParams& param, int id, QObject *parent = 0);
		QString GetLastDownloaded () const;
		int FilesCount () const;
		void Begin ();
	private:
		QString Download (const QUrl&, int);
		QList<QUrl> CSSParser (const QString&) const;
		QString CSSUrlReplace (const QString&, const FileData&);
		bool HTMLReplace (QWebElementCollection::iterator element, const FileData& data);
		bool WriteData (const QString& filename, const QString& data);
		void HandleProvider (QObject *provider, int id, const QUrl& url,
				const QString& filename, int recLevel);
	private slots:
		void handleJobFinished (int id);
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
		void fileDownloaded (int id, int count);
		void mirroringFinished (int id);
	};
};
};

#endif // PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
