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

#include "otzerkaludownloader.h"
#include <QWebElement>
#include <QWebPage>

namespace LeechCraft
{
namespace Otzerkalu
{
	DownloadParams::DownloadParams ()
	{
	}
	
	DownloadParams::DownloadParams (const QUrl& downloadUrl, const QString& destDir,
				int recLevel, bool fromOtherSite)
	: DownloadUrl_ (downloadUrl)
	, DestDir_ (destDir)
	, RecLevel_ (recLevel)
	, FromOtherSite_ (fromOtherSite)
	{
	}
	
	namespace
	{
		Entity GetEntity (const QUrl& url, const QString& filename)
		{
			return Util::MakeEntity (url,
					filename,
					LeechCraft::Internal |
						LeechCraft::DoNotNotifyUser |
						LeechCraft::DoNotSaveInHistory |
						LeechCraft::NotPersistent |
						LeechCraft::DoNotAnnounceEntity);
		}
	}

	OtzerkaluDownloader::FileData::FileData ()
	{
	}
	
	OtzerkaluDownloader::FileData::FileData (const QUrl& url, const QString& filename,
			int recLevel)
	: Url_ (url)
	, Filename_ (filename)
	, RecLevel_ (recLevel)
	{
	}

	OtzerkaluDownloader::OtzerkaluDownloader (const DownloadParams& param, QObject *parent)
	: QObject (parent)
	, Param_ (param)
	{
		UrlCount_ = 0;
	}
	
	void OtzerkaluDownloader::Begin ()
	{
		Download (Param_.DownloadUrl_);
	}
	
	void OtzerkaluDownloader::HandleProvider (QObject *provider, int id,
			const QUrl& url, const QString& filename, int recLevel)
	{
		FileMap_.insert (id, FileData (url, filename, recLevel));
		connect (provider,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)),
				Qt::UniqueConnection);
	}
	
	void OtzerkaluDownloader::handleJobFinished (int id)
	{
		--UrlCount_;
		const FileData& data = FileMap_ [id];
		if (!data.RecLevel_ || (!Param_.FromOtherSite_ &&
				data.Url_.host () != Param_.DownloadUrl_.host ()))
			return;

		const QString& filename = data.Filename_;

		QFile file (filename);

		if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
			return;

		QTextStream in (&file);
		const QString& array = in.readAll ();
		QWebPage page;

		page.mainFrame ()->setHtml (array, data.Url_);

		QWebElement document = page.mainFrame ()->documentElement ();
		QWebElementCollection uel = document.findAll ("*[href]") + document.findAll ("*[src]");

		for (QWebElementCollection::iterator urlElement = uel.begin ();
				urlElement != uel.end (); ++urlElement)
		{
			QUrl url = (*urlElement).attribute ("href");
			if (!url.isValid ())
				url = (*urlElement).attribute ("src");

			if (!url.isValid ())
				continue;

			const QString& filename = Download (url);
			if (filename.isEmpty ())
				continue;

			if ((*urlElement).hasAttribute ("href"))
				(*urlElement).setAttribute ("href", filename);
			else
				(*urlElement).setAttribute ("src", filename);
		}
		if (!UrlCount_)
			emit gotEntity (Util::MakeNotification (tr ("Download complete"),
					tr ("Download complete %1")
						.arg (data.Url_.toString ()),
					PInfo_));

		WriteData (filename, page.mainFrame ()->toHtml ());
	}
	
	QString OtzerkaluDownloader::Download (const QUrl& url)
	{
		const QFileInfo fi (url.path ());
		const QString& name = fi.fileName ();
		const QString& path = Param_.DestDir_ + '/' + url.host () +
				fi.path ();
		const QString& filename = path + '/' + (name.isEmpty () ? "index.html" : name);
		
		QDir::root ().mkpath (path);

		int id = -1;
		QObject *pr;
		emit delegateEntity (GetEntity (url, filename), &id, &pr);
		if (id == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not download"
					<< url
					<< "to"
					<< filename;
			emit gotEntity (Util::MakeNotification ("Otzerkalu",
					tr ("Could not download %1")
						.arg (url.toString ()),
					PCritical_));
			return QString ();
		}
		++UrlCount_;
		HandleProvider (pr, id, url, filename, Param_.RecLevel_ - 1);
		
		return filename;
	}
	
	bool OtzerkaluDownloader::WriteData(const QString& filename, const QString& data)
	{
		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Text))
			return false;

		QTextStream out (&file);
		out << data;
		return true;
	}
}
}
