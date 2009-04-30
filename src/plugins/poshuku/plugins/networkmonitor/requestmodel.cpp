#include "requestmodel.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QtDebug>

Q_DECLARE_METATYPE (QNetworkReply*);

using namespace LeechCraft::Poshuku::Plugins::NetworkMonitor;

RequestModel::RequestModel (QObject *parent)
: QStandardItemModel (parent)
{
	setHorizontalHeaderLabels (QStringList (tr ("Date started"))
			<< tr ("Date finished")
			<< tr ("Type")
			<< tr ("Host"));
}

void RequestModel::handleRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QNetworkReply *rep)
{
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
			SIGNAL (finished ()),
			this,
			SLOT (handleFinished ()));
}

void RequestModel::handleFinished ()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
	if (!reply)
	{
		qWarning () << Q_FUNC_INFO << sender () << "not found";
		return;
	}

	for (int i = 0; i < rowCount (); ++i)
	{
		QStandardItem *ci = item (i);
		if (ci->data ().value<QNetworkReply*> () == reply)
		{
			item (i, 1)->setText (QDateTime::currentDateTime ().toString ());
			break;
		}
	}
}

