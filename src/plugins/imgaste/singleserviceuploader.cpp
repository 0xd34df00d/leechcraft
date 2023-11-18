/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "singleserviceuploader.h"
#include <QStandardItemModel>
#include <QNetworkReply>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include <util/network/handlenetworkreply.h>

namespace LC::Imgaste
{
	SingleServiceUploader::SingleServiceUploader (const HostingService& service,
			const QByteArray& data,
			Format format,
			QStandardItemModel *reprModel,
			QObject *parent)
	: QObject { parent }
	, Service_ { service }
	{
		Promise_.reportStarted ();

		const QList<QStandardItem*> reprRow
		{
			new QStandardItem { tr ("Image upload") },
			new QStandardItem { tr ("Uploading...") },
			new QStandardItem
		};
		for (const auto item : reprRow)
		{
			item->setEditable (false);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);
		}
		reprModel->appendRow (reprRow);

		auto setUploadProgress = [reprRow] (qint64 done, qint64 total)
		{
			Util::SetJobHolderProgress (reprRow, done, total,
					tr ("%1 of %2")
							.arg (Util::MakePrettySize (done), Util::MakePrettySize (total)));
		};
		setUploadProgress (0, data.size ());

		const auto reply = Service_.Post (data, format, GetProxyHolder ()->GetNetworkAccessManager ());
		connect (reply,
				&QNetworkReply::uploadProgress,
				this,
				setUploadProgress);

		Util::HandleReplySeq<Util::ErrorInfo<Util::ReplyError>, Util::ResultInfo<Util::ReplyWithHeaders>> (reply, this) >>
				Util::Visitor
				{
					[this, url = reply->request ().url ()] (const Util::ReplyError& reply)
					{
						Util::ReportFutureResult (Promise_,
								NetworkRequestError
								{
									url,
									reply.Error_,
									!reply.HttpStatusCode_.isNull () && reply.HttpStatusCode_.canConvert<int> () ?
											std::optional<int> { reply.HttpStatusCode_.toInt () } :
											std::optional<int> {},
									reply.ErrorString_
								});
					},
					[this] (const Util::ReplyWithHeaders& reply)
					{
						Util::ReportFutureResult (Promise_, Result_t::LeftLift (Service_.GetLink (reply.Data_, reply.Headers_)));
					}
				}.Finally ([this, reprModel, reprRow]
						{
							deleteLater ();
							reprModel->removeRow (reprRow.first ()->row ());
						});
	}

	QFuture<SingleServiceUploader::Result_t> SingleServiceUploader::GetFuture ()
	{
		return Promise_.future ();
	}
}
