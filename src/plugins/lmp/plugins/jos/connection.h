/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QDir>
#include <libimobiledevice/afc.h>
#include <interfaces/lmp/iunmountablesync.h>
#include "mobileraii.h"

template<typename T>
class QFutureWatcher;

namespace LC
{
namespace LMP
{
struct UnmountableFileInfo;

namespace jOS
{
	class GpodDb;
	struct UploadResult;

	class Connection : public QObject
	{
		Q_OBJECT

		const MobileRaii<idevice_t, idevice_error_t> Device_;
		const MobileRaii<lockdownd_client_t, lockdownd_error_t> Lockdown_;
		const MobileRaii<lockdownd_service_descriptor_t, lockdownd_error_t> Service_;
		const MobileRaii<afc_client_t, afc_error_t> AFC_;

		const QString TempDirPath_;

		bool CopiedDb_ = false;

		GpodDb *DB_ = nullptr;

		struct QueueItem
		{
			QString LocalPath_;
			UnmountableFileInfo Info_;
		};
		QList<QueueItem> UploadQueue_;

		QFutureWatcher<UploadResult> *CurUpWatcher_ = nullptr;
	public:
		enum CopyOption
		{
			NoCopyOption = 0x0,
			CopyCreate = 0x1
		};
		Q_DECLARE_FLAGS (CopyOptions, CopyOption)

		Connection (const QByteArray&);

		afc_client_t GetAFC () const;

		void Upload (const QString&, const UnmountableFileInfo&);

		QString GetFileInfo (const QString&, const QString&) const;
		bool Exists (const QString&);
	private:
		QString GetNextFilename (const QString&);

		QStringList ReadDir (const QString&, QDir::Filters);
		bool DownloadDir (const QString&, CopyOptions = NoCopyOption);
		bool DownloadFile (const QString&);
		bool MkDir (const QString&);
		bool UploadDir (const QString&);
		bool UploadFile (const QString&);
		bool UploadDatabase ();
	private slots:
		void itdbCopyFinished ();
		void rotateUploadQueue ();
		void handleTrackUploaded ();
		void handleDbLoaded ();
	signals:
		void error (const QString&);
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};

	typedef std::shared_ptr<Connection> Connection_ptr;
}
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::LMP::jOS::Connection::CopyOptions)
