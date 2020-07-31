/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendinguploadbase.h"
#include <memory>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QFile>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sys/mimedetector.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include <interfaces/ijobholder.h>

namespace LC
{
namespace Zalil
{
	PendingUploadBase::PendingUploadBase (const QString& filename,
			const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, ProgressRow_
	{
		new QStandardItem { tr ("Uploading %1")
			.arg (QFileInfo { filename }.fileName ()) },
		new QStandardItem { tr ("Uploading...") },
		new QStandardItem
	}
	, ProgressRowGuard_
	{
		[this]
		{
			if (const auto model = ProgressRow_.value (0)->model ())
				model->removeRow (ProgressRow_.value (0)->row ());
		}
	}
	, Filename_ { filename }
	, Proxy_ { proxy }
	{
		for (const auto item : ProgressRow_)
		{
			item->setEditable (false);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);
		}
	}

	const QList<QStandardItem*>& PendingUploadBase::GetReprRow () const
	{
		return ProgressRow_;
	}

	QHttpMultiPart* PendingUploadBase::MakeStandardMultipart ()
	{
		auto fileDev = std::make_unique<QFile> (Filename_, this);
		if (!fileDev->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< Filename_
					<< fileDev->errorString ();
			return nullptr;
		}

		const auto multipart = new QHttpMultiPart { QHttpMultiPart::FormDataType, this };

		QHttpPart textPart;
		const QFileInfo fi { Filename_ };
		textPart.setHeader (QNetworkRequest::ContentDispositionHeader,
				"form-data; name=\"file\"; filename=\"" + fi.fileName () + "\"");
		textPart.setHeader (QNetworkRequest::ContentTypeHeader, Util::MimeDetector {} (Filename_));

		textPart.setBodyDevice (fileDev.release ());

		multipart->append (textPart);

		return multipart;
	}

	void PendingUploadBase::handleUploadProgress (qint64 done, qint64 total)
	{
		Util::SetJobHolderProgress (ProgressRow_, done, total,
				tr ("%1 of %2")
					.arg (Util::MakePrettySize (done))
					.arg (Util::MakePrettySize (total)));
	}

	void PendingUploadBase::handleError ()
	{
		deleteLater ();

		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->error ()
				<< reply->errorString ();
	}
}
}
