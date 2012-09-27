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

#include "dumbsync.h"
#include <memory>
#include <functional>
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <interfaces/lmp/ilmpproxy.h>

typedef std::shared_ptr<QFile> QFile_ptr;

namespace LeechCraft
{
namespace LMP
{
namespace DumbSync
{
	void Plugin::Init (ICoreProxy_ptr)
	{
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

	QObject* Plugin::GetObject ()
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
			const int maxDim = 200;
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

			px.save (targetDir.absoluteFilePath ("cover.jpg"), "JPG", 80);
		}
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QString& to, const QString& relPath)
	{
		QString target = to;
		if (!target.endsWith ('/') && !relPath.startsWith ('/'))
			target += '/';
		target += relPath;

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

		const auto& artPath = LMPProxy_->FindAlbumArt (origLocalPath);
		std::function<WorkerThreadResult (void)> copier = [target, localPath, artPath] () -> WorkerThreadResult
				{
					QFile_ptr file (new QFile (localPath));
					file->copy (target);
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
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_dumbsync, LeechCraft::LMP::DumbSync::Plugin);
