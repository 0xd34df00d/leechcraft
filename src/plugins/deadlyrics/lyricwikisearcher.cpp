#include "lyricwikisearcher.h"
#include <QHttp>
#include <QHttpRequestHeader>
#include <QDomDocument>
#include <QtDebug>
#include <QCryptographicHash>
#include "core.h"

LyricWikiSearcher::LyricWikiSearcher ()
{
	setObjectName ("lyricwiki");
}

QByteArray LyricWikiSearcher::Start (const QStringList& asa)
{
	QByteArray data = QByteArray ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
				"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
				"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
				"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
				"xmlns:si=\"http://soapinterop.org/xsd\" "
				"xmlns:tns=\"urn:LyricWiki\" "
				"xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\" "
				"xmlns:wsdl=\"http://schemas.xmlsoap.org/wsdl/\" "
				"xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
				"SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
				"<SOAP-ENV:Body><tns:getSong>"
				"<artist xsi:type=\"xsd:string\">")
		.append (asa.at (0))
		.append ("</artist><song xsi:type=\"xsd:string\">")
		.append (asa.at (1))
		.append ("</song></tns:getSong></SOAP-ENV:Body></SOAP-ENV:Envelope>");

	QHttpRequestHeader request ("POST", "/server.php");
	request.setValue ("TE", "deflate,gzip;q=0.3");
	request.setValue ("Connection", "close");
	request.setValue ("Host", "lyricwiki.org");
	request.setValue ("User-Agent", "DeadLyRicS/LeechCraft");
	request.setValue ("Content-Length", QString::number (data.size ()));
	request.setValue ("Content-Type", "text/xml; charset=utf-8");
	request.setValue ("SOAPAction", "\"urn:LyricWiki#getSong\"");

	QHttp *http = new QHttp (this);
	http->setHost ("www.lyricwiki.org");
	http->request (request, data);

	connect (http,
			SIGNAL (done (bool)),
			this,
			SLOT (handleFinished ()));

	QByteArray hash = QCryptographicHash::hash (asa.join ("").toUtf8 (),
			QCryptographicHash::Sha1);
	http->setObjectName (hash);
	http->setProperty ("IDHash", hash);
	return hash;
}

void LyricWikiSearcher::Stop (const QByteArray& hash)
{
	qDeleteAll (findChildren<QHttp*> (hash));
}

void LyricWikiSearcher::handleFinished ()
{
	QHttp *http = qobject_cast<QHttp*> (sender ());
	QByteArray response = http->readAll ();
	http->deleteLater ();

	QDomDocument doc;
	doc.setContent (response, false);
	QDomNodeList lyrics = doc.elementsByTagName ("lyrics");
	QDomNodeList artist = doc.elementsByTagName ("artist");
	QDomNodeList song = doc.elementsByTagName ("song");
	QDomNodeList url = doc.elementsByTagName ("url");
	if (!lyrics.size () ||
			!artist.size () ||
			!song.size () ||
			!url.size ())
	{
		qWarning () << Q_FUNC_INFO << "Lyrics fetch error" << response;
		return;
	}

	QString text = lyrics.at (0).toElement ().text ();
	if (text == "Not found")
	{
		qWarning () << Q_FUNC_INFO << "No lyrics fetched";
		return;
	}

	Lyrics result =
	{
		artist.at (0).toElement ().text (),
		"",
		song.at (0).toElement ().text (),
		text,
		url.at (0).toElement ().text ()
	};

	emit textFetched (result, http->property ("IDHash").toByteArray ());
}

