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

#include "webfilestorageadaptor.h"
#include <QUrl>
#include <interfaces/iwebfilestorage.h>
#include "core.h"

namespace LeechCraft
{
namespace DBusManager
{
	WebFileStorageAdaptor::WebFileStorageAdaptor (QObject *parent)
	: QDBusAbstractAdaptor (parent)
	, WFS_ (qobject_cast<IWebFileStorage*> (parent))
	{
		setAutoRelaySignals (false);
		connect (parent,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)));
	}

	QStringList WebFileStorageAdaptor::GetServiceVariants () const
	{
		return WFS_->GetServiceVariants ();
	}

	void WebFileStorageAdaptor::UploadFile (const QString& filename, const QString& service)
	{
		WFS_->UploadFile (filename, service);
	}

	void WebFileStorageAdaptor::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		emit FileUploaded (filename, url.toString ());
	}
}
}
