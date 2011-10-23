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
#include <QDebug>
#include <boost/shared_ptr.hpp>
#include <magic.h>
#include "playlistview.h"
#include "chooseurldialog.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListAddMenu::PlayListAddMenu (QWidget *parent)
	: QMenu (parent)
	{
#ifdef Q_WS_WIN
		SupportedFormat_ << "3gp" << "asf" << "wmv" << "au" << "avi"
				<< "flv" << "mov" << "mp4" << "ogm" << "ogg"
				<< "mkv" << "mka" << "ts" << "mpg" << "mp3"
				<< "mp2" << "nsc" << "nsv" << "nut" << "a52"
				<< "dts" << "aac" << "flac" << "dv" << "vid"
				<< "tta" << "tac" << "ty" << "wav" << "xa";
#endif
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
			emit addItem (fileName);
	}

	void PlayListAddMenu::handleAddFolder ()
	{
		const QString& fileDir = QFileDialog::getExistingDirectory (this,
				tr ("Choose directory"), QDir::homePath ());
		if (fileDir.isEmpty ())
			return;
		
		const QFileInfoList& fileInfoList = StoragedFiles (fileDir);
		Q_FOREACH (const QFileInfo& fileInfo, fileInfoList)
			emit addItem (fileInfo.absoluteFilePath ());
	}
	
	void PlayListAddMenu::handleAddUrl ()
	{
		ChooseURLDialog d;
		if (d.exec () == QDialog::Accepted)
			emit addItem (d.GetUrl ());
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
			else if (IsFileSupported (fi))
 				list << fi;
		}
		return list;
	}
	
	bool PlayListAddMenu::IsFileSupported (const QFileInfo& file)
	{
#ifdef Q_WS_X11
		boost::shared_ptr<magic_set> magic (magic_open (MAGIC_MIME_TYPE),
				magic_close);
	
		magic_load (magic.get (), NULL);
		const QString& mime =  QString (magic_file (magic.get (),
						file.absoluteFilePath ().toAscii ()));
		return mime.contains ("audio") || mime.contains ("video");		
#endif
#ifdef Q_WS_WIN
		Q_FOREACH (const QString& format, SupportedFormat_)
			if (file.suffix () == format)
				return true;
		return false;
#endif
	}
}
}