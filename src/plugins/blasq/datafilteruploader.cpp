/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "datafilteruploader.h"
#include <QtDebug>
#include <QTemporaryFile>
#include <QUrl>
#include <QInputDialog>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <interfaces/structures.h>
#include <interfaces/idatafilter.h>
#include "accountsmanager.h"
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/isupportuploads.h"
#include "uploadphotosdialog.h"

namespace LC
{
namespace Blasq
{
	DataFilterUploader::DataFilterUploader (const Entity& e, AccountsManager *accMgr, QObject *parent)
	: QObject { parent }
	, AccMgr_ { accMgr }
	, Entity_ { e }
	{
		const auto& accId = e.Additional_ ["DataFilter"].toByteArray ();

		if (!accId.isEmpty ())
			UploadToAcc (accId);
		else
			SelectAcc ();
	}

	void DataFilterUploader::SelectAcc ()
	{
		const auto& accs = AccMgr_->GetAccounts ();
		const auto& accNames = Util::Map (accs, &IAccount::GetName);

		bool ok = false;
		const auto& chosenAcc = QInputDialog::getItem (nullptr,
				tr ("Select account"),
				tr ("Please select the account to use while uploading the photo:"),
				accNames,
				0,
				false,
				&ok);

		if (!ok)
		{
			deleteLater ();
			return;
		}

		const auto acc = accs.value (accNames.indexOf (chosenAcc));
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "no account for account name"
					<< chosenAcc;
			deleteLater ();
			return;
		}

		UploadToAcc (acc->GetID ());
	}

	void DataFilterUploader::UploadToAcc (const QByteArray& accId)
	{
		const auto acc = AccMgr_->GetAccount (accId);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "no account for ID"
					<< accId;
			deleteLater ();
			return;
		}

		const auto& image = Entity_.Entity_.value<QImage> ();
		const auto& localFile = Entity_.Entity_.toUrl ().toLocalFile ();
		if (!image.isNull ())
		{
			auto tempFile = new QTemporaryFile { this };
			Entity_.Entity_.value<QImage> ().save (tempFile, "PNG", 0);
			UploadFileName_ = tempFile->fileName ();
		}
		else if (QFile::exists (localFile))
			UploadFileName_ = localFile;

		const auto dia = new UploadPhotosDialog { acc->GetQObject () };
		dia->LockFiles ();
		dia->SetFiles ({ { UploadFileName_, {} } });
		dia->open ();
		dia->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, dia, acc]
			{
				connect (acc->GetQObject (),
						SIGNAL (itemUploaded (UploadItem, QUrl)),
						this,
						SLOT (checkItemUploaded (UploadItem, QUrl)));

				const auto isu = qobject_cast<ISupportUploads*> (acc->GetQObject ());
				isu->UploadImages (dia->GetSelectedCollection (), dia->GetSelectedFiles ());
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};

		connect (dia,
				SIGNAL (rejected ()),
				this,
				SLOT (deleteLater ()));
	}

	void DataFilterUploader::checkItemUploaded (const UploadItem& item, const QUrl& url)
	{
		if (item.FilePath_ != UploadFileName_)
			return;

		if (const auto cb = Entity_.Additional_ ["DataFilterCallback"].value<DataFilterCallback_f> ())
			cb (url);

		deleteLater ();
	}
}
}
