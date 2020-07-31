/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QHash>
#include "interfaces/lmp/iunmountablesync.h"
#include "transcodingparams.h"
#include "syncmanagerbase.h"
#include "copymanager.h"

namespace LC
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
			QObject* GetQObject () const
			{
				return Syncer_->GetQObject ();
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
