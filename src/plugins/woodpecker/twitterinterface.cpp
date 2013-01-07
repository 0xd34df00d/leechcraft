#include <boost/shared_ptr.hpp>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QDateTime>
#include <QDebug>
#include <qjson/parser.h>
#include <QtKOAuth/QtKOAuth>

#include "twitterinterface.h"

namespace LeechCraft
{
namespace Woodpecker
{

twitterInterface::twitterInterface (QObject *parent) :
	QObject (parent)
{
	HttpClient = new QNetworkAccessManager (this);
	oauthRequest = new KQOAuthRequest;
	oauthManager = new KQOAuthManager (this);

	oauthRequest->setEnableDebugOutput (true); // TODO: Remove debug
	consumerKey = QString ("nbwLYUDIlgsMgDFCu6jfuA");
	consumerKeySecret = QString ("7TWYPzLUqZlihIRA2VWfZhCRfss2JNKvkSWMQx4");

	connect (oauthManager, SIGNAL (requestReady (QByteArray)),
			 this, SLOT (onRequestReady (QByteArray)));

	connect (oauthManager, SIGNAL (authorizedRequestDone()),
			 this, SLOT (onAuthorizedRequestDone()));

	connect (HttpClient, SIGNAL (finished (QNetworkReply*)),
			 this, SLOT (replyFinished (QNetworkReply*)));

}

twitterInterface::~twitterInterface()
{
	delete oauthRequest;
	delete oauthManager;
}

void twitterInterface::getCommonFeed (unsigned int number)
{
	QString link ("http://api.twitter.com/1/statuses/public_timeline.json?count=2&include_entities=true");

	requestTwitter (link);
}



void twitterInterface::requestTwitter (QUrl requestAddress)
{
	HttpClient->get (QNetworkRequest (requestAddress));
	//           getAccess();
	//           xauth();
}

void twitterInterface::replyFinished (QNetworkReply *reply)
{
	QList< std::shared_ptr<Tweet> >    tweets;
	QByteArray       jsonText;
	jsonText = QByteArray (reply->readAll());
	reply->deleteLater();

	tweets.clear();
	tweets = parseReply (jsonText);
	emit tweetsReady (tweets);
}

QList< std::shared_ptr<Tweet> > twitterInterface::parseReply (QByteArray json)
{
	QJson::Parser parser;
	QList< std::shared_ptr<Tweet> > result;
	bool ok;

	QVariantList answers = parser.parse (json, &ok).toList();

	if (!ok) {
		qDebug() << "Parsing error at parseReply " << QString::fromUtf8(json);
	}

	QVariantMap tweetMap;
	QVariantMap userMap;

	for (int i = answers.count() - 1; i >= 0 ; i--)
	{
		tweetMap = answers[i].toMap();
		userMap = tweetMap["user"].toMap();
		QLocale::setDefault (QLocale::English);
		std::shared_ptr<Tweet> tempTweet (new Tweet (this->parent()));

		tempTweet->setText (tweetMap["text"].toString());
		tempTweet->author()->setUsername (userMap["screen_name"].toString());
		tempTweet->author()->downloadAvatar (userMap["profile_image_url"].toString());
		connect(tempTweet->author(), SIGNAL(userReady()), this->parent(), SLOT(updateTweetList()));
		tempTweet->setDateTime (QLocale().toDateTime (tweetMap["created_at"].toString(), QLatin1String ("ddd MMM dd HH:mm:ss +0000 yyyy")));
		tempTweet->setId (tweetMap["id"].toULongLong());

		qDebug() << tempTweet->dateTime().toString() << " : " << tempTweet->id();

		result.push_back (tempTweet);
	}

	return result;
}

void twitterInterface::getAccess()  {
	connect (oauthManager, SIGNAL (temporaryTokenReceived (QString, QString)),
			 this, SLOT (onTemporaryTokenReceived (QString, QString)));

	connect (oauthManager, SIGNAL (authorizationReceived (QString, QString)),
			 this, SLOT (onAuthorizationReceived (QString, QString)));

	connect (oauthManager, SIGNAL (accessTokenReceived (QString, QString)),
			 this, SLOT (onAccessTokenReceived (QString, QString)));

	oauthRequest->initRequest (KQOAuthRequest::TemporaryCredentials, QUrl ("https://api.twitter.com/oauth/request_token"));
	oauthRequest->setConsumerKey (consumerKey);
	oauthRequest->setConsumerSecretKey (consumerKeySecret);
	oauthManager->setHandleUserAuthorization (true);

	oauthManager->executeRequest (oauthRequest);

}

void twitterInterface::signedRequest (twitterRequest req, KQOAuthRequest::RequestHttpMethod method, KQOAuthParameters params)
{

	QUrl reqUrl;

	if (this->token.isEmpty() ||
			this->tokenSecret.isEmpty()) {
		qDebug() << "No access tokens. Aborting.";

		return;
	}

	switch (req)
	{
	case TRHomeTimeline:
		reqUrl = "https://api.twitter.com/1/statuses/home_timeline.json";
		params.insert ("count", "50");
		params.insert ("include_entities", "true");
		break;

	case TRUserTimeline:
		reqUrl = "http://api.twitter.com/1/statuses/user_timeline.json";
		params.insert ("include_entities", "true");
		break;

	case TRUpdate:
		reqUrl = "http://api.twitter.com/1/statuses/update.json";
		break;
		
	case TRDirect:
		reqUrl = "https://api.twitter.com/1/direct_messages.json";
		
	case TRRetweet:
		reqUrl = QString("http://api.twitter.com/1/statuses/retweet/").append(params.value("rt_id")).append(".json");
		break;
		
	case TRReply:
		reqUrl = "http://api.twitter.com/1/statuses/update.json";
		break;

	case TRSPAMReport:
		reqUrl = "http://api.twitter.com/1/report_spam.json";
		break;
		
	default:
		return;

	}

	oauthRequest->initRequest (KQOAuthRequest::AuthorizedRequest, reqUrl);
	oauthRequest->setHttpMethod (method);
	oauthRequest->setConsumerKey (consumerKey);
	oauthRequest->setConsumerSecretKey (consumerKeySecret);
	oauthRequest->setToken (this->token);
	oauthRequest->setTokenSecret (this->tokenSecret);

	oauthRequest->setAdditionalParameters (params);

	oauthManager->executeRequest (oauthRequest);

}

void twitterInterface::sendTweet (QString tweet)
{
	KQOAuthParameters param;
	param.insert ("status", tweet);
	signedRequest (TRUpdate, KQOAuthRequest::POST, param);
}

void twitterInterface::retweet(long unsigned int id)
{
	KQOAuthParameters param;
	param.insert ("rt_id", QString::number(id));
	signedRequest (TRRetweet, KQOAuthRequest::POST, param);
}

void twitterInterface::reply(long unsigned int replyid, QString tweet)
{
	KQOAuthParameters param;
	param.insert ("status", tweet);
	param.insert ("in_reply_to_status_id", QString::number(replyid));
	signedRequest (TRReply, KQOAuthRequest::POST, param);
}


void twitterInterface::onAuthorizedRequestDone() {
	qDebug() << "Request sent to Twitter!";
}

void twitterInterface::onRequestReady (QByteArray response) {
	qDebug() << "Response from the service: recvd";// << response;
	emit tweetsReady (parseReply (response));
}

void twitterInterface::onAuthorizationReceived (QString token, QString verifier) {
	qDebug() << "User authorization received: " << token << verifier;

	oauthManager->getUserAccessTokens (QUrl ("https://api.twitter.com/oauth/access_token"));

	if (oauthManager->lastError() != KQOAuthManager::NoError) {
	}

}

void twitterInterface::onAccessTokenReceived (QString token, QString tokenSecret) {
	qDebug() << "Access token received: " << token << tokenSecret;

	this->token = token;
	this->tokenSecret = tokenSecret;

	qDebug() << "Access tokens now stored. You are ready to send Tweets from user's account!";

	emit authorized (token, tokenSecret);

}

void twitterInterface::onTemporaryTokenReceived (QString token, QString tokenSecret)
{
	qDebug() << "Temporary token received: " << token << tokenSecret;

	QUrl userAuthURL ("https://api.twitter.com/oauth/authorize");

	if (oauthManager->lastError() == KQOAuthManager::NoError) {
		qDebug() << "Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
		oauthManager->getUserAuthorization (userAuthURL);
	}

}


void twitterInterface::xauth() {
	connect (oauthManager, SIGNAL (accessTokenReceived (QString, QString)),
			 this, SLOT (onAccessTokenReceived (QString, QString)));

	KQOAuthRequest_XAuth *oauthRequest = new KQOAuthRequest_XAuth (this);
	oauthRequest->initRequest (KQOAuthRequest::AccessToken, QUrl ("https://api.twitter.com/oauth/access_token"));
	oauthRequest->setConsumerKey ("nbwLYUDIlgsMgDFCu6jfuA");
	oauthRequest->setConsumerSecretKey ("7TWYPzLUqZlihIRA2VWfZhCRfss2JNKvkSWMQx4");

	// oauthRequest->setXAuthLogin("login","password");

	oauthManager->executeRequest (oauthRequest);
}

void twitterInterface::searchTwitter (QString text)
{
	QString link ("http://search.twitter.com/search.json?q=" + text);
	setLastRequestMode(FMSearchResult);
	requestTwitter (link);
}

void twitterInterface::getHomeFeed()
{
	qDebug() << "Getting home feed";
	setLastRequestMode(FMHomeTimeline);
	signedRequest (TRHomeTimeline, KQOAuthRequest::GET);
}

void twitterInterface::getMoreTweets(QString last)
{
	KQOAuthParameters param;

	qDebug() << "Getting more tweets from " << last;
	param.insert ("max_id",last);
	param.insert ("count",QString("%1").arg(30));
	setLastRequestMode(FMHomeTimeline);
	signedRequest (TRHomeTimeline, KQOAuthRequest::GET,param);
}


void twitterInterface::getUserTimeline (QString username)
{
	KQOAuthParameters param;
	param.insert ("screen_name", username);
	setLastRequestMode(FMUserTimeline);
	signedRequest (TRUserTimeline, KQOAuthRequest::GET, param);
}

void twitterInterface::login (QString savedToken, QString savedTokenSecret)
{
	token = savedToken;
	tokenSecret = savedTokenSecret;
	qDebug() << "Successfully logged in";
}

void twitterInterface::reportSPAM(QString username, long unsigned int userid)
{
	KQOAuthParameters param;

	param.insert ("screen_name", username);
	if (userid)
		param.insert ("user_id", QString::number(userid));
	signedRequest (TRSPAMReport, KQOAuthRequest::POST, param);
}

}
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
