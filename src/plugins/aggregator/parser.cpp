#include "parser.h"
#include <QDomElement>
#include <QStringList>
#include <QtDebug>

channels_container_t Parser::Parse (const channels_container_t& o, const QByteArray& n)
{
	QDomDocument newd;
	newd.setContent (n, true);
	return Parse (o, newd);
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

QStringList Parser::GetDCCategories (const QDomElement& parent) const
{
	QStringList result;

	QDomNodeList nodes = parent.elementsByTagNameNS ("http://purl.org/dc/elements/1.1/", "subject");
	for (int i = 0; i < nodes.size (); ++i)
	{
		QDomElement elem = nodes.at (i).toElement ();
		if (elem.isNull ())
			continue;
		if (!elem.text ().isEmpty ())
			result += elem.text ();
	}

	result.removeAll ("");

	return result;
}

