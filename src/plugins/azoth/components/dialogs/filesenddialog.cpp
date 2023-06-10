/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filesenddialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iwebfilestorage.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/itransfermanager.h"
#include "core.h"
#include "chattabsmanager.h"
#include "transferjobmanager.h"
#include "pendinguploadpaster.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	FileSendDialog::FileSendDialog (ICLEntry *entry, const QString& suggested, QWidget *parent)
	: QDialog (parent)
	, Entry_ (entry)
	, EntryVariant_ (Core::Instance ().GetChatTabsManager ()->GetActiveVariant (entry))
	, AccSupportsFT_ (false)
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (send ()));
		show ();

		auto name = entry->GetEntryName ();
		if (name != entry->GetHumanReadableID ())
			name += " (" + entry->GetHumanReadableID () + ")";
		Ui_.TargetLabel_->setText (name);

		auto acc = Entry_->GetParentAccount ();
		auto itm = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (itm && itm->IsAvailable ())
		{
			AccSupportsFT_ = true;
			Ui_.TransferMethod_->addItem (tr ("Protocol file transfer"));
		}

		FillSharers ();

		if (!Ui_.TransferMethod_->count ())
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("There aren't any file transfer methods available. "
						"Please either make sure protocol file transfers are enabled and "
						"active, or install a module like NetStoreManager."));
			deleteLater ();
			return;
		}

		if (suggested.isEmpty ())
		{
			if (XmlSettingsManager::Instance ().property ("AutoOpenFileDialogOnSend").toBool ())
				on_FileBrowse__released ();
		}
		else
			Ui_.FileEdit_->setText (suggested);
	}

	void FileSendDialog::FillSharers ()
	{
		const auto& sharers = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ();
		QMap<QString, QObject*> variants;
		for (auto sharerObj : sharers)
		{
			auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
			for (const auto& var : sharer->GetServiceVariants ())
			{
				const int idx = Ui_.TransferMethod_->count ();
				Ui_.TransferMethod_->addItem (var);
				Pos2Sharer_ [idx] = { sharerObj, var };
			}
		}
	}

	void FileSendDialog::SendSharer (const SharerInfo& info)
	{
		const auto& filename = Ui_.FileEdit_->text ();

		auto sharer = qobject_cast<IWebFileStorage*> (info.Sharer_);
		sharer->UploadFile (filename, info.Service_);
		new PendingUploadPaster (info.Sharer_, Entry_, EntryVariant_, filename);
	}

	void FileSendDialog::SendProto ()
	{
		auto acc = Entry_->GetParentAccount ();
		auto xferMgr = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!xferMgr)
		{
			qWarning () << Q_FUNC_INFO
					<< "null Xfer manager for"
					<< Entry_->GetQObject ();
			return;
		}

		const auto& filename = Ui_.FileEdit_->text ();
		if (filename.isEmpty ())
			return;

		QObject *job = xferMgr->SendFile (Entry_->GetEntryID (),
				EntryVariant_, filename, Ui_.CommentEdit_->toPlainText ());
		if (!job)
		{
			const auto& e = Util::MakeNotification ("Azoth",
						tr ("Unable to send file to %1.")
							.arg (Entry_->GetEntryName ()),
						Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			return;
		}
		Core::Instance ().GetTransferJobManager ()->HandleJob (job);
	}

	void FileSendDialog::send ()
	{
		const int idx = Ui_.TransferMethod_->currentIndex ();
		if (Pos2Sharer_.contains (idx))
			SendSharer (Pos2Sharer_ [idx]);
		else if (AccSupportsFT_)
			SendProto ();
		else
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("No file transfer methods available."));
	}

	void FileSendDialog::on_FileBrowse__released ()
	{
		const auto& filename = QFileDialog::getOpenFileName (0,
				tr ("Select file to send"),
				XmlSettingsManager::Instance ()
						.Property ("LastFileSendDir", QDir::homePath ()).toString ());
		if (filename.isEmpty ())
			return;

		Ui_.FileEdit_->setText (filename);

		const auto& dir = QFileInfo { filename }.absolutePath ();
		XmlSettingsManager::Instance ().setProperty ("LastFileSendDir", dir);
	}
}
}
