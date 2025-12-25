/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumbsync.h"
#include <memory>
#include <functional>
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

using QFile_ptr = std::shared_ptr<QFile>;

namespace LC::LMP::DumbSync
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"LMP_DumbSync">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpdumbsyncsettings.xml"_qs);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.DumbSync"_qba;
	}

	QString Plugin::GetName () const
	{
		return "LMP DumbSync"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Music collection synchronization with Mass Storage-like devices, like USB Flash drives and Rockbox players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.LMP.CollectionSync"_qba };
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QString Plugin::GetSyncSystemName () const
	{
		return tr ("dumb copying");
	}

	SyncConfLevel Plugin::CouldSync (const QString& path)
	{
		const QFileInfo fi { path };
		if (!fi.isDir () || !fi.isWritable ())
			return SyncConfLevel::None;

		if (const auto& entries = fi.dir ().entryList (QDir::Dirs);
			entries.contains (".rockbox"_ql, Qt::CaseInsensitive) ||
			entries.contains ("music"_ql, Qt::CaseInsensitive))
			return SyncConfLevel::High;

		return SyncConfLevel::Medium;
	}

	namespace
	{
		QImage GetScaledPixmap (const QString& pxFile)
		{
			if (pxFile.isEmpty ())
				return {};

			QImage img { pxFile };
			const int maxDim = XmlSettingsManager::Instance ().property ("CoverDim").toInt ();
			if (img.size ().width () <= maxDim && img.size ().height () <= maxDim)
				return img;

			return img.scaled (maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		void WriteScaledPixmap (const QString& pxFile, const QString& target)
		{
			const auto& coverName = XmlSettingsManager::Instance ().property ("CoverName").toString ();

			const auto& targetDir = QFileInfo { target }.absoluteDir ();
			if (targetDir.exists (coverName))
				return;

			const auto& px = GetScaledPixmap (pxFile);
			if (px.isNull ())
				return;

			constexpr auto aaQuality = 80;
			if (const auto& fullCoverPath = targetDir.absoluteFilePath (coverName);
				!px.save (fullCoverPath, "JPG", aaQuality))
				qWarning () << "unable to save album art from" << pxFile << "to" << fullCoverPath;
		}
	}

	Util::ContextTask<Plugin::UploadResult> Plugin::Upload (UploadJob job)
	{
		QString target = job.Target_;
		if (!target.endsWith ('/') && !job.TargetRelPath_.startsWith ('/'))
			target += '/';
		target += job.TargetRelPath_;
		qDebug () << "uploading" << job.LocalPath_ << "(from" << job.OriginalLocalPath_ << ") to " << target;

		const QString& dirPath = job.TargetRelPath_.section ('/', 0, -2);
		if (!QDir (job.Target_).mkpath (dirPath))
			co_return UploadFailure
			{
				QFile::PermissionsError,
				tr ("Unable to create the directory path %1 on target device %2.").arg (dirPath, job.Target_)
			};

		const auto& artPath = LMPProxy_->GetUtilProxy ()->FindAlbumArt (job.OriginalLocalPath_);

		co_return co_await QtConcurrent::run ([&] () -> UploadResult
				{
					if (QFile file { job.LocalPath_ };
						!file.copy (target))
					{
						qWarning () << "failed to copy" << job.LocalPath_ << "to" << target << ":" << file.error () << file.errorString ();
						return { Util::AsLeft, { file.error (), file.errorString () } };
					}
					if (XmlSettingsManager::Instance ().property ("UploadCovers").toBool ())
						WriteScaledPixmap (artPath, target);
					return UploadSuccess {};
				});
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_dumbsync, LC::LMP::DumbSync::Plugin);
