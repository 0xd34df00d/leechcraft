/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "connection.h"
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QStringList>
#include <QtDebug>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QRandomGenerator>
#include <libimobiledevice/lockdown.h>
#include <gpod/itdb.h>
#include "afcfile.h"
#include "gpoddb.h"

namespace LC
{
namespace LMP
{
namespace jOS
{
	namespace
	{
		QString GetTemporaryDir ()
		{
			const auto& tempLocation = QStandardPaths::writableLocation (QStandardPaths::TempLocation);

			QTemporaryFile file { tempLocation + '/' + "lmp_jos_XXXXXX" };
			if (file.open ())
				return file.fileName ();
			else
				return tempLocation + "/lmp_jos";
		}
	}

	const QStringList DbSubdirs { "Artwork", "Device", "iTunes" };

	struct UploadResult
	{
		QString LocalPath_;
		QString ErrorString_;
	};

	Connection::Connection (const QByteArray& udid)
	: Device_ { MakeRaii<idevice_t> ([udid] (idevice_t *dev)
				{ return idevice_new (dev, udid.constData ()); },
			idevice_free) }
	, Lockdown_ { MakeRaii<lockdownd_client_t> ([this] (lockdownd_client_t *ld)
				{ return lockdownd_client_new_with_handshake (Device_, ld, "LMP jOS"); },
			lockdownd_client_free) }
	, Service_ { MakeRaii<lockdownd_service_descriptor_t> ([this] (lockdownd_service_descriptor_t *service)
				{ return lockdownd_start_service (Lockdown_, "com.apple.afc", service); },
			lockdownd_service_descriptor_free) }
	, AFC_ {  MakeRaii<afc_client_t> ([this] (afc_client_t *afc)
				{ return afc_client_new (Device_, Service_, afc); },
			afc_client_free) }
	, TempDirPath_ { GetTemporaryDir () }
	{
		qDebug () << Q_FUNC_INFO
				<< "created connection to"
				<< udid
				<< "with temp dir"
				<< TempDirPath_;

		auto watcher = new QFutureWatcher<bool> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (itdbCopyFinished ()));
		const auto& future = QtConcurrent::run ([this] () -> bool
			{
				bool result = true;
				for (const auto& dir : DbSubdirs)
					if (!DownloadDir ("/iTunes_Control/" + dir, CopyCreate))
						result = false;
				return result;
			});
		watcher->setFuture (future);
	}

	afc_client_t Connection::GetAFC () const
	{
		return AFC_;
	}

	void Connection::Upload (const QString& localPath, const UnmountableFileInfo& info)
	{
		UploadQueue_.append ({ localPath, info });

		if (DB_ && UploadQueue_.size () == 1)
			rotateUploadQueue ();
	}

	namespace
	{
		void ListCleanup (char **info)
		{
			if (!info)
				return;

			for (auto p = info; *p; ++p)
				free (*p);
			free (info);
		}
	}

	QString Connection::GetFileInfo (const QString& path, const QString& key) const
	{
		char **info = nullptr;
		if (const auto err = afc_get_file_info (AFC_, path.toUtf8 (), &info))
		{
			if (err != AFC_E_OBJECT_NOT_FOUND)
				qWarning () << Q_FUNC_INFO
						<< "error getting info for"
						<< path
						<< err;
			return {};
		}

		if (!info)
		{
			qWarning () << Q_FUNC_INFO
					<< "couldn't find any properties"
					<< key
					<< "for file"
					<< path
					<< "at all";
			return {};
		}

		auto guard = std::shared_ptr<void> (nullptr,
				[info] (void*) { ListCleanup (info); });

		QString lastKey;
		for (auto p = info; *p; ++p)
			if (lastKey.isEmpty ())
				lastKey = QString::fromUtf8 (*p);
			else
			{
				if (lastKey == key)
					return QString::fromUtf8 (*p);

				lastKey.clear ();
			}

		qWarning () << Q_FUNC_INFO
				<< "couldn't find property"
				<< key
				<< "for file"
				<< path;
		return {};
	}

	bool Connection::Exists (const QString& path)
	{
		return !GetFileInfo (path, "st_ifmt").isNull ();
	}

