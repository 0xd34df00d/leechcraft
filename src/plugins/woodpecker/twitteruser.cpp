#include "twitteruser.h"
namespace LeechCraft
{
namespace Woodpecker
{

TwitterUser::TwitterUser (QObject *parent) :
	QObject (parent)
{
	http = new QNetworkAccessManager (this);

}

TwitterUser::TwitterUser (QString username, QObject *parent) :
	QObject (parent)
{
	this->m_username = username;
}

void TwitterUser::avatarDownloaded (QNetworkReply *reply)
{
	QByteArray data;

	data = reply->readAll();
	avatar.loadFromData (data);
	reply->deleteLater();
	emit userReady();
}

void TwitterUser::downloadAvatar (QString path)
{
	req = new QNetworkRequest (QUrl (path));
	connect (http, SIGNAL (finished (QNetworkReply*)),
			 this, SLOT (avatarDownloaded (QNetworkReply*)));

	http->get (*req);
}

TwitterUser::~TwitterUser()
{
	if (req) delete req;

	http->deleteLater();
}

}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
