/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requestmodel.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QTextCodec>
#include <QDateTime>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "headermodel.h"

using GuardedReply_t = QPointer<QNetworkReply>;
Q_DECLARE_METATYPE (GuardedReply_t)

namespace LC
{
namespace Plugins
{
namespace NetworkMonitor
{
RequestModel::RequestModel (QObject *parent)
: QStandardItemModel { parent }
, RequestHeadersModel_ { new HeaderModel { this } }
, ReplyHeadersModel_ { new HeaderModel { this } }
{
	setHorizontalHeaderLabels ({
			tr ("Date started"),
			tr ("Date finished"),
			tr ("Type"),
			tr ("Host")
		});
}

HeaderModel* RequestModel::GetRequestHeadersModel () const
{
	return RequestHeadersModel_;
}

HeaderModel* RequestModel::GetReplyHeadersModel () const
{
	return ReplyHeadersModel_;
}

void RequestModel::handleRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QNetworkReply *rep)
{
	if (rep->isFinished ())
	{
		qWarning () << Q_FUNC_INFO
			<< "skipping the finished reply"
			<< rep;
		return;
	}
	QList<QStandardItem*> items;
	QString opName;
	switch (op)
	{
		case QNetworkAccessManager::HeadOperation:
			opName = "HEAD";
			break;
		case QNetworkAccessManager::GetOperation:
			opName = "GET";
			break;
		case QNetworkAccessManager::PutOperation:
			opName = "PUT";
			break;
		case QNetworkAccessManager::PostOperation:
			opName = "POST";
			break;
		case QNetworkAccessManager::DeleteOperation:
			opName = "DELETE";
			break;
		case QNetworkAccessManager::UnknownOperation:
			opName = "Unknown";
			break;
		case QNetworkAccessManager::CustomOperation:
			opName = "Custom";
			break;
	}
	items.push_back (new QStandardItem (QDateTime::currentDateTime ().toString ()));
	items.push_back (new QStandardItem (tr ("In progress")));
	items.push_back (new QStandardItem (opName));
	items.push_back (new QStandardItem (req.url ().toString ()));
	items.first ()->setData (QVariant::fromValue<GuardedReply_t> (rep));
	appendRow (items);

	connect (rep,
			SIGNAL (error (QNetworkReply::NetworkError )),
			this,
			SLOT (handleFinished ()));
	connect (rep,
			SIGNAL (finished ()),
			this,
			SLOT (handleFinished ()));
	connect (rep,
			SIGNAL (destroyed (QObject*)),
			this,
			SLOT (handleGonnaDestroy (QObject*)));
}

namespace
{
	template<typename T>
	QMap<QString, QVariant> GetHeaders (const T *object)
	{
		QMap<QString, QVariant> result;
		const auto codec = QTextCodec::codecForName ("UTF-8");
		for (const auto& header : object->rawHeaderList ())
			result [codec->toUnicode (header)] = codec->toUnicode (object->rawHeader (header));
		return result;
	}

	template<typename T>
	void FeedHeaders (const T& object, HeaderModel *model)
	{
		FeedHeaders (GetHeaders (object), model);
	}

	template<>
	void FeedHeaders (const QMap<QString, QVariant>& headers, HeaderModel *model)
	{
		for (const auto& pair : Util::Stlize (headers))
			model->AddHeader (pair.first, pair.second.toString ());
	}
}

void RequestModel::handleFinished ()
{
	const auto reply = qobject_cast<QNetworkReply*> (sender ());
	if (!reply)
	{
		qWarning () << Q_FUNC_INFO
			<< sender ()
			<< "not found";
		return;
	}

	for (int i = 0; i < rowCount (); ++i)
	{
		if (item (i)->data ().value<GuardedReply_t> () != reply)
			continue;

		if (Clear_)
		{
			removeRow (i);
			break;
		}

		item (i, 0)->setData ({});
		item (i, 1)->setText (QDateTime::currentDateTime ().toString ());

		const auto& r = reply->request ();
		item (i, 1)->setData (GetHeaders (&r));

		auto headers = GetHeaders (reply);
		headers ["[HTTP response]"] = QString ("%1 (%2)")
				.arg (reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ())
				.arg (reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ());
		item (i, 2)->setData (headers);
		break;
	}
}

void RequestModel::setClear (bool clear)
{
	Clear_ = clear;

	if (Clear_)
	{
		for (int i = rowCount () - 1; i >= 0; --i)
			if (!item (i)->data ().value<GuardedReply_t> ())
				removeRow (i);
		handleCurrentChanged ({});
	}
}

void RequestModel::handleCurrentChanged (const QModelIndex& newItem)
{
	RequestHeadersModel_->clear ();
	ReplyHeadersModel_->clear ();

	if (!newItem.isValid ())
		return;

	const auto reply = item (newItem.row (), 0)->data ().value<GuardedReply_t> ();
	if (reply)
	{
		const auto& r = reply->request ();
		FeedHeaders (&r, RequestHeadersModel_);
		FeedHeaders (reply.data (), ReplyHeadersModel_);
	}
	else
	{
		FeedHeaders (item (newItem.row (), 1)->
				data ().toMap (), RequestHeadersModel_);
		FeedHeaders (item (newItem.row (), 2)->
				data ().toMap (), ReplyHeadersModel_);
	}
}

void RequestModel::handleGonnaDestroy (QObject *obj)
{
	for (int i = 0; i < rowCount (); ++i)
	{
		const auto ci = item (i);
		if (ci->data ().value<GuardedReply_t> () == obj)
		{
			if (Clear_)
				removeRow (i);
			else
				ci->setData ({});
			break;
		}
	}
}
}
}
}
