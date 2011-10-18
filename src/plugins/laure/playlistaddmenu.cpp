/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "playlistaddmenu.h"
#include <QFileDialog>
#include "playlistview.h"
#include "chooseurldialog.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListAddMenu::PlayListAddMenu (PlayListView *playListView, QWidget *parent)
	: QMenu (parent)
	, PlayListView_ (playListView)
	{
		QAction *addFiles = new QAction (tr ("Add files"), this);
		QAction *addFolder = new QAction (tr ("Add folder"), this);
		QAction *addURL = new QAction (tr ("Add URL"), this);
		
		addAction (addFiles);
		addAction (addFolder);
		addAction (addURL);
		
		connect (addURL,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleAddUrl ()));
		connect (addFiles,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleAddFiles ()));
		connect (addFolder,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleAddFolder ()));
	}
	
	void PlayListAddMenu::handleAddFiles ()
	{
		const QStringList& fileNames = QFileDialog::getOpenFileNames (this,
				tr ("Choose file"), QDir::homePath ());
		Q_FOREACH (const QString& fileName, fileNames)
			PlayListView_->AddItem (fileName);
	}

	void PlayListAddMenu::handleAddFolder ()
	{
		const QString& fileDir = QFileDialog::getExistingDirectory (this,
				tr ("Choose directory"), QDir::homePath ());
		if (fileDir.isEmpty ())
			return;
		
		const QFileInfoList& fileInfoList = StoragedFiles (fileDir);
		Q_FOREACH (const QFileInfo& fileInfo, fileInfoList)
			PlayListView_->AddItem (fileInfo.absoluteFilePath ());
	}
	
	void PlayListAddMenu::handleAddUrl ()
	{
		ChooseURLDialog d;
		if (d.exec () == QDialog::Accepted)
		{
			const QString& url = d.GetUrl ();
			PlayListView_->AddItem (url);
		}
	}

	QFileInfoList PlayListAddMenu::StoragedFiles (const QString& path)
	{
		QDir dir (path);
		QFileInfoList list;
		QFileInfoList fil = dir.entryInfoList (QDir::Dirs
				| QDir::Files | QDir::NoDotAndDotDot);
		
		if (fil.isEmpty ())
			return QFileInfoList ();
		
		Q_FOREACH (QFileInfo fi, fil)
		{
			if (fi.isDir ())
				list << StoragedFiles (fi.absoluteFilePath ());
			else
 				list << fi;
		}
		return list;
	}
}
}