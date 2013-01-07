#ifndef TWITTERUSER_H
#define TWITTERUSER_H

#include <QObject>
#include <QPixmap>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QDebug>
namespace LeechCraft
{
namespace Woodpecker
{

class TwitterUser : public QObject
{
	Q_OBJECT
public:
	QPixmap  avatar;

	explicit TwitterUser (QObject *parent = 0);
	explicit TwitterUser (QString username, QObject *parent = 0);
	~TwitterUser ();


	void setUsername (QString username) {
		m_username = username;
	}
	QString username () {
		return m_username;
	}

	void downloadAvatar (QString path);

private:
	QString m_username;
	QNetworkRequest *req;
	QNetworkAccessManager *http;

signals:
	void userReady();

public slots:
	void avatarDownloaded (QNetworkReply *reply);

};
}
}

#endif // TWITTERUSER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
