/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upmanager.h"
#include <QApplication>
#include <QClipboard>
#include <QStandardItemModel>
#include <QFileInfo>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/xpc/progressmanager.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "xmlsettingsmanager.h"
#include "utils.h"

namespace LC
{
namespace NetStoreManager
{
	UpManager::UpManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Progress_ (new Util::ProgressManager (this))
	, Proxy_ (proxy)
	{
	}

	IJobHolderRepresentationHandler_ptr UpManager::CreateReprHandler ()
	{
		return Progress_->CreateDefaultHandler ();
	}

	void UpManager::RemovePending (const QString& path)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		Uploads_ [acc].removeAll (path);
		ReprItems_ [acc].remove (path);
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
		}
		else if (Uploads_ [acc].contains (path))
		{
			const Entity& e = Util::MakeNotification ("NetStoreManager",
					tr ("%1 is already uploading to %2.")
						.arg (QFileInfo (path).fileName ())
						.arg (acc->GetAccountName ()),
					Priority::Warning);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return;
		}

		acc->Upload (path, id);

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());

		const QFileInfo fi (path);
		auto row = Progress_->AddRow ({
					.Name_ = tr ("Uploading %1 to %2...").arg (fi.fileName (), plugin->GetStorageName ()),
					.Specific_ = ProcessInfo
					{
						.Parameters_ = byHand ? FromUserInitiated : NoParameters,
						.Kind_ = ProcessKind::Upload,
					}
				},
				{ .Total_ = fi.size (), .CustomStateText_ = tr ("Initializing...") });
		ReprItems_ [acc] [path] = std::move (row);

		if (byHand &&
				XmlSettingsManager::Instance ().Property ("CopyUrlOnUpload", false).toBool ())
			ScheduleAutoshare (path);
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
				Priority::Warning);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void UpManager::handleUpStatusChanged (const QString& status, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		if (const auto& row = ReprItems_ [acc] [filepath])
			row->SetState (ProcessState::Running, status);
	}

	void UpManager::handleUpFinished (const QByteArray& id, const QString& filePath)
	{
		RemovePending (filePath);
		const auto& fileName = QFileInfo { filePath }.fileName ();
		const auto& e = Util::MakeNotification ("NetStoreManager",
				tr ("File %1 was uploaded successfully")
						.arg ("<em>" + fileName + "</em>"),
				Priority::Info);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		if (!Autoshare_.remove (filePath))
			return;

		auto ifl = qobject_cast<ISupportFileListings*> (sender ());
		if (!ifl)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't support file listings, cannot autoshare";
			return;
		}

		Util::Sequence (this, ifl->RequestUrl (id)) >>
				Utils::HandleRequestFileUrlResult (Proxy_->GetEntityManager (),
						tr ("Failed to auto-share file %1.").arg ("<em>" + fileName + "</em>"),
						[=, this] (const QUrl& url) { emit fileUploaded (filePath, url); });
	}

	void UpManager::handleUpProgress (quint64 done, quint64 total, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		if (const auto& row = ReprItems_ [acc] [filepath])
		{
			row->SetDone (done);
			row->SetTotal (total);
		}
	}
}
}
