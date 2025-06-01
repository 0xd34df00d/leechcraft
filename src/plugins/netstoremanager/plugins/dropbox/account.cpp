/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "account.h"
#include <algorithm>
#include <QtDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMainWindow>
#include <QPushButton>
#include <QFuture>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "core.h"
#include "uploadmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	Account::Account (const QString& name, QObject *parentPlugin)
	: QObject (parentPlugin)
	, ParentPlugin_ (parentPlugin)
	, Name_ (name)
	, Trusted_ (false)
	, DriveManager_ (new DriveManager (this, this))
	{
		connect (DriveManager_,
				SIGNAL (gotSharedFileUrl (QUrl, QDateTime)),
				this,
				SLOT (handleSharedFileUrl (QUrl, QDateTime)));
		connect (DriveManager_,
				SIGNAL (gotNewItem (DBoxItem)),
				this,
				SLOT (handleGotNewItem (DBoxItem)));

		if (UserID_.isEmpty ())
			DriveManager_->RequestUserId ();
	}

	QObject* Account::GetQObject ()
	{
		return this;
	}

	QObject* Account::GetParentPlugin () const
	{
		return ParentPlugin_;
	}

	QByteArray Account::GetUniqueID () const
	{
		return ("NetStoreManager.DBox_" + Name_).toUtf8 ();
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return FileListings;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	void Account::Upload (const QString& filepath, const QByteArray& parentId,
			UploadType ut, const QByteArray& id)
	{
		auto uploadManager = new UploadManager (filepath,
				ut, parentId, this, id);

		connect (uploadManager,
				SIGNAL (uploadProgress (quint64, quint64, QString)),
				this,
				SIGNAL (upProgress (quint64, quint64, QString)));
		connect (uploadManager,
				SIGNAL (uploadError (QString, QString)),
				this,
				SIGNAL (upError (QString, QString)));
		connect (uploadManager,
				SIGNAL (finished (QByteArray, QString)),
				this,
				SIGNAL (upFinished (QByteArray, QString)));
		connect (uploadManager,
				SIGNAL (uploadStatusChanged (QString, QString)),
				this,
				SIGNAL (upStatusChanged (QString, QString)));
	}

	void Account::Download (const QByteArray& id, const QString& filepath,
			TaskParameters tp, bool open)
	{
		if (id.isEmpty ())
			return;

		const auto& url = DriveManager_->GenerateDownloadUrl (id);
		emit downloadFile (url, filepath, tp, open);
	}

	ListingOps Account::GetListingOps () const
	{
		return ListingOp::Delete | ListingOp::DirectorySupport;
	}

	HashAlgorithm Account::GetCheckSumAlgorithm () const
	{
		return HashAlgorithm::Md5;
	}

	QFuture<Account::RefreshResult_t> Account::RefreshListing ()
	{
		return DriveManager_->RefreshListing ();
	}

	void Account::RefreshChildren (const QByteArray& parentId)
	{
		DriveManager_->RefreshListing (parentId);
	}

	namespace
	{
		static const QString PublicUrlTemplate { "https://dl.dropbox.com/u/%1/%2" };
	}

	QFuture<Account::RequestUrlResult_t> Account::RequestUrl (const QByteArray& id)
	{
		if (id.isNull ())
			return Util::MakeReadyFuture (RequestUrlResult_t { InvalidItem {} });

		const bool directLinkAvailable = id.startsWith ("/Public");
		if (directLinkAvailable)
		{
			const QUrl url { PublicUrlTemplate.arg (UserID_, QString { id }.remove ("/Public/")) };
			return Util::MakeReadyFuture (RequestUrlResult_t { url });
		}

		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		QMessageBox mbox (QMessageBox::Question,
				tr ("Share item"),
				tr ("Direct links available only for files in Public folder."
						" What type of link do you want?"),
				QMessageBox::Cancel,
				rootWM->GetPreferredWindow());

		QPushButton dropboxShareLink (tr ("DropBox share link"));
		QPushButton dropboxPreviewLink (tr ("DropBox preview link"));
		mbox.setDefaultButton (QMessageBox::Cancel);

		mbox.addButton (&dropboxShareLink, QMessageBox::YesRole);
		mbox.addButton (&dropboxPreviewLink, QMessageBox::AcceptRole);

		mbox.exec ();

		ShareType type;
		if (mbox.clickedButton () == &dropboxShareLink)
			type = ShareType::Share;
		else if (mbox.clickedButton () == &dropboxPreviewLink)
			type = ShareType::Preview;
		else
			return Util::MakeReadyFuture (RequestUrlResult_t { UserCancelled {} });

		return DriveManager_->ShareEntry (id, type).then (RequestUrlResult_t::EmbeddingLeft ());
	}

	void Account::CreateDirectory (const QString& name, const QByteArray& parentId)
	{
		if (name.isEmpty ())
			return;
		DriveManager_->CreateDirectory (name, parentId);
	}

	void Account::Delete (const QList<QByteArray>& ids, bool ask)
	{
		if (ids.isEmpty ())
			return;

		if (ask)
		{
			auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
			auto res = QMessageBox::warning (rootWM->GetPreferredWindow (),
					"LeechCraft",
					tr ("Are you sure you want to delete all selected items? This action cannot be undone."
						"<br><i>Note: if you delete a directory then all files in it will also be deleted.</i>"),
					QMessageBox::Ok | QMessageBox::Cancel);
			if (res != QMessageBox::Ok)
				return;
		}

		for (const auto& id : ids)
			DriveManager_->RemoveEntry (id);
	}

	void Account::Copy (const QList<QByteArray>& ids, const QByteArray& newParentId)
	{
		for (const auto& id : ids)
			DriveManager_->Copy (id, newParentId);
	}

	void Account::Move (const QList<QByteArray>& ids, const QByteArray& newParentId)
	{
		for (const auto& id : ids)
			DriveManager_->Move (id, newParentId);
	}

	void Account::MoveToTrash (const QList<QByteArray>&)
	{
	}

	void Account::RestoreFromTrash (const QList<QByteArray>&)
	{
	}

	void Account::Rename (const QByteArray&, const QString&)
	{
	}

	void Account::RequestChanges ()
	{
	}

	QByteArray Account::Serialize () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
				<< Name_
				<< Trusted_
				<< UserID_
				<< AccessToken_;
		return result;
	}

	Account_ptr Account::Deserialize (const QByteArray& data, QObject* parentPlugin)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return Account_ptr ();
		}

		QString name;
		str >> name;
		Account_ptr acc (new Account (name, parentPlugin));
		str >> acc->Trusted_
				>> acc->UserID_
				>> acc->AccessToken_;
		return acc;
	}

	bool Account::IsTrusted () const
	{
		return Trusted_;
	}

	void Account::SetTrusted (bool trust)
	{
		Trusted_ = trust;
	}

	void Account::SetAccessToken (const QString& token)
	{
		AccessToken_ = token;
	}

	QString Account::GetAccessToken () const
	{
		return AccessToken_;
	}

	void Account::SetUserID (const QString& uid)
	{
		UserID_ = uid;
	}

	DriveManager* Account::GetDriveManager () const
	{
		return DriveManager_;
	}

	void Account::handleGotNewItem (const DBoxItem& item)
	{
		const auto& storageItem = ToStorageItem (item);
		emit gotNewItem (storageItem, storageItem.ParentID_);
		emit listingUpdated (storageItem.ParentID_);
	}
}
}
}
