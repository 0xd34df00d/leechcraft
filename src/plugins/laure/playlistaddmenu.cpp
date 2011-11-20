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
#include <QRegExp>
#include <QUrl>
#include "playlistview.h"
#include "chooseurldialog.h"


namespace LeechCraft
{
namespace Laure
{
	PlayListAddMenu::PlayListAddMenu (QWidget *parent)
	: QMenu (parent)
	{
#ifdef HAVE_MAGIC
		Magic_ = boost::shared_ptr<magic_set> (magic_open (MAGIC_MIME_TYPE),
				magic_close);
		magic_load (Magic_.get (), NULL);
#else
		Formats_ << "3gp" << "asf" << "wmv" << "au" << "avi"
				<< "flv" << "mov" << "mp4" << "ogm" << "ogg"
				<< "mkv" << "mka" << "ts" << "mpg" << "mp3"
				<< "mp2" << "nsc" << "nsv" << "nut" << "a52"
				<< "dts" << "aac" << "flac" << "dv" << "vid"
				<< "tta" << "tac" << "ty" << "wav" << "xa";
#endif
		QAction *addFiles = new QAction (tr ("Add files"), this);
		QAction *addFolder = new QAction (tr ("Add folder"), this);
		QAction *addURL = new QAction (tr ("Add URL"), this);
		QAction *importPL = new QAction (tr ("Import playList"), this);
		
		addAction (addFiles);
		addAction (addFolder);
		addAction (addURL);
		addAction (importPL);
		
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
		connect (importPL,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleImportPlayList ()));
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
		if (d.exec () == QDialog::Accepted && d.IsUrlValid ())
			emit addItem (d.GetUrl ());
	}
	
	void PlayListAddMenu::handleImportPlayList ()
	{
		const QString& fileName = QFileDialog::getOpenFileName (this,
				tr ("Choose playlist"), QDir::homePath (), "*.m3u");
		if (fileName.isEmpty ())
			return;
		
		const QString& mime = QString (magic_file (Magic_.get (), fileName.toAscii ()));
		if (!mime.contains ("text"))
			return;
		
		if (!QFileInfo (fileName).suffix ().compare ("m3u", Qt::CaseInsensitive))
			LoadM3U (fileName);
	}
	
	void PlayListAddMenu::LoadM3U (const QString& fileName)
	{
		QUrl globalUrl (fileName);
		QFile file (fileName);
		
		if (!file.open (QIODevice::ReadOnly| QIODevice::Text))
			return;
		
		while (!file.atEnd ())
		{
			QString line = file.readLine ();
			line.chop (1);
			if (line.indexOf (QRegExp ("#EXT(M3U)|(INF)")) < 0)
			{
				QUrl url (line);
				if (url.isRelative ())
					url = globalUrl.resolved (url);
				emit addItem (url.toString ());
			}
		}
	}

	QFileInfoList PlayListAddMenu::StoragedFiles (const QString& path)
	{
		QDir dir (path);
		QFileInfoList list;
		const auto& fil = dir.entryInfoList (QDir::AllEntries | QDir::NoDotAndDotDot);
		
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
	
	bool PlayListAddMenu::IsFileSupported (const QFileInfo& file) const
	{
#ifdef HAVE_MAGIC
		const QString& mime = magic_file (Magic_.get (),
				file.absoluteFilePath ().toAscii ());
		return mime.contains ("audio") || mime.contains ("video");		
#else
		Q_FOREACH (const QString& format, Formats_)
			if (file.suffix () == format)
				return true;
		return false;
#endif
	}
}
}