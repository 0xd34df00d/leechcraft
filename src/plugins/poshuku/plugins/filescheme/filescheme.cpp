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

#include "filescheme.h"
#include <typeinfo>
#include <QIcon>
#include <util/util.h>
#include "schemereply.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace FileScheme
{
	void FileScheme::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku_filescheme"));
	}

	void FileScheme::SecondInit ()
	{
	}

	void FileScheme::Release ()
	{
	}

	QByteArray FileScheme::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FileScheme";
	}

	QString FileScheme::GetName () const
	{
		return "Poshuku FileScheme";
	}

	QString FileScheme::GetInfo () const
	{
		return tr ("Provides support for file:// scheme.");
	}

	QIcon FileScheme::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/filescheme/resources/images/poshuku_filescheme.svg");
	}

	QStringList FileScheme::Provides () const
	{
		return QStringList ("file://");
	}

	QStringList FileScheme::Needs () const
	{
		return QStringList ();
	}

	QStringList FileScheme::Uses () const
	{
		return QStringList ();
	}

	void FileScheme::SetProvider (QObject*, const QString&)
	{
	}

	QSet<QByteArray> FileScheme::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void FileScheme::hookNAMCreateRequest (IHookProxy_ptr proxy,
			QNetworkAccessManager*,
			QNetworkAccessManager::Operation *op,
			QIODevice**)
	{
		if (*op != QNetworkAccessManager::GetOperation)
			return;

		const QNetworkRequest& req = proxy->GetValue ("request").value<QNetworkRequest> ();
		const QUrl& url = req.url ();
		if (url.scheme () != "file" ||
				!QFileInfo (url.toLocalFile ()).isDir ())
			return;

		proxy->CancelDefault ();
		proxy->SetReturnValue (QVariant::fromValue<QNetworkReply*> (new SchemeReply (req, this)));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_filescheme,
		LeechCraft::Poshuku::FileScheme::FileScheme);

