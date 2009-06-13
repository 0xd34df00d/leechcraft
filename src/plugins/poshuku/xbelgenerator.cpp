#include "xbelgenerator.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <plugininterface/util.h>
#include "core.h"

using namespace LeechCraft::Plugins::Poshuku;

QString TagGetter (const QDomElement& elem)
{
	if (elem.tagName () == "folder")
		return elem.firstChildElement ("title").text ();
	else
		return QString ();
}

void TagSetter (QDomDocument& doc,
		QDomElement& elem, const QString& tag)
{
	QDomElement title = doc.createElement ("title");
	QDomText text = doc.createTextNode (tag);
	title.appendChild (text);
	elem.appendChild (title);
}

XbelGenerator::XbelGenerator (QByteArray& output)
{
	QDomDocument document;
	QDomElement root = document.createElement ("xbel");
	root.setAttribute ("version", "1.0");
	document.appendChild (root);
	for (FavoritesModel::items_t::const_iterator i =
			Core::Instance ().GetFavoritesModel ()->GetItems ().begin (),
			end = Core::Instance ().GetFavoritesModel ()->GetItems ().end ();
			i != end; ++i)
	{
		QDomElement inserter = LeechCraft::Util::GetElementForTags (i->Tags_,
				root, document, "folder",
				boost::function<QString (const QDomElement&)> (TagGetter),
				boost::bind (TagSetter, document, _1, _2));

		QDomElement item = document.createElement ("bookmark");
		item.setAttribute ("href", i->URL_);

		QDomElement title = document.createElement ("title");
		QDomText titleText = document.createTextNode (i->Title_);
		title.appendChild (titleText);
		item.appendChild (title);

		inserter.appendChild (item);
	}

	output = document.toByteArray (4);
}


