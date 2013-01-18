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

#include "graffititab.h"
#include <functional>
#include <QFileSystemModel>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QtDebug>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/itagresolver.h>
#include <interfaces/lmp/mediainfo.h>
#include "filesmodel.h"

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	GraffitiTab::GraffitiTab (ILMPProxy_ptr proxy, const TabClassInfo& tc, QObject *plugin)
	: LMPProxy_ (proxy)
	, TC_ (tc)
	, Plugin_ (plugin)
	, FSModel_ (new QFileSystemModel (this))
	, FilesModel_ (new FilesModel (this))
	{
		Ui_.setupUi (this);

		FSModel_->setRootPath (QDir::homePath ());
		FSModel_->setFilter (QDir::Dirs | QDir::NoDotAndDotDot);
		FSModel_->setReadOnly (true);
		Ui_.DirectoryTree_->setModel (FSModel_);

		Ui_.FilesList_->setModel (FilesModel_);
	}

	TabClassInfo GraffitiTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* GraffitiTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void GraffitiTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* GraffitiTab::GetToolBar () const
	{
		return 0;
	}

	void GraffitiTab::on_DirectoryTree__activated (const QModelIndex& index)
	{
		setEnabled (false);
		FilesModel_->Clear ();
		const auto& path = FSModel_->filePath (index);

		auto watcher = new QFutureWatcher<QList<QFileInfo>> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleIterateFinished ()));

		auto worker = [this, path] () { return LMPProxy_->RecIterateInfo (path, false); };
		watcher->setFuture (QtConcurrent::run (std::function<QList<QFileInfo> ()> (worker)));
	}

	void GraffitiTab::handleIterateFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QList<QFileInfo>>*> (sender ());
		watcher->deleteLater ();

		const auto& files = watcher->result ();
		FilesModel_->AddFiles (files);

		auto resolver = LMPProxy_->GetTagResolver ();
		auto worker = [resolver, files] () -> QList<MediaInfo>
		{
			QList<MediaInfo> infos;
			for (const auto& file : files)
				try
				{
					infos << resolver->ResolveInfo (file.absoluteFilePath ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
				}
			return infos;
		};

		auto scanWatcher = new QFutureWatcher<QList<MediaInfo>> ();
		connect (scanWatcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleScanFinished ()));
		scanWatcher->setFuture (QtConcurrent::run (std::function<QList<MediaInfo> ()> (worker)));
	}

	void GraffitiTab::handleScanFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QList<MediaInfo>>*> (sender ());
		watcher->deleteLater ();

		FilesModel_->SetInfos (watcher->result ());
		setEnabled (true);
	}
}
}
}
