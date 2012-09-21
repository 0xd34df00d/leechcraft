/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "core.h"
#include <util/util.h>
#include <QDesktopServices>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	Core::Core ()
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const LeechCraft::Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::DelegateEntity (const LeechCraft::Entity& e, const QString& targetPath)
	{
		int id = -1;
		QObject *pr;
		emit delegateEntity (e, &id, &pr);
		if (id == -1)
		{
			Entity notif = Util::MakeNotification (tr ("Import error"),
					tr ("Could not find plugin to download %1.")
							.arg (e.Entity_.toString ()),
					PCritical_);
			notif.Additional_ ["UntilUserSees"] = true;
			emit gotEntity (notif);
			return;
		}
		Id2SavePath_ [id] = targetPath;
		HandleProvider (pr, id);
	}

	void Core::HandleProvider (QObject *provider, int id)
	{
		if (Downloaders_.contains (provider))
			return;

		Downloaders_ << provider;
		connect (provider,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)));
		connect (provider,
				SIGNAL (jobRemoved (int)),
				this,
				SLOT (handleJobRemoved (int)));
		connect (provider,
				SIGNAL (jobError (int, IDownload::Error)),
				this,
				SLOT (handleJobError (int, IDownload::Error)));

		Id2Downloader_ [id] = provider;
	}

	void Core::handleJobFinished (int id)
	{
		QString path = Id2SavePath_.take (id);
		Id2Downloader_.remove (id);
		QFile::rename (QDesktopServices::storageLocation (QDesktopServices::TempLocation) +
				"/" + QFileInfo (path).fileName (),
				path);
	}

	void Core::handleJobRemoved (int id)
	{
		Id2Downloader_.remove (id);
		Id2SavePath_.remove (id);
	}

	void Core::handleJobError (int id, IDownload::Error err)
	{
		Id2Downloader_.remove (id);
		Id2SavePath_.remove (id);
	}

}
}
}
