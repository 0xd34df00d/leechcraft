#ifndef TRACKERSCHANGER_H
#define TRACKERSCHANGER_H
#include <QDialog>
#include "ui_trackerschanger.h"

class TrackersChanger : public QDialog
{
    Q_OBJECT

    Ui::TrackersChanger Ui_;
public:
    TrackersChanger (QWidget *parent = 0);
    void SetTrackers (const QStringList&);
    QStringList GetTrackers () const;
};

#endif

