#ifndef BLOCK_H
#define BLOCK_H
#include <QtGlobal>

class Block
{
	quint32 PieceIndex_;
	quint32 Offset_;
	quint32 Length_;
public:
	Block (quint32, quint32, quint32);

	quint32 GetPieceIndex () const;
	quint32 GetOffset () const;
	quint32 GetLength () const;
};

bool operator== (const Block&, const Block&);

#endif
