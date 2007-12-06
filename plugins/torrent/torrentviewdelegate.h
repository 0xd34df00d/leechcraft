#ifndef TORRENTVIEWDELEGATE_H
#define TORRENTVIEWDELEGATE_H
#include <QItemDelegate>
#include "torrentplugin.h"

class TorrentViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	TorrentViewDelegate (TorrentPlugin *tp = 0)
	: QItemDelegate (tp)
	{ }

	void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&);
};

#endif

