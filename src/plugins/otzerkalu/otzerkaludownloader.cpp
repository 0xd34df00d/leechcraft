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
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebPage>

namespace LeechCraft
{
namespace Otzerkalu
{
	DownloadParams::DownloadParams ()
	{
	}
	
	DownloadParams::DownloadParams (const QUrl& downloadUrl, const QString& destDir,
				const int recLevel, const bool fromOtherSite)
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
			return LeechCraft::Util::MakeEntity (url,
					filename,
					LeechCraft::Internal |
						LeechCraft::DoNotNotifyUser |
						LeechCraft::DoNotSaveInHistory |
						LeechCraft::NotPersistent |
						LeechCraft::DoNotAnnounceEntity);
		}
	}

	OtzerkaluDownloader::FileData::FileData()
	{
	}
	
	OtzerkaluDownloader::FileData::FileData(const QUrl& url, const QString& filename, const int recLevel)
	: Url_ (url)
	, Filename_ (filename)
	, RecLevel_ (recLevel)
	{
	}


	
	OtzerkaluDownloader::OtzerkaluDownloader (const DownloadParams& param, QObject *parent)
	: QObject (parent)
	, Param_ (param)
	{
		const QString& filename = Param_.DestDir_ + "/" + Param_.DownloadUrl_.host () +
				"/" + Param_.DownloadUrl_.path ();
		
		int id = -1;
		QObject *pr;
		emit delegateEntity (GetEntity (Param_.DownloadUrl_, filename), &id, &pr);
		if (id == -1)
		{
			qWarning () << Q_FUNC_INFO << "Otzerkalu: Could not download a file";
			emit gotEntity (Util::MakeNotification ("Otzerkalu",
					tr ("Could not download a file"),
					LeechCraft::PCritical_));
		}
		else
			HandleProvider (pr, id, Param_.DownloadUrl_, filename, Param_.RecLevel_);
	}
	
	void OtzerkaluDownloader::HandleProvider (QObject *provider, int id,
			const QUrl& url, const QString& filename, int recLevel)
	{
		FileMap_.insert (id, FileData (url, filename, recLevel));
		connect (provider,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)));
	}
	
	void OtzerkaluDownloader::handleJobFinished (int id)
	{
		const FileData& data = FileMap_ [id];
		if (!data.RecLevel_)
			return;
		
		const QString& filename = data.Filename_;
		const QUrl& tmpUrl = data.Url_;

		QWebPage page;
		page.mainFrame ()->load(filename);
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
			if (url.isRelative ())
				url = tmpUrl.resolved (url);
						
			QString urlVal = Param_.DestDir_ + "/" + url.host () + "/" + url.path ();
						
			int id = -1;
			QObject *pr;
			emit delegateEntity (GetEntity (url, urlVal), &id, &pr);
			if (id == -1)
			{
				qWarning () << Q_FUNC_INFO << "Otzerkalu: Could not download a file";
				emit gotEntity (Util::MakeNotification ("Otzerkalu",
						tr ("Could not download a file"),
						PCritical_));
				continue;
			}
				
			HandleProvider (pr, id, url, urlVal, data.RecLevel_ - 1);
			if ((*urlElement).hasAttribute ("href"))
				(*urlElement).setAttribute ("href", urlVal);
			else
				(*urlElement).setAttribute ("src", urlVal);

		}
				
		WriteData (filename, page.mainFrame ()->toHtml ());
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

};
};