	QString Connection::GetNextFilename (const QString& origPath)
	{
		auto mdir = [] (int num)
		{
			return QString { "/iTunes_Control/Music/F%1" }
					.arg (num, 2, 10, QChar { '0' });
		};

		int lastMD = 0;
		for ( ; lastMD < 100; ++lastMD)
			if (!Exists (mdir (lastMD)))
				break;

		if (!lastMD)
		{
			qWarning () << Q_FUNC_INFO
					<< "no music dirs were found";
			return {};
		}

		auto& gen = *QRandomGenerator::global ();

		const auto dirNum = gen.bounded (lastMD);
		const auto& dir = mdir (dirNum);
		if (!Exists (dir))
		{
			qWarning () << Q_FUNC_INFO
					<< "chosen directory for"
					<< dirNum
					<< "of"
					<< lastMD
					<< "doesn't exist";
			return {};
		}

		auto ext = origPath.section ('.', -1, -1).toLower ();
		if (ext.isEmpty ())
			ext = "mp3";

		QString filename;
		while (true)
		{
			filename = QString { "jos%1" }.arg (gen.bounded (999999), 6, 10, QChar { '0' });
			filename += '.' + ext;
			filename.prepend (dir + '/');

			if (!Exists (filename))
				break;
		}

		return filename;
	}

	namespace
	{
		template<typename AFC>
		afc_error_t TryReadDir (const AFC& afc, const QString& path)
		{
			char **list = nullptr;
			const auto ret = afc_read_directory (afc, path.toUtf8 (), &list);
			ListCleanup (list);
			return ret;
		}
	}

	QStringList Connection::ReadDir (const QString& path, QDir::Filters filters)
	{
		char **list = nullptr;
		if (const auto err = afc_read_directory (AFC_, path.toUtf8 (), &list))
		{
			qWarning () << Q_FUNC_INFO
					<< "error reading"
					<< path
					<< err;
			return {};
		}

		if (!list)
			return {};

		QStringList result;

		for (auto p = list; *p; ++p)
		{
			const auto& filename = QString::fromUtf8 (*p);
			free (*p);

			if (filters == QDir::NoFilter)
			{
				result << filename;
				continue;
			}

			if (filters & QDir::NoDotAndDotDot &&
					(filename == "." || filename == ".."))
				continue;

			if (!(filters & QDir::Hidden) && filename.startsWith ("."))
				continue;

			const auto& filetype = GetFileInfo (path + '/' + filename, "st_ifmt");

			if ((filetype == "S_IFREG" && filters & QDir::Files) ||
				(filetype == "S_IFDIR" && filters & QDir::Dirs) ||
				(filetype == "S_IFLNK" && !(filters & QDir::NoSymLinks)))
				result << filename;
		}

		free (list);

		return result;
	}

	bool Connection::DownloadDir (const QString& dir, CopyOptions options)
	{
		qDebug () << "copying dir" << dir;

		if (TryReadDir (AFC_, dir) == AFC_E_OBJECT_NOT_FOUND)
		{
			qDebug () << "dir not found";
			if (options & CopyCreate)
				QDir {}.mkpath (TempDirPath_ + dir);

			return true;
		}

		bool result = true;

		for (const auto& file : ReadDir (dir, QDir::Files | QDir::NoDotAndDotDot))
			if (!DownloadFile (dir + '/' + file))
				result = false;

		for (const auto& subdir : ReadDir (dir, QDir::Dirs | QDir::NoDotAndDotDot))
			if (!DownloadDir (dir + '/' + subdir))
				result = false;

		return result;
	}

	namespace
	{
		template<typename Src, typename Dest>
		bool CopyIODevice (Src&& srcFile, const QString& srcId, Dest&& destFile, const QString& destId)
		{
			if (!srcFile.isOpen () && !srcFile.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open src file at"
						<< srcId
						<< srcFile.errorString ();
				return false;
			}

			if (!destFile.isOpen () && !destFile.open (QIODevice::WriteOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open dest file at"
						<< destId
						<< srcFile.errorString ();
				return false;
			}

			for (qint64 copied = 0, size = srcFile.size (); copied < size; )
			{
				const auto& data = srcFile.read (1024 * 1024);
				if (data.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "got empty data chunk";
					return false;
				}

				if (destFile.write (data) == -1)
				{
					qWarning () << Q_FUNC_INFO
							<< "cannot write to destination file"
							<< destId
							<< destFile.errorString ();
					return false;
				}

				copied += data.size ();
			}

			return true;
		}
	}

	bool Connection::DownloadFile (const QString& file)
	{
		qDebug () << "copying file" << file;

		const auto& localFilePath = TempDirPath_ + file;
		const auto& localDirPath = localFilePath.section ('/', 0, -2);

		QDir dir;
		dir.mkpath (localDirPath);

		return CopyIODevice (AfcFile { file, this }, file, QFile { localFilePath }, localFilePath);
	}

