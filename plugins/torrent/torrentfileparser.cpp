#include <QtDebug>
#include "torrentfileparser.h"

TorrentFileParser::TorrentFileParser ()
: Mode_ (Singlefile)
, StateOK_ (false)
{
	Parser_ = new BencodeParser ();
}

TorrentFileParser::~TorrentFileParser ()
{
	delete Parser_;
}

bool TorrentFileParser::Parse (const QByteArray& file)
{
	StateOK_ = Parser_->Parse (file);

	if (StateOK_)
	{
		Info_ = ((Parser_->GetParsed ()) ["info"]).value<Dictionary_t> ();
		Announce_ = ((Parser_->GetParsed ()) ["announce"]).toByteArray ();
	}
	else
	{
		qDebug () << Parser_->GetErrorString ();
	}

	return StateOK_;
}

Dictionary_t TorrentFileParser::GetInfo () const
{
	return Info_;
}

QString TorrentFileParser::GetAnnounce () const
{
	return Announce_;
}

