#include "core.h"
#include <stdexcept>
#include <QDomDocument>
#include <QMetaType>

QDataStream& operator<< (QDataStream& out, const UrlDescription& d)
{
	quint8 version = 1;
	out << version
		<< d.Template_
		<< d.Type_
		<< d.IndexOffset_
		<< d.PageOffset_;
	return out;
}

QDataStream& operator>> (QDataStream& in, UrlDescription& d)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
		throw std::runtime_error ("Unknown version for UrlDescription");
	in >> d.Template_
		>> d.Type_
		>> d.IndexOffset_
		>> d.PageOffset_;
	return in;
}

QDataStream& operator<< (QDataStream& out, const QueryDescription& d)
{
	quint8 version = 1;
	out << version
		<< static_cast<quint8> (d.Role_)
		<< d.Title_
		<< d.TotalResults_
		<< d.SearchTerms_
		<< d.Count_
		<< d.StartIndex_
		<< d.StartPage_
		<< d.Language_
		<< d.InputEncoding_
		<< d.OutputEncoding_;
	return out;
}

QDataStream& operator>> (QDataStream& in, QueryDescription& d)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
		throw std::runtime_error ("Unknown version for QueryDescription");
	quint8 role;
	in >> role;
	d.Role_ = static_cast<QueryDescription::Role> (role);
	in >> d.Title_
		>> d.TotalResults_
		>> d.SearchTerms_
		>> d.Count_
		>> d.StartIndex_
		>> d.StartPage_
		>> d.Language_
		>> d.InputEncoding_
		>> d.OutputEncoding_;
	return in;
}

QDataStream& operator<< (QDataStream& out, const Description& d)
{
	quint8 version = 1;
	out << version
		<< d.ShortName_
		<< d.Description_
		<< d.URLs_
		<< d.Contact_
		<< d.Tags_
		<< d.Queries_
		<< d.Developer_
		<< d.Attribution_
		<< static_cast<quint8> (d.Right_)
		<< d.Adult_
		<< d.Languages_
		<< d.InputEncodings_
		<< d.OutputEncodings_;
	return out;
}

QDataStream& operator>> (QDataStream& in, Description& d)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
		throw std::runtime_error ("Unknown version for Description");
	in >> d.ShortName_
		>> d.Description_
		>> d.URLs_
		>> d.Contact_
		>> d.Tags_
		>> d.Queries_
		>> d.Developer_
		>> d.Attribution_;
	quint8 sr;
	in >> sr;
	d.Right_ = static_cast<Description::SyndicationRight> (sr);
	in >> d.Adult_
		>> d.Languages_
		>> d.InputEncodings_
		>> d.OutputEncodings_;
	return in;
}

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

void Core::Add (const QString& contents)
{
	QDomDocument doc;
	QString errorString;
	int line, column;
	if (!doc.setContent (contents, true, &errorString, &line, &column))
	{
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

	QDomElement shortNameTag = doc.firstChildElement ("ShortName");
	QDomElement descriptionTag = doc.firstChildElement ("Description");
	QDomElement urlTag = doc.firstChildElement ("Url");
	if (shortNameTag.isNull () ||
			descriptionTag.isNull () ||
			urlTag.isNull () ||
			!urlTag.hasAttribute ("template") ||
			!urlTag.hasAttribute ("type"))
	{
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
			urlTag.attribute ("template"),
			urlTag.attribute ("type"),
			urlTag.attribute ("indexOffset", "1").toInt (),
			urlTag.attribute ("pageOffset", "1").toInt ()
		};
		descr.URLs_ << d;

		urlTag = urlTag.nextSiblingElement ("Url");
	}

	QDomElement contactTag = doc.firstChildElement ("Contact");
	if (!contactTag.isNull ())
		descr.Contact_ = contactTag.text ();

	QDomElement tagsTag = doc.firstChildElement ("Tags");
	if (!tagsTag.isNull ())
		descr.Tags_ = tagsTag.text ().split (' ', QString::SkipEmptyParts);

	QDomElement longNameTag = doc.firstChildElement ("LongName");
	if (!longNameTag.isNull ())
		descr.LongName_ = longNameTag.text ();

	QDomElement queryTag = doc.firstChildElement ("Query");
	while (!queryTag.isNull () && queryTag.hasAttribute ("role"))
	{
		QueryDescription::Role r;
		QString role = queryTag.attribute ("role");
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
			queryTag.attribute ("title"),
			queryTag.attribute ("totalResults", "-1").toInt (),
			queryTag.attribute ("searchTerms"),
			queryTag.attribute ("count", "-1").toInt (),
			queryTag.attribute ("startIndex", "-1").toInt (),
			queryTag.attribute ("startPage", "-1").toInt (),
			queryTag.attribute ("language", "*"),
			queryTag.attribute ("inputEncoding", "UTF-8"),
			queryTag.attribute ("outputEncoding", "UTF-8")
		};
		descr.Queries_ << d;
		queryTag = queryTag.nextSiblingElement ("Query");
	}

	QDomElement developerTag = doc.firstChildElement ("Developer");
	if (!developerTag.isNull ())
		descr.Developer_ = developerTag.text ();

	QDomElement attributionTag = doc.firstChildElement ("Attribution");
	if (!attributionTag.isNull ())
		descr.Attribution_ = attributionTag.text ();

	descr.Right_ = Description::SROpen;
	QDomElement syndicationRightTag = doc.firstChildElement ("SyndicationRight");
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
	QDomElement adultContentTag = doc.firstChildElement ("AdultContent");
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

	QDomElement languageTag = doc.firstChildElement ("Language");
	bool was = false;;
	while (!languageTag.isNull ())
	{
		descr.Languages_ << languageTag.text ();
		was = true;
		languageTag = languageTag.nextSiblingElement ("Language");
	}
	if (!was)
		descr.Languages_ << "*";

	QDomElement inputEncodingTag = doc.firstChildElement ("InputEncoding");
	was = false;
	while (!inputEncodingTag.isNull ())
	{
		descr.InputEncodings_ << inputEncodingTag.text ();
		was = true;
		inputEncodingTag = inputEncodingTag.nextSiblingElement ("InputEncoding");
	}
	if (!was)
		descr.InputEncodings_ << "UTF-8";

	QDomElement outputEncodingTag = doc.firstChildElement ("OutputEncoding");
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

