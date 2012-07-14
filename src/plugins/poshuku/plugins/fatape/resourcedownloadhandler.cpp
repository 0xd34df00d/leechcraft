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

#include "resourcedownloadhandler.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QNetworkRequest>
#include <QSettings>
#include "userscript.h"



namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	ResourceDownloadHandler::ResourceDownloadHandler (const QString& resourceName, 
			UserScript *script, QNetworkReply *reply)
	: ResourceName_ (resourceName)
	, Script_ (script)
	, Reply_ (reply)
	{}

	void ResourceDownloadHandler::handleFinished ()
	{
		QFile resource (Script_->GetResourcePath (ResourceName_));
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");


		if (!resource.open (QFile::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to save resource"
				<< ResourceName_
				<< "from"
				<< Reply_->url ().toString ();
			return;
		}
		resource.write (Reply_->readAll ());

		settings.setValue (QString ("resources/%1/%2/%3")
				.arg (qHash (Script_->Namespace ()))
				.arg (Script_->Name ())
				.arg (ResourceName_), 
				Reply_->header (QNetworkRequest::ContentTypeHeader));
		Reply_->deleteLater ();
		deleteLater ();
	}
}
}
}

