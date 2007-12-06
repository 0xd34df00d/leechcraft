#include "block.h"

Block::Block (quint32 index, quint32 offset, quint32 length)
: PieceIndex_ (index)
, Offset_ (offset)
, Length_ (length)
{
}

quint32 Block::GetPieceIndex () const
{
	return PieceIndex_;
}

quint32 Block::GetOffset () const
{
	return Offset_;
}

quint32 Block::GetLength () const
{
	return Length_;
}

bool operator== (const Block& b1, const Block& b2)
{
	return (b1.GetPieceIndex () == b2.GetPieceIndex () &&
			b1.GetOffset () == b2.GetOffset () &&
			b1.GetLength () == b2.GetLength ());
}

