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
#include <QFile>

namespace LeechCraft
{
namespace LMP
{
	class ISyncPlugin;

	class CopyManager : public QObject
	{
		Q_OBJECT
	public:
		struct CopyJob
		{
			ISyncPlugin *Syncer_;
			QString From_;
			QString OrigPath_;
			bool RemoveOnFinish_;
			QString MountPoint_;
			QString Filename_;
		};
	private:
		QList<CopyJob> Queue_;
		CopyJob CurrentJob_;
	public:
		CopyManager (QObject* = 0);

		void Copy (const CopyJob&);
	private:
		void StartJob (const CopyJob&);
		bool IsRunning () const;
	private slots:
		void handleUploadFinished (const QString& localPath,
				QFile::FileError error, const QString& errorStr);
	signals:
		void startedCopying (const QString&);
		void finishedCopying ();
	};
}
}
