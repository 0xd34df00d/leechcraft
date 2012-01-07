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

#include "upmanager.h"
#include <QApplication>
#include <QClipboard>
#include <interfaces/structures.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	UpManager::UpManager (QObject *parent)
	: QObject (parent)
	{
	}

	void UpManager::handleUploadRequest (IStorageAccount *acc, const QString& path)
	{
		if (!Uploads_.contains (acc))
		{
			QObject *accObj = acc->GetObject ();
			connect (accObj,
					SIGNAL (gotURL (QUrl, QString)),
					this,
					SLOT (handleGotURL (QUrl, QString)));
		}
		else if (Uploads_ [acc].contains (path))
		{
			const Entity& e = Util::MakeNotification ("NetStoreManager",
					tr ("%1 is already uploading to %2.")
						.arg (QFileInfo (path).fileName ())
						.arg (acc->GetAccountName ()),
					PWarning_);
			emit gotEntity (e);
			return;
		}

		acc->Upload (path);
	}

	void UpManager::handleGotURL (const QUrl& url, const QString& path)
	{
		const QString& urlStr = url.toString ();
		qApp->clipboard ()->setText (urlStr, QClipboard::Clipboard);
		qApp->clipboard ()->setText (urlStr, QClipboard::Selection);

		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		Uploads_ [acc].removeAll (path);

		auto *plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());

		const Entity& e = Util::MakeNotification ("NetStoreManager",
				tr ("%1 is successfully uploaded to %2, URL is pasted into clipboard.")
					.arg (QFileInfo (path).fileName ())
					.arg (plugin->GetStorageName ()),
				PInfo_);
		emit gotEntity (e);
	}
}
}
