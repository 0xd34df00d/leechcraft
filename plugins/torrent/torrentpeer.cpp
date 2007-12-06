#include "torrentpeer.h"

bool operator== (const TorrentPeer& tp1, const TorrentPeer& tp2)
{
	return (tp1.Port_ == tp2.Port_ &&
			tp1.Address_ == tp2.Address_ &&
			tp1.ID_ == tp2.ID_);
}

