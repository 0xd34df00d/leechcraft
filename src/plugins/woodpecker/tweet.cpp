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
	this->m_text = text;

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

Tweet& Tweet::operator = (const Tweet &rhs)
{
	if (this == &rhs)				// Same object?
		return *this;				// Yes, so skip assignment, and just return *this.

	this->m_author = rhs.author();
	this->m_created = rhs.dateTime();
	this->m_text = rhs.text();

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

}
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
