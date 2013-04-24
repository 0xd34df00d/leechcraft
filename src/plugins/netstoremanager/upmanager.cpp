/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QStandardItemModel>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "xmlsettingsmanager.h"

inline uint qHash (const QStringList& id)
{
	return qHash (id.join ("/"));
}

namespace LeechCraft
{
namespace NetStoreManager
{
	UpManager::UpManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, ReprModel_ (new QStandardItemModel (0, 3, this))
	, Proxy_ (proxy)
	{
	}

	QAbstractItemModel* UpManager::GetRepresentationModel () const
	{
		return ReprModel_;
	}

	void UpManager::RemovePending (const QString& path)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		Uploads_ [acc].removeAll (path);

		auto items = ReprItems_ [acc].take (path);
		if (items.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty items list for"
					<< path;
			return;
		}
		ReprModel_->removeRow (items.first ()->row ());
	}

	IStoragePlugin* UpManager::GetSenderPlugin ()
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		return qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
	}

	void UpManager::ScheduleAutoshare (const QString& path)
	{
		Autoshare_ << path;
	}

	void UpManager::handleUploadRequest (IStorageAccount *acc, const QString& path,
			const QByteArray& id, bool byHand)
	{
		if (!Uploads_.contains (acc))
		{
			QObject *accObj = acc->GetQObject ();
			connect (accObj,
					SIGNAL (upError (QString, QString)),
					this,
					SLOT (handleError (QString, QString)));
			connect (accObj,
					SIGNAL (upStatusChanged (QString, QString)),
					this,
					SLOT (handleUpStatusChanged (QString, QString)));
			connect (accObj,
					SIGNAL (upFinished (QByteArray, QString)),
					this,
					SLOT (handleUpFinished (QByteArray, QString)));
			connect (accObj,
					SIGNAL (upProgress (quint64, quint64, QString)),
					this,
					SLOT (handleUpProgress (quint64, quint64, QString)));

			if (qobject_cast<ISupportFileListings*> (accObj))
				connect (accObj,
						SIGNAL (gotFileUrl (QUrl, QByteArray)),
						this,
						SLOT (handleGotURL (QUrl, QByteArray)));
		}
		else if (Uploads_ [acc].contains (path))
		{
			const Entity& e = Util::MakeNotification ("NetStoreManager",
					tr ("%1 is already uploading to %2.")
						.arg (QFileInfo (path).fileName ())
						.arg (acc->GetAccountName ()),
					PWarning_);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return;
		}

		acc->Upload (path, id);

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());

		QList<QStandardItem*> row;
		row << new QStandardItem (tr ("Uploading %1 to %2...")
					.arg (QFileInfo (path).fileName ())
					.arg (plugin->GetStorageName ()));
		row << new QStandardItem ();
		row << new QStandardItem (tr ("Initializing..."));
		row.last ()->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
				CustomDataRoles::RoleJobHolderRow);
		ReprModel_->appendRow (row);

		ReprItems_ [acc] [path] = row;

		if (byHand &&
				XmlSettingsManager::Instance ().Property ("CopyUrlOnUpload", false).toBool ())
			ScheduleAutoshare (path);
	}

	void UpManager::handleGotURL (const QUrl& url, const QByteArray& id)
	{
		const auto& handlers = URLHandlers_.take (id);
		if (!handlers.isEmpty ())
		{
			Q_FOREACH (auto handler, handlers)
				handler (url, id);

			return;
		}

		const QString& urlStr = url.toString ();
		qApp->clipboard ()->setText (urlStr, QClipboard::Clipboard);
		qApp->clipboard ()->setText (urlStr, QClipboard::Selection);

		auto plugin = GetSenderPlugin ();
		const Entity& e = Util::MakeNotification (plugin->GetStorageName (),
				tr ("URL is pasted into clipboard."),
				PInfo_);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void UpManager::handleError (const QString& str, const QString& path)
	{
		qWarning () << Q_FUNC_INFO << str << path;

		RemovePending (path);

		auto plugin = GetSenderPlugin ();
		const Entity& e = Util::MakeNotification (plugin->GetStorageName (),
				tr ("Failed to upload %1: %2.")
					.arg (path)
					.arg (str),
				PWarning_);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void UpManager::handleUpStatusChanged (const QString& status, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;
		list [1]->setText (status);
	}

	void UpManager::handleUpFinished (const QByteArray& id, const QString& filePath)
	{
		RemovePending (filePath);
		const auto& e =Util::MakeNotification ("NetStoreManager",
				tr ("File %1 was uploaded successfully")
						.arg ("<em>" + QFileInfo (filePath).fileName () + "</em>"),
				PWarning_);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		if (Autoshare_.remove (filePath))
		{
			auto ifl = qobject_cast<ISupportFileListings*> (sender ());
			if (!ifl)
			{
				qWarning () << Q_FUNC_INFO
						<< "account doesn't support file listings, cannot autoshare";
				return;
			}

			URLHandlers_ [id] << [this, filePath] (const QUrl& url, const QByteArray&)
			{
				emit fileUploaded (filePath, url);
			};

			ifl->RequestUrl (id);
		}
	}

	void UpManager::handleUpProgress (quint64 done, quint64 total, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;

		auto item = list.at (2);
		item->setText (tr ("%1 of %2")
				.arg (Util::MakePrettySize (done))
				.arg (Util::MakePrettySize (total)));
		item->setData (done, ProcessState::Done);
		item->setData (total, ProcessState::Total);
	}
}
}
