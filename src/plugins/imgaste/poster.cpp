/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "poster.h"
#include <QStandardItemModel>
#include <QNetworkReply>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include <util/network/handlenetworkreply.h>

namespace LC::Imgaste
{
	Poster::Poster (HostingService service,
			const QByteArray& data,
			const QString& format,
			ICoreProxy_ptr proxy,
			QStandardItemModel *reprModel,
			QObject *parent)
	: QObject (parent)
	, Worker_ (MakeWorker (service))
	, Proxy_ (proxy)
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
							.arg (Util::MakePrettySize (done))
							.arg (Util::MakePrettySize (total)));
		};
		setUploadProgress (0, data.size ());

		const auto reply = Worker_->Post (data, format, proxy->GetNetworkAccessManager ());
		connect (reply,
				&QNetworkReply::uploadProgress,
				this,
				setUploadProgress);

		Util::HandleReplySeq<Util::ErrorInfo<Util::ReplyError>, Util::ResultInfo<Util::ReplyWithHeaders>> (reply, this) >>
				Util::Visitor
				{
					[this, url = reply->request ().url ()] (Util::ReplyError reply)
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
					[this] (Util::ReplyWithHeaders reply)
					{
						Util::ReportFutureResult (Promise_, Result_t::LeftLift (Worker_->GetLink (reply.Data_, reply.Headers_)));
					}
				}.Finally ([this, reprModel, reprRow]
						{
							deleteLater ();
							reprModel->removeRow (reprRow.first ()->row ());
						});
	}

	QFuture<Poster::Result_t> Poster::GetFuture ()
	{
		return Promise_.future ();
	}
}
