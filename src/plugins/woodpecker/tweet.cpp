#include "tweet.h"
namespace LeechCraft
{
namespace Woodpecker
{

Tweet::Tweet (QObject *parent) :
	QObject (parent)
{
	this->m_author = new TwitterUser (parent);
}

Tweet::Tweet (QString text, TwitterUser *author, QObject *parent) :
	QObject (parent)
{
	this->m_id = 0;
	this->setText(text);

	if (author)
		this->m_author = new TwitterUser (parent);

	else
	{
		this->m_author = author;
		author->setParent (this);
	}
}

Tweet::~Tweet()
{
	m_author->deleteLater();
}

Tweet::Tweet(const Tweet& original): QObject()
{
    this->m_author = original.author();
    this->m_created = original.dateTime();
    this->setText(original.text());
    this->m_id = original.id();
}

Tweet& Tweet::operator = (const Tweet &rhs)
{
	if (this == &rhs)				// Same object?
		return *this;				// Yes, so skip assignment, and just return *this.

	this->m_id = rhs.id();
	this->m_author = rhs.author();
	this->m_created = rhs.dateTime();
	this->setText(rhs.text());

	return *this;
}

bool Tweet::operator == (const Tweet &other)
{
	return (this->m_id == other.id());
}

bool Tweet::operator != (const Tweet &other)
{
	return ! (*this == other);
}

bool Tweet::operator < (const Tweet &other)
{
	return (this->m_id < other.id());
}

bool Tweet::operator > (const Tweet &other)
{
	return (this->m_id > other.id());
}

void Tweet::setText (QString text) 
{
 QRegExp rx("\\s((http|https)://[a-z0-9]+([-.]{1}[a-z0-9]+)*.[a-z]{2,5}(([0-9]{1,5})?/?.*))(\\s|,|$)");
 rx.setMinimal(true);
 
 qDebug() << "Parsing links for tweet " << m_id;
 
 if (rx.indexIn(text) != -1) {
  for (auto link : rx.capturedTexts())
  {
   qDebug() << link;
  }
  qDebug() << "The link: " << rx.capturedTexts()[1];
 }
 m_text = text;
 
 QString html = text;
 html.replace(rx, QString(" <a href=\"%1\">%1</a> ").arg(rx.capturedTexts()[1]));
 qDebug() << "HTML: " << html;
 
 m_document.setHtml(html);
}

    
}
}
// kate: indent-mode cstyle; indent-width 1; replace-tabs on; 
