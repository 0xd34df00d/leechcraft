#ifndef TORRENTFILEPARSER_H
#define TORRENTFILEPARSER_H
#include "bencodeparser.h"

class TorrentFileParser
{
	Dictionary_t Info_;
	QString Announce_;

	enum Mode { Singlefile, Multifile };

	Mode Mode_;

	bool StateOK_;

	BencodeParser *Parser_;
public:
	TorrentFileParser ();
	virtual ~TorrentFileParser ();
	bool Parse (const QByteArray&);
	Dictionary_t GetInfo () const;
	QString GetAnnounce () const;
};

#endif

