#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#include <QWidget>
#include <QList>
#include "config.h"

class GraphWidget : public QWidget
{
    Q_OBJECT

    QList<quint64> Speeds_;
    QColor Color_;
public:
    LEECHCRAFT_API GraphWidget (const QColor&, QWidget *parent = 0);

    LEECHCRAFT_API void PushSpeed (quint64);
protected:
    virtual void paintEvent (QPaintEvent*);
};

#endif

