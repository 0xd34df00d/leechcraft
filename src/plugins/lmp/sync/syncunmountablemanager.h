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

#include <QStringList>
#include <QHash>
#include "interfaces/lmp/iunmountablesync.h"
#include "transcodingparams.h"
#include "syncmanagerbase.h"
#include "copymanager.h"

namespace LeechCraft
{
namespace LMP
{
	class IUnmountableSync;

	class SyncUnmountableManager : public SyncManagerBase
	{
		Q_OBJECT
	public:
		struct AddFilesParams
		{
			IUnmountableSync* Syncer_;
			QByteArray DevID_;
			QByteArray StorageID_;
			QStringList Files_;
			TranscodingParams TCParams_;
		};
	private:
		QHash<QString, AddFilesParams> Source2Params_;

		struct CopyJob
		{
			QObject* GetObject () const
			{
				return Syncer_->GetObject ();
			}

			void Upload () const
			{
				Syncer_->Upload (Filename_, OrigPath_, DevID_, StorageID_);
			}

			QString Filename_;
			bool RemoveOnFinish_;

			IUnmountableSync *Syncer_;
			QByteArray DevID_;
			QByteArray StorageID_;
			QString OrigPath_;
		};
		CopyManager<CopyJob> *CopyMgr_;
	public:
		SyncUnmountableManager (QObject* = 0);

		void AddFiles (const AddFilesParams&);
	protected slots:
		void handleFileTranscoded (const QString& from, const QString& transcoded, QString);
	};
}
}
