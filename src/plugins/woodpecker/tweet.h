#ifndef TWEET_H
#define TWEET_H

#include <QObject>
#include <QDateTime>
#include <QTextLayout>
#include <QListWidget>
#include <QTextDocument>
#include <memory>
#include "twitteruser.h"

namespace LeechCraft
{
namespace Woodpecker
{

class Tweet : public QObject
{
	Q_OBJECT

public:
	Tweet (QObject *parent = 0);
	Tweet (QString text, TwitterUser *author = 0, QObject *parent = 0);
    Tweet (const Tweet& original);
	~Tweet ();
	void setText (QString text);

	QString text () const {
		return m_text;
	}
	
	qulonglong id () const {
		return m_id;
	}
	void setId (qulonglong id) {
		m_id = id;
	}

	TwitterUser* author () const {
		return m_author;
	}
	void setAuthor (TwitterUser *newAuthor) {
		m_author = newAuthor;
	}

	QDateTime dateTime () const {
		return m_created;
	}
	
	void setDateTime (QDateTime datetime) {
		m_created = datetime;
	}
	
	QTextDocument* getDocument() {
		return &m_document;
	}
	
	Tweet& operator= (const Tweet&);
	bool operator== (const Tweet&);
	bool operator!= (const Tweet&);
	bool operator> (const Tweet&);
	bool operator< (const Tweet&);

private:
	qulonglong	m_id;
	QString		m_text;
	TwitterUser	*m_author;
	QDateTime	m_created;
	QTextDocument m_document;

signals:

public slots:

};
}
}

Q_DECLARE_METATYPE ( LeechCraft::Woodpecker::Tweet );
Q_DECLARE_METATYPE ( std::shared_ptr<LeechCraft::Woodpecker::Tweet> );

#endif // TWEET_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
