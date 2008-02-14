#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "mainviewdelegate.h"
#include "jobmanager.h"
#include "httpplugin.h"

MainViewDelegate::MainViewDelegate (HttpPlugin *plugin)
: QItemDelegate (plugin)
{
}

void MainViewDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid ())
        return;

    if (index.column () == JobManager::TListPercent)
    {
        QStyleOptionProgressBar pbo;
        pbo.state = QStyle::State_Enabled;
        pbo.direction = QApplication::layoutDirection ();
        pbo.rect = option.rect;
        pbo.fontMetrics = QApplication::fontMetrics ();
        pbo.minimum = 0;
        pbo.maximum = 100;
        pbo.textAlignment = Qt::AlignCenter;
        pbo.textVisible = true;

        quint64 ready = JobManager::Instance ().data (JobManager::Instance ().index (index.row (), JobManager::TListDownloaded, QModelIndex ()), Qt::DisplayRole).value<quint64> ();
        quint64 total = JobManager::Instance ().data (JobManager::Instance ().index (index.row (), JobManager::TListTotal, QModelIndex ()), Qt::DisplayRole).value<quint64> ();
        int progress = total ? (100 * static_cast<double> (ready) / static_cast<double> (total)) : total;
        pbo.text = QString::number (progress).append ("% (%1 of %2)").arg (Proxy::Instance ()->MakePrettySize (ready)).arg (Proxy::Instance ()->MakePrettySize (total));
        pbo.progress = progress;

        QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
    }
    else if (index.column () == JobManager::TListSpeed)
    {
        quint64 speed = JobManager::Instance ().data (index).value<quint64> ();
        painter->save ();
        painter->setRenderHint (QPainter::Antialiasing, true);
        painter->setFont (option.font);
        painter->drawText (option.rect, Qt::AlignCenter, Proxy::Instance ()->MakePrettySize (speed) + tr ("/s"));
        painter->restore ();
    }
    else if (index.column () == JobManager::TListDownloadTime || index.column () == JobManager::TListRemainingTime)
    {
        quint64 time = JobManager::Instance ().data (index).value<quint64> ();
        painter->save ();
        painter->setRenderHint (QPainter::Antialiasing, true);
        painter->setFont (option.font);
        painter->drawText (option.rect, Qt::AlignCenter, Proxy::Instance ()->MakeTimeFromLong (time).toString ());
        painter->restore ();
    }
    else
    {
        QItemDelegate::paint (painter, option, index);
        return;
    }

}

