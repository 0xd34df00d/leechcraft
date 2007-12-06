#include <QModelIndex>
#include <QStyleOptionProgressBar>
#include <QStyle>
#include <QApplication>
#include "torrentplugin.h"
#include "torrentviewdelegate.h"
#include "torrentclient.h"

void TorrentViewDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (index.column () != TorrentPlugin::CProgress)
	{
		QItemDelegate::paint (painter, option, index);
		return;
	}

	QStyleOptionProgressBar pbo;
	pbo.state = QStyle::State_Enabled;
	pbo.direction = QApplication::layoutDirection ();
	pbo.rect = option.rect;
	pbo.fontMetrics = QApplication::fontMetrics ();
	pbo.minimum = 0;
	pbo.maximum = 100;
	pbo.textAlignment = Qt::AlignCenter;
	pbo.textVisible = true;

	int progress = qobject_cast<TorrentPlugin*> (parent ())->GetClientForRow (index.row ())->GetProgress ();
	pbo.progress = progress < 0 ? 0 : progress;
	pbo.text = QString::number (progress) + QString ("%");

	QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
}

