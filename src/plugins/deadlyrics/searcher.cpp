#include "searcher.h"

bool operator== (const Lyrics& l1, const Lyrics& l2)
{
	return (l1.Author_ == l2.Author_ ||
			l1.Title_ == l2.Title_) &&
		l1.Text_ == l2.Text_;
}

Searcher::~Searcher ()
{
}

