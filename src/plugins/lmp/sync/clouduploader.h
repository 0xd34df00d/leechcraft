/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#pragma once

#include <QObject>
#include <QList>
#include <interfaces/lmp/icloudstorageplugin.h>

namespace LeechCraft
{
namespace LMP
{
	class CloudUploader : public QObject
	{
		Q_OBJECT

		ICloudStoragePlugin *Cloud_;
	public:
		struct UploadJob
		{
			bool RemoveOnFinish_;
			QString Account_;
			QString Filename_;
		};
	private:
		QList<UploadJob> Queue_;
		UploadJob CurrentJob_;
	public:
		CloudUploader (ICloudStoragePlugin*, QObject* = 0);

		void Upload (const UploadJob&);
	private:
		void StartJob (const UploadJob&);
		bool IsRunning () const;
	private slots:
		void handleUploadFinished (const QString& localPath,
				LeechCraft::LMP::CloudStorageError error, const QString& errorStr);
	signals:
		void startedCopying (const QString&);
		void finishedCopying ();
	};
}
}
