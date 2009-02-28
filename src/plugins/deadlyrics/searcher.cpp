#include "searcher.h"
#include <stdexcept>

bool operator== (const Lyrics& l1, const Lyrics& l2)
{
	return (l1.Author_ == l2.Author_ ||
			l1.Title_ == l2.Title_) &&
		l1.Text_ == l2.Text_;
}

QDataStream& operator<< (QDataStream& out, const Lyrics& lyrics)
{
	quint8 version = 1;
	out << version
		<< lyrics.Author_
		<< lyrics.Album_
		<< lyrics.Title_
		<< lyrics.Text_
		<< lyrics.URL_;
	return out;
}

QDataStream& operator>> (QDataStream& in, Lyrics& lyrics)
{
	quint8 version = 0;
	in >> version;
	if (version == 1)
	{
		in >> lyrics.Author_
			>> lyrics.Album_
			>> lyrics.Title_
			>> lyrics.Text_
			>> lyrics.URL_;
	}
	else
		throw std::runtime_error (qPrintable (QObject::tr ("Unknown %1 %2")
				.arg (version)
				.arg (Q_FUNC_INFO)));

	return in;
}

Searcher::~Searcher ()
{
}

