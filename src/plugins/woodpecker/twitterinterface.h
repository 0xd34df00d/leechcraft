#ifndef TWITTERINTERFACE_H
#define TWITTERINTERFACE_H

#include <boost/shared_ptr.hpp>

#include <QList>
#include <QObject>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QSettings>

#include <QtKOAuth/QtKOAuth>

#include "tweet.h"

namespace LeechCraft
{
namespace Woodpecker
{

enum twitterRequest
{
	TRHomeTimeline,
	TRMentions,
	TRUserTimeline,
	TRUpdate,
	TRDirect,
	TRRetweet,
	TRReply,
};

enum feedMode
{
	FMHomeTimeline,
	FMMentions,
	FMUserTimeline,
	FMSearchResult,
	FMDirect,
};

class twitterInterface : public QObject
{
	Q_OBJECT

public:
	explicit twitterInterface (QObject *parent = 0);
	~twitterInterface ();
	void sendTweet (QString tweet);
	void retweet (long unsigned int id);
	void reply (long unsigned int replyid, QString tweet);
	void getAccess ();
	void login (QString savedToken, QString savedTokenSecret);
	feedMode getLastRequestMode() { return  lastRequestMode;};
	void setLastRequestMode(feedMode newLastRequestMode) { lastRequestMode = newLastRequestMode;};

private:
	QNetworkAccessManager *HttpClient;
	KQOAuthManager *oauthManager;
	KQOAuthRequest *oauthRequest;
	QString token;
	QString tokenSecret;
	QString consumerKey;
	QString consumerKeySecret;
	QSettings *settings;
	feedMode lastRequestMode;

	void signedRequest (twitterRequest req, KQOAuthRequest::RequestHttpMethod method = KQOAuthRequest::GET, KQOAuthParameters params = KQOAuthParameters());

	void requestTwitter (QUrl requestAddress);
	QList < std::shared_ptr<Tweet> > parseReply (QByteArray json);


	void xauth();

private slots:
	void replyFinished (QNetworkReply* reply);

	void onTemporaryTokenReceived (QString temporaryToken, QString temporaryTokenSecret);
	void onAuthorizationReceived (QString token, QString verifier);
	void onRequestReady (QByteArray);
	void onAuthorizedRequestDone ();
	void onAccessTokenReceived (QString token, QString tokenSecret);
signals:
	void tweetsReady (QList< std::shared_ptr<Tweet> >);
	void authorized (QString, QString);

public slots:
	void getCommonFeed (unsigned int number);        // Downloads number of tweets from the main twitter feed
	void getHomeFeed ();
	void searchTwitter (QString text);
	void getUserTimeline (QString username);
	void getMoreTweets (QString last);
};
}
}

#endif // TWITTERINTERFACE_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
