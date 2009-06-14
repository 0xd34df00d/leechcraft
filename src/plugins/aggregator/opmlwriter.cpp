#include "opmlwriter.h"
#include <boost/function.hpp>
#include <QByteArray>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>
#include <plugininterface/util.h>

using namespace LeechCraft::Plugins::Aggregator;

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

QString TagGetter (const QDomElement& elem)
{
	return elem.attribute ("text");
}

void TagSetter (QDomElement& result, const QString& tag)
{
	result.setAttribute ("text", tag);
	result.setAttribute ("isOpen", "true");
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

		QDomElement inserter;
		inserter = LeechCraft::Util::GetElementForTags (tags,
				body, doc, "outline",
				boost::function<QString (const QDomElement&)> (TagGetter),
				boost::function<void (QDomElement&, const QString&)> (TagSetter));
		QDomElement item = doc.createElement ("outline");
		item.setAttribute ("title", i->Title_);
		item.setAttribute ("xmlUrl", i->ParentURL_);
		item.setAttribute ("htmlUrl", i->Link_);
		inserter.appendChild (item);
	}

	root.appendChild (body);
}


