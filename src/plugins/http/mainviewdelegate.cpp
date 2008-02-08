#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QtDebug>
#include "mainviewdelegate.h"
#include "httpplugin.h"

MainViewDelegate::MainViewDelegate (HttpPlugin *plugin)
: QItemDelegate (plugin)
{
}

void MainViewDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column () != HttpPlugin::TListPercent)
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

    int progress = qobject_cast<HttpPlugin*> (parent ())->GetPercentageForRow (index.row ());
    pbo.progress = progress < 0 ? 0 : progress;
    pbo.text = QString::number (progress).append ("%");

    QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
}

