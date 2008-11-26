#include "opmlwriter.h"
#include <QByteArray>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>

OPMLWriter::OPMLWriter ()
{
}

OPMLWriter::~OPMLWriter ()
{
}

QString OPMLWriter::Write (const channels_shorts_t& channels,
		const QString& title,
		const QString& owner,
		const QString& ownerEmail) const
{
	QDomDocument doc;
	QDomElement root = doc.createElement ("opml");
	doc.appendChild (root);
	WriteHead (root, doc, title, owner, ownerEmail);
	WriteBody (root, doc, channels);

	return doc.toString ();
}

void OPMLWriter::WriteHead (QDomElement& root,
		QDomDocument& doc,
		const QString& title,
		const QString& owner,
		const QString& ownerEmail) const
{
	QDomElement head = doc.createElement ("head");
	QDomElement text = doc.createElement ("text");
	head.appendChild (text);
	root.appendChild (head);

	if (!title.isEmpty ())
	{
		QDomText t = doc.createTextNode (title);
		text.appendChild (t);
	}
	if (!owner.isEmpty ())
	{
		QDomElement elem = doc.createElement ("owner");
		QDomText t = doc.createTextNode (owner);
		elem.appendChild (t);
		head.appendChild (elem);
	}
	if (!ownerEmail.isEmpty ())
	{
		QDomElement elem = doc.createElement ("ownerEmail");
		QDomText t = doc.createTextNode (ownerEmail);
		elem.appendChild (t);
		head.appendChild (elem);
	}
}

void OPMLWriter::WriteBody (QDomElement& root,
		QDomDocument& doc,
		const channels_shorts_t& channels) const
{
	QDomElement body = doc.createElement ("body");
	for (channels_shorts_t::const_iterator i = channels.begin (),
			end = channels.end (); i != end; ++i)
	{
		QStringList tags = i->Tags_;
		tags.sort ();

		QDomElement inserter = GetElementForTags (tags, body, doc);
		QDomElement item = doc.createElement ("outline");
		item.setAttribute ("title", i->Title_);
		item.setAttribute ("xmlUrl", i->ParentURL_);
		item.setAttribute ("htmlUrl", i->Link_);
		inserter.appendChild (item);
	}

	root.appendChild (body);
}

QDomElement OPMLWriter::GetElementForTags (const QStringList& tags,
		QDomNode& node, QDomDocument& document) const
{
	QDomNodeList elements = node.childNodes ();
	for (int i = 0; i < elements.size (); ++i)
	{
		QDomElement elem = elements.at (i).toElement ();
		if (elem.attribute ("text") == tags.at (0) &&
				!elem.hasAttribute ("xmlUrl"))
		{
			if (tags.size () > 1)
			{
				QStringList childTags = tags;
				childTags.removeAt (0);
				return GetElementForTags (childTags, elem, document);
			}
			else
				return elem;
		}
	}

	QDomElement result = document.createElement ("outline");
	result.setAttribute ("text", tags.at (0));
	result.setAttribute ("isOpen", "true");
	node.appendChild (result);
	if (tags.size () > 1)
	{
		QStringList childTags = tags;
		childTags.removeAt (0);
		return GetElementForTags (childTags, result, document);
	}
	else
		return result;
}

