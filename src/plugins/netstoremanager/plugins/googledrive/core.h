/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Core : public QObject
	{
		Q_OBJECT
		Q_DISABLE_COPY (Core);

		ICoreProxy_ptr Proxy_;

		Core ();

		QObjectList Downloaders_;
		QMap<int, QObject*> Id2Downloader_;
		QMap<int, QString> Id2SavePath_;
	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetProxy () const;

		void SendEntity (const LeechCraft::Entity& e);
		void DelegateEntity (const LeechCraft::Entity& e, const QString& targetPath);
	private:
		void HandleProvider (QObject *provider, int id);

	private slots:
		void handleJobFinished (int id);
		void handleJobRemoved (int id);
		void handleJobError (int id, IDownload::Error err);

	signals:
		void gotEntity (const LeechCraft::Entity& e);
		void delegateEntity (const LeechCraft::Entity& e, int *id, QObject **provider);
	};
}
}
}
