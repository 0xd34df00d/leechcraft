/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
