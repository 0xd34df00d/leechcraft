/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Georg Rudoy
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

#include "otzerkalu.h"
#include <QIcon>

#include "otzerkaludialog.h"
#include "otzerkaludownloader.h"

namespace LeechCraft
{
namespace Otzerkalu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otzerkalu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otzerkalu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Otzerkalu allows to recursively download a web site.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	bool Plugin::CouldHandle (const LeechCraft::Entity& entity) const
	{
		return entity.Mime_ == "x-leechcraft/otzerkalu-url"
			&& entity.Entity_.canConvert<QUrl> ();
	}

	void Plugin::Handle (LeechCraft::Entity entity)
	{
		QUrl dUrl = qvariant_cast<QUrl> (entity.Entity_);
		
		OtzerkaluDialog dialog;
		dialog.exec ();
		if (dialog.exec () != QDialog::Accepted)
			return;
		OtzerkaluDownloader downloader (DownloadParams (dUrl, dialog.GetDir (), dialog.GetRecursionLevel (), dialog.IsFromOtherSite ()), this);
		
		connect (&downloader, SIGNAL (donwloadCompleted ()), this, SLOT (downloadCompleted ()));
	}
	
	void Plugin::downloadCompleted ()
	{
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_otzerkalu, LeechCraft::Otzerkalu::Plugin);

