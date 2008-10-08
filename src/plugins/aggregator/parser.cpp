#include "parser.h"
#include <QDomElement>
#include <QStringList>
#include <QtDebug>

const QString Parser::DC_ = "http://purl.org/dc/elements/1.1/";
const QString Parser::WFW_ = "http://wellformedweb.org/CommentAPI/";
const QString Parser::Atom_ = "http://www.w3.org/2005/Atom";
const QString Parser::RDF_ = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

channels_container_t Parser::Parse (const channels_container_t& o,
		const QByteArray& n)
{
	QDomDocument newd;
	newd.setContent (n, true);
	return Parse (o, newd);
}

QString Parser::GetLink (const QDomElement& parent) const
{
    QString result;
    QDomElement link = parent.firstChildElement ("link");
    while (!link.isNull ())
    {
        if (!link.hasAttribute ("rel") || link.attribute ("rel") == "alternate")
		{
			if (!link.hasAttribute ("href"))
				result = link.text ();
			else
				result = link.attribute ("href");
			break;
		}
        link = link.nextSiblingElement ("link");
    }
    return result;
}

QString Parser::GetAuthor (const QDomElement& parent) const
{
	QString result;
	QDomNodeList nodes = parent.elementsByTagNameNS (DC_,
			"creator");
	if (nodes.size ())
		result = nodes.at (0).toElement ().text ();
	return result;
}

QString Parser::GetCommentsRSS (const QDomElement& parent) const
{
	QString result;
	QDomNodeList nodes = parent.elementsByTagNameNS (WFW_,
			"commentRss");
	if (nodes.size ())
		result = nodes.at (0).toElement ().text ();
	return result;
}

QDateTime Parser::GetDCDateTime (const QDomElement& parent) const
{
	QDomNodeList dates = parent.elementsByTagNameNS (DC_, "date");
	if (!dates.size ())
		return QDateTime ();
	return FromRFC3339 (dates.at (0).toElement ().text ());
}

QStringList Parser::GetAllCategories (const QDomElement& parent) const
{
	return GetDCCategories (parent) + GetPlainCategories (parent);
}

QStringList Parser::GetDCCategories (const QDomElement& parent) const
{
	QStringList result;

	QDomNodeList nodes =
		parent.elementsByTagNameNS (DC_,
				"subject");
	for (int i = 0; i < nodes.size (); ++i)
		result += nodes.at (i).toElement ().text ();

	result.removeAll ("");

	return result;
}

QStringList Parser::GetPlainCategories (const QDomElement& parent) const
{
	QStringList result;

	QDomNodeList nodes =
		parent.elementsByTagName ("category");
	for (int i = 0; i < nodes.size (); ++i)
		result += nodes.at (i).toElement ().text ();

	result.removeAll ("");

	return result;
}

// Via
// http://www.theukwebdesigncompany.com/articles/entity-escape-characters.php
QString Parser::UnescapeHTML (const QString& escaped) const
{
    QString result = escaped;
    result.replace ("&euro;", "€");
    result.replace ("&quot;", "\"");
    result.replace ("&amp;", "&");
    result.replace ("&nbsp;", " ");
    result.replace ("&lt;", "<");
    result.replace ("&gt;", ">");
	result.replace ("&#8217;", "'");
	result.replace ("&#8230;", "...");
    return result;
}

QDateTime Parser::FromRFC3339 (const QString& t) const
{
	int hoursShift = 0, minutesShift = 0;
	if (t.size () < 19)
		return QDateTime ();
	QDateTime result = QDateTime::fromString (t.left (19).toUpper (), "yyyy-MM-ddTHH:mm:ss");
	QRegExp fractionalSeconds ("(\\.)(\\d+)");
	if (fractionalSeconds.indexIn (t) > -1)
	{
		bool ok;
		int fractional = fractionalSeconds.cap (2).toInt (&ok);
		if (ok)
		{
			if (fractional < 100)
				fractional *= 10;
			if (fractional <10) 
				fractional *= 100;
			result.addMSecs (fractional);
		}
	}
	QRegExp timeZone ("(\\+|\\-)(\\d\\d)(:)(\\d\\d)$");
	if (timeZone.indexIn (t) > -1)
	{
		short int multiplier = -1;
		if (timeZone.cap (1) == "-")
			multiplier = 1;
		hoursShift = timeZone.cap (2).toInt ();
		minutesShift = timeZone.cap (4).toInt ();
		result = result.addSecs (hoursShift * 3600 * multiplier + minutesShift * 60 * multiplier);
	}
	result.setTimeSpec (Qt::UTC);
	return result.toLocalTime ();
}