	bool Connection::MkDir (const QString& path)
	{
		const auto err = afc_make_directory (AFC_, path.toUtf8 ());
		if (err)
			qWarning () << Q_FUNC_INFO
					<< path
					<< err;

		return !err;
	}

	bool Connection::UploadDir (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
		if (!Exists (path) && !MkDir (path))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot mkdir"
					<< path;
			return false;
		}

		const QDir localDir { TempDirPath_ + '/' + path };
		for (const auto& file : localDir.entryList (QDir::Files | QDir::NoDotAndDotDot))
			if (!UploadFile (path + '/' + file))
				return false;

		for (const auto& dir : localDir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
			if (!UploadDir (path + '/' + dir))
				return false;

		return true;
	}

	bool Connection::UploadFile (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
		return CopyIODevice (QFile { TempDirPath_ + '/' + path }, path, AfcFile { path, this }, path);
	}

	bool Connection::UploadDatabase ()
	{
		DB_->Save ();

		const auto& syncGuard = DB_->GetSyncGuard ();
		Q_UNUSED (syncGuard);
		for (const auto& dir : DbSubdirs)
			if (!UploadDir ("/iTunes_Control/" + dir))
				return false;

		return true;
	}

	void Connection::itdbCopyFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<bool>*> (sender ());
		const auto result = watcher->result ();
		watcher->deleteLater ();

		qDebug () << Q_FUNC_INFO
				<< "copy finished, is good?"
				<< result;

		CopiedDb_ = result;

		if (!result)
			emit error (tr ("Error loading iTunes database from the device."));

		DB_ = new GpodDb (TempDirPath_, this);
		auto loadWatcher = new QFutureWatcher<QString> (this);
		connect (loadWatcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleDbLoaded ()));

		loadWatcher->setFuture (QtConcurrent::run ([this] ()
				{
					qDebug () << Q_FUNC_INFO << "loading db...";
					auto res = DB_->Load ();
					if (res.Result_ != GpodDb::Result::NotFound)
						return res.Message_;

					qDebug () << "cannot find one, reinitializing the device";

					res = DB_->Reinitialize ();

					if (res.Result_ != GpodDb::Result::Success)
					{
						qWarning () << Q_FUNC_INFO
								<< "failed to reinitialize";
						return res.Message_;
					}

					if (!UploadDir ("/iTunes_Control/Music"))
						return tr ("Cannot upload Music directory after reinitializing");

					return DB_->Load ().Message_;
				}));
	}

	void Connection::rotateUploadQueue ()
	{
		if (UploadQueue_.isEmpty ())
		{
			if (!UploadDatabase ())
				qWarning () << Q_FUNC_INFO
						<< "error uploading database";
			return;
		}

		const auto& task = UploadQueue_.takeFirst ();
		qDebug () << Q_FUNC_INFO << "uploading" << task.LocalPath_;
		const auto& future = QtConcurrent::run ([this, task] () -> UploadResult
				{
					const auto& filename = GetNextFilename (task.LocalPath_);
					if (filename.isEmpty ())
						return { task.LocalPath_, tr ("Cannot get remote filename") };

					if (!CopyIODevice (QFile { task.LocalPath_ }, task.LocalPath_,
								AfcFile { filename, this }, filename))
						return { task.LocalPath_, tr ("Cannot copy file") };

					DB_->AddTrack (task.LocalPath_, filename, task.Info_);
					return { task.LocalPath_, {} };
				});

		CurUpWatcher_ = new QFutureWatcher<UploadResult> ();
		connect (CurUpWatcher_,
				SIGNAL (finished ()),
				this,
				SLOT (handleTrackUploaded ()));
		CurUpWatcher_->setFuture (future);
	}

	void Connection::handleTrackUploaded ()
	{
		const auto watcher = dynamic_cast<QFutureWatcher<UploadResult>*> (sender ());
		const auto& result = watcher->result ();
		watcher->deleteLater ();

		qDebug () << Q_FUNC_INFO << result.LocalPath_ << result.ErrorString_;

		emit uploadFinished (result.LocalPath_,
				result.ErrorString_.isEmpty () ? QFile::NoError : QFile::UnspecifiedError,
				result.ErrorString_);

		rotateUploadQueue ();
	}

	void Connection::handleDbLoaded ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QString>*> (sender ());
		const auto& msg = watcher->result ();
		watcher->deleteLater ();

		if (!msg.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot load database:"
					<< msg;
			delete DB_;
			DB_ = nullptr;
			return;
		}

		rotateUploadQueue ();
	}
}
}
}
