/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "requestmodel.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextCodec>
#include <QDateTime>
#include <QtDebug>
#include "headermodel.h"

Q_DECLARE_METATYPE (QNetworkReply*);

using namespace LeechCraft::Plugins::NetworkMonitor;

LeechCraft::Plugins::NetworkMonitor::RequestModel::RequestModel (QObject *parent)
: QStandardItemModel (parent)
, Clear_ (true)
{
	setHorizontalHeaderLabels (QStringList (tr ("Date started"))
			<< tr ("Date finished")
			<< tr ("Type")
			<< tr ("Host"));

	RequestHeadersModel_ = new HeaderModel (this);
	ReplyHeadersModel_ = new HeaderModel (this);
}

HeaderModel* RequestModel::GetRequestHeadersModel () const
{
	return RequestHeadersModel_;
}

HeaderModel* RequestModel::GetReplyHeadersModel () const
{
	return ReplyHeadersModel_;
}

void LeechCraft::Plugins::NetworkMonitor::RequestModel::handleRequest (QNetworkAccessManager::Operation op,
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
		case QNetworkAccessManager::UnknownOperation:
			opName = "Unknown";
			break;
	}
	items.push_back (new QStandardItem (QDateTime::currentDateTime ().toString ()));
	items.push_back (new QStandardItem (tr ("In progress")));
	items.push_back (new QStandardItem (opName));
	items.push_back (new QStandardItem (req.url ().toString ()));
	items.first ()->setData (QVariant::fromValue<QNetworkReply*> (rep));
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
		QMap<QString, QVariant> GetHeaders (const T* object)
	{
		QMap<QString, QVariant> result;
		QList<QByteArray> headers = object->rawHeaderList ();
		QTextCodec *codec = QTextCodec::codecForName ("UTF-8");
		Q_FOREACH (QByteArray header, headers)
			result [codec->toUnicode (header)] = codec->toUnicode (object->rawHeader (header));
		return result;
	}

	template<typename T>
		void FeedHeaders (T object, HeaderModel* model)
	{
		QMap<QString, QVariant> headers = GetHeaders (object);
		Q_FOREACH (QString header, headers.keys ())
			model->AddHeader (header, headers [header].toString ());
	}

	template<>
		void FeedHeaders (QMap<QString, QVariant> headers, HeaderModel* model)
	{
		Q_FOREACH (QString header, headers.keys ())
			model->AddHeader (header, headers [header].toString ());
	}
}

void RequestModel::handleFinished ()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
	if (!reply)
	{
		qWarning () << Q_FUNC_INFO
			<< sender ()
			<< "not found";
		return;
	}

	for (int i = 0; i < rowCount (); ++i)
	{
		QStandardItem *ci = item (i);
		if (ci->data ().value<QNetworkReply*> () == reply)
		{
			if (Clear_)
				qDeleteAll (takeRow (i));
			else
			{
				item (i, 1)->setText (QDateTime::currentDateTime ().toString ());
				item (i, 0)->setData (0);
				QNetworkRequest r = reply->request ();
				item (i, 1)->setData (GetHeaders (&r));
				item (i, 2)->setData (GetHeaders (reply));
			}
			break;
		}
	}
}

void RequestModel::setClear (bool clear)
{
	Clear_ = clear;
	if (Clear_)
	{
		for (int i = rowCount () - 1; i >= 0; --i)
			if (!item (i)->data ().value<QNetworkReply*> ())
				qDeleteAll (takeRow (i));
		handleCurrentChanged (QModelIndex ());
	}
}

void RequestModel::handleCurrentChanged (const QModelIndex& newItem)
{
	RequestHeadersModel_->clear ();
	ReplyHeadersModel_->clear ();

	if (!newItem.isValid ())
		return;

	QNetworkReply *reply = item (itemFromIndex (newItem)->row (), 0)->
		data ().value<QNetworkReply*> ();
	if (reply)
	{
		QNetworkRequest r = reply->request ();
		FeedHeaders (&r, RequestHeadersModel_);
		FeedHeaders (reply, ReplyHeadersModel_);
	}
	else
	{
		FeedHeaders (item (itemFromIndex (newItem)->row (), 1)->
				data ().toMap (), RequestHeadersModel_);
		FeedHeaders (item (itemFromIndex (newItem)->row (), 2)->
				data ().toMap (), ReplyHeadersModel_);
	}
}

void RequestModel::handleGonnaDestroy (QObject *obj)
{
	if (!obj && sender ())
		obj = sender ();

	for (int i = 0; i < rowCount (); ++i)
	{
		QStandardItem *ci = item (i);
		if (ci->data ().value<QNetworkReply*> () == obj)
		{
			qDeleteAll (takeRow (i));
			break;
		}
	}
}

