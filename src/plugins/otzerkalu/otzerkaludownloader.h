/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QStack>
#include <QWebFrame>

#include <interfaces/structures.h>
#include <plugininterface/util.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
namespace Otzerkalu
{
	struct DownloadParams
	{
		DownloadParams ();
		DownloadParams (const QUrl& downloadUrl, const QString& destDir,
				int recLevel, bool fromOtherSite);
		QUrl DownloadUrl_;
		QString DestDir_;
		int RecLevel_;
		bool FromOtherSite_;
	};
	
	class OtzerkaluDownloader : public QObject
	{
		Q_OBJECT

		struct FileData
		{
			FileData ();
			FileData (const QUrl& url, const QString& filename, int recLevel);
			QUrl Url_;
			QString Filename_;
			int RecLevel_;
		};
		
		const DownloadParams Param_;
		QMap<int, FileData> FileMap_;
	public:
		OtzerkaluDownloader (const DownloadParams& param, QObject *parent = 0);
		
		void Begin ();
	private:
		QString Download (const QUrl&);
		bool WriteData (const QString& filename, const QString& data);
		void HandleProvider (QObject *provider, int id, const QUrl& url,
				const QString& filename, int recLevel);
	private slots:
		void handleJobFinished (int id);
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
	};
};
};

#endif // PLUGINS_OTZERKALU_OTZERKALUDOWNLOADER_H
