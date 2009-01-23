#include "lyricwikisearcher.h"
#include <QHttp>
#include <QHttpRequestHeader>
#include <QDomDocument>
#include <QtDebug>
#include "core.h"

LyricWikiSearcher::LyricWikiSearcher ()
{
	setObjectName ("lyricwiki");
}

void LyricWikiSearcher::Start (const QString& artist, const QString& song,
		const QString&)
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
		.append (artist)
		.append ("</artist><song xsi:type=\"xsd:string\">")
		.append (song)
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
}

void LyricWikiSearcher::Stop ()
{
	qDeleteAll (findChildren<QHttp*> ());
}

void LyricWikiSearcher::handleFinished ()
{
	QHttp *http = qobject_cast<QHttp*> (sender ());
	QByteArray response = http->readAll ();
	http->deleteLater ();

	QDomDocument doc;
	doc.setContent (response, false);
	QDomNodeList lyrics = doc.elementsByTagName ("lyrics");
	if (!lyrics.size ())
	{
		qWarning () << Q_FUNC_INFO << "Lyrics fetch error";
		return;
	}

	QString text = lyrics.at (0).toElement ().text ();
	if (text == "Not found")
	{
		qWarning () << Q_FUNC_INFO << "No lyrics fetched";
		return;
	}

	emit textFetched (text);
}

