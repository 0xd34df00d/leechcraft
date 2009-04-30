#include "networkaccessmanager.h"

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
{
}

QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QIODevice *out)
{
	QNetworkReply *result = QNetworkAccessManager::createRequest (op, req, out);
	emit requestCreated (op, req, result);
	return result;
}

