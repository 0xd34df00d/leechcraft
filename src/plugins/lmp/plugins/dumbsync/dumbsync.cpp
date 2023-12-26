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
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>

using QFile_ptr = std::shared_ptr<QFile>;

namespace LC::LMP::DumbSync
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"LMP_DumbSync">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lmp_dumbsync");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpdumbsyncsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{

	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.DumbSync";
	}

	QString Plugin::GetName () const
	{
		return "LMP DumbSync";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with portable players that show themselves as Flash drives, like Rockbox players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CollectionSync";
		return result;
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
		QFileInfo fi (path);
		if (!fi.isDir () || !fi.isWritable ())
			return SyncConfLevel::None;

		if (fi.dir ().entryList (QDir::Dirs).contains (".rockbox", Qt::CaseInsensitive) ||
			fi.dir ().entryList (QDir::Dirs).contains ("music", Qt::CaseInsensitive))
			return SyncConfLevel::High;

		return SyncConfLevel::Medium;
	}

	namespace
	{
		struct WorkerThreadResult
		{
			QFile_ptr File_;
		};

		QImage GetScaledPixmap (const QString& pxFile)
		{
			if (pxFile.isEmpty ())
				return QImage ();

			QImage img (pxFile);
			const int maxDim = XmlSettingsManager::Instance ().property ("CoverDim").toInt ();
			if (img.size ().width () <= maxDim && img.size ().height () <= maxDim)
				return img;

			return img.scaled (maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		void WriteScaledPixmap (const QString& pxFile, const QString& target)
		{
			const auto& targetDir = QFileInfo (target).absoluteDir ();
			if (targetDir.exists ("cover.jpg"))
				return;

			const auto& px = GetScaledPixmap (pxFile);
			if (px.isNull ())
				return;

			const auto& name = XmlSettingsManager::Instance ().property ("CoverName").toString ();
			px.save (targetDir.absoluteFilePath (name), "JPG", 80);
		}
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QString& to, const QString& relPath)
	{
		QString target = to;
		if (!target.endsWith ('/') && !relPath.startsWith ('/'))
			target += '/';
		target += relPath;
		qDebug () << Q_FUNC_INFO
				<< "uploading"
				<< localPath
				<< "(from"
				<< origLocalPath
				<< ") to "
				<< target;

		const QString& dirPath = relPath.left (relPath.lastIndexOf ('/'));
		if (!QDir (to).mkpath (dirPath))
		{
			emit uploadFinished (localPath,
					QFile::PermissionsError,
					tr ("Unable to create the directory path %1 on target device %2.")
						.arg (dirPath)
						.arg (to));
			return;
		}

		auto watcher = new QFutureWatcher<WorkerThreadResult> (this);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleCopyFinished ()));

		const auto& artPath = LMPProxy_->GetUtilProxy ()->FindAlbumArt (origLocalPath);
		std::function<WorkerThreadResult (void)> copier = [target, localPath, artPath] () -> WorkerThreadResult
				{
					QFile_ptr file (new QFile (localPath));
					file->copy (target);
					if (XmlSettingsManager::Instance ().property ("UploadCovers").toBool ())
						WriteScaledPixmap (artPath, target);
					return { file };
				};
		const auto& future = QtConcurrent::run (copier);
		watcher->setFuture (future);
	}

	void Plugin::handleCopyFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<WorkerThreadResult>*> (sender ());
		if (!watcher)
			return;

		const auto& result = watcher->result ();
		auto file = result.File_;
		qDebug () << Q_FUNC_INFO << file->error ();
		if (file->error () != QFile::NoError)
			qWarning () << Q_FUNC_INFO
					<< file->errorString ();

		emit uploadFinished (file->fileName (), file->error (), file->errorString ());
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_dumbsync, LC::LMP::DumbSync::Plugin);
