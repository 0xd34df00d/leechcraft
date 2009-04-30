#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H
#include <QNetworkAccessManager>

class NetworkAccessManager : public QNetworkAccessManager
{
	Q_OBJECT
public:
	NetworkAccessManager (QObject* = 0);
protected:
	QNetworkReply* createRequest (Operation,
			const QNetworkRequest&, QIODevice*);
signals:
	void requestCreated (QNetworkAccessManager::Operation,
			const QNetworkRequest&, QNetworkReply*);
};

#endif

