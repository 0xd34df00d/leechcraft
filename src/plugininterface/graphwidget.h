#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#include <QWidget>
#include <QList>

class GraphWidget : public QWidget
{
    Q_OBJECT

    QList<quint64> Speeds_;
    QColor Color_;
public:
    GraphWidget (const QColor&, QWidget *parent = 0);

    void PushSpeed (quint64);
protected:
    virtual void paintEvent (QPaintEvent*);
};

#endif

