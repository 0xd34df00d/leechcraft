#include "core.h"
#include <QDomDocument>
#include <QMetaType>
#include <QFile>
#include <QtDebug>
#include <plugininterface/util.h>

const QString Core::OS_ = "http://a9.com/-/spec/opensearch/1.1/";

Core::Core ()
{
	qRegisterMetaTypeStreamOperators<UrlDescription> ("UrlDescription");
	qRegisterMetaTypeStreamOperators<QueryDescription> ("QueryDescription");
	qRegisterMetaTypeStreamOperators<Description> ("Description");
}

Core& Core::Instance ()
{
	static Core c;
	return c;
}

void Core::Add (const QString& url)
{
	QString name = LeechCraft::Util::GetTemporaryName ();
	qDebug () << Q_FUNC_INFO << name << url;
	LeechCraft::DownloadEntity e =
	{
		url.toUtf8 (),
		name,
		QString (),
		LeechCraft::Internal |
			LeechCraft::DoNotSaveInHistory
	};

	int id = -1;
	QObject *provider;
	emit delegateEntity (e, &id, &provider);
	if (id == -1)
	{
		emit error (tr ("%1 wasn't delegated")
				.arg (url));
		return;
	}

	HandleProvider (provider);
	Jobs_ [id] = name;
}

void Core::handleJobFinished (int id)
{
	QString filename = Jobs_ [id];
	Jobs_.remove (id);

	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		emit error (tr ("Could not open file %1.")
				.arg (filename));
		return;
	}

	HandleEntity (file.readAll ());

	file.close ();
	if (!file.remove ())
		emit warning (tr ("Could not remove temporary file %1.")
				.arg (filename));
}

void Core::handleJobRemoved (int)
{
}

void Core::handleJobError (int id)
{
	emit error (tr ("A job was delegated, but it failed.")
			.arg (Jobs_ [id]));
	Jobs_.remove (id);
}

