#ifndef COLLECTORWIDGET_H
#define COLLECTORWIDGET_H
#include <QWidget>

class CollectorWidget : public QWidget
{
 Q_OBJECT
public:
 CollectorWidget (QWidget *parent = 0);
signals:
 void collected ();
};

#endif