void Core::HandleEntity (const QString& contents)
{
	QDomDocument doc;
	QString errorString;
	int line, column;
	if (!doc.setContent (contents, true, &errorString, &line, &column))
	{
		qDebug () << contents;
		emit error (tr ("XML parse error %1 at %2:%3.")
				.arg (errorString)
				.arg (line)
				.arg (column));
		return;
	}

	QDomElement root = doc.documentElement ();
	if (root.tagName () != "OpenSearchDescription")
	{
		emit error (tr ("Not and OpenSearch description."));
		return;
	}

	QDomElement shortNameTag = root.firstChildElement ("ShortName");
	QDomElement descriptionTag = root.firstChildElement ("Description");
	QDomElement urlTag = root.firstChildElement ("Url");
	if (shortNameTag.isNull () ||
			descriptionTag.isNull () ||
			urlTag.isNull () ||
			!urlTag.hasAttribute ("template") ||
			!urlTag.hasAttribute ("type"))
	{
		qWarning () << shortNameTag.isNull ()
			<< descriptionTag.isNull ()
			<< urlTag.isNull ();
		for (int i = 0, size = urlTag.attributes ().size (); i < size; ++i)
			qWarning () << urlTag.attributes ().item (i).nodeName ();
		emit error (tr ("Malformed OpenSearch description."));
		return;
	}

	Description descr;
	descr.ShortName_ = shortNameTag.text ();
	descr.Description_ = descriptionTag.text ();

	while (!urlTag.isNull ())
	{
		UrlDescription d =
		{
			urlTag.attributeNS (OS_, "template"),
			urlTag.attributeNS (OS_, "type"),
			urlTag.attributeNS (OS_, "indexOffset", "1").toInt (),
			urlTag.attributeNS (OS_, "pageOffset", "1").toInt ()
		};
		descr.URLs_ << d;

		urlTag = urlTag.nextSiblingElement ("Url");
	}

	QDomElement contactTag = root.firstChildElement ("Contact");
	if (!contactTag.isNull ())
		descr.Contact_ = contactTag.text ();

	QDomElement tagsTag = root.firstChildElement ("Tags");
	if (!tagsTag.isNull ())
		descr.Tags_ = tagsTag.text ().split (' ', QString::SkipEmptyParts);

	QDomElement longNameTag = root.firstChildElement ("LongName");
	if (!longNameTag.isNull ())
		descr.LongName_ = longNameTag.text ();

	QDomElement queryTag = root.firstChildElement ("Query");
	while (!queryTag.isNull () && queryTag.hasAttributeNS (OS_, "role"))
	{
		QueryDescription::Role r;
		QString role = queryTag.attributeNS (OS_, "role");
		if (role == "request")
			r = QueryDescription::RoleRequest;
		else if (role == "example")
			r = QueryDescription::RoleExample;
		else if (role == "related")
			r = QueryDescription::RoleRelated;
		else if (role == "correction")
			r = QueryDescription::RoleCorrection;
		else if (role == "subset")
			r = QueryDescription::RoleSubset;
		else if (role == "superset")
			r = QueryDescription::RoleSuperset;
		else
		{
			queryTag = queryTag.nextSiblingElement ("Query");
			continue;
		}

		QueryDescription d =
		{
			r,
			queryTag.attributeNS (OS_, "title"),
			queryTag.attributeNS (OS_, "totalResults", "-1").toInt (),
			queryTag.attributeNS (OS_, "searchTerms"),
			queryTag.attributeNS (OS_, "count", "-1").toInt (),
			queryTag.attributeNS (OS_, "startIndex", "-1").toInt (),
			queryTag.attributeNS (OS_, "startPage", "-1").toInt (),
			queryTag.attributeNS (OS_, "language", "*"),
			queryTag.attributeNS (OS_, "inputEncoding", "UTF-8"),
			queryTag.attributeNS (OS_, "outputEncoding", "UTF-8")
		};
		descr.Queries_ << d;
		queryTag = queryTag.nextSiblingElement ("Query");
	}

	QDomElement developerTag = root.firstChildElement ("Developer");
	if (!developerTag.isNull ())
		descr.Developer_ = developerTag.text ();

	QDomElement attributionTag = root.firstChildElement ("Attribution");
	if (!attributionTag.isNull ())
		descr.Attribution_ = attributionTag.text ();

	descr.Right_ = Description::SROpen;
	QDomElement syndicationRightTag = root.firstChildElement ("SyndicationRight");
	if (!syndicationRightTag.isNull ())
	{
		QString sr = syndicationRightTag.text ();
		if (sr == "limited")
			descr.Right_ = Description::SRLimited;
		else if (sr == "private")
			descr.Right_ = Description::SRPrivate;
		else if (sr == "closed")
			descr.Right_ = Description::SRClosed;
	}

	descr.Adult_ = false;
	QDomElement adultContentTag = root.firstChildElement ("AdultContent");
	if (!adultContentTag.isNull ())
	{
		QString text = adultContentTag.text ();
		if (!(text == "false" ||
				text == "FALSE" ||
				text == "0" ||
				text == "no" ||
				text == "NO"))
			descr.Adult_ = true;
	}

	QDomElement languageTag = root.firstChildElement ("Language");
	bool was = false;;
	while (!languageTag.isNull ())
	{
		descr.Languages_ << languageTag.text ();
		was = true;
		languageTag = languageTag.nextSiblingElement ("Language");
	}
	if (!was)
		descr.Languages_ << "*";

	QDomElement inputEncodingTag = root.firstChildElement ("InputEncoding");
	was = false;
	while (!inputEncodingTag.isNull ())
	{
		descr.InputEncodings_ << inputEncodingTag.text ();
		was = true;
		inputEncodingTag = inputEncodingTag.nextSiblingElement ("InputEncoding");
	}
	if (!was)
		descr.InputEncodings_ << "UTF-8";

	QDomElement outputEncodingTag = root.firstChildElement ("OutputEncoding");
	was = false;
	while (!outputEncodingTag.isNull ())
	{
		descr.InputEncodings_ << outputEncodingTag.text ();
		was = true;
		outputEncodingTag = outputEncodingTag.nextSiblingElement ("OutputEncoding");
	}
	if (!was)
		descr.InputEncodings_ << "UTF-8";
}

void Core::HandleProvider (QObject *provider)
{
	if (Downloaders_.contains (provider))
		return;
	
	Downloaders_ << provider;
	connect (provider,
			SIGNAL (jobFinished (int)),
			this,
			SLOT (handleJobFinished (int)));
	connect (provider,
			SIGNAL (jobRemoved (int)),
			this,
			SLOT (handleJobRemoved (int)));
	connect (provider,
			SIGNAL (jobError (int, IDownload::Error)),
			this,
			SLOT (handleJobError (int, IDownload::Error)));
}

