#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QDateTime>
#include <QList>
#include <QPair>

class Core : public QObject
{
 Q_OBJECT

 int TimerID_;
 QList<quint64> UsedIDs_;
 QList<QPair<QDateTime, quint64> > SingleShots_;
public:
 Core (QObject *parent = 0);
 void Release ();
 quint64 AddSingleShot (QDateTime);
protected:
 virtual void timerEvent (QTimerEvent*);
private:
 void ReadSettings ();
private slots:
 void writeSettings ();
signals:
 void shot (quint64);
};

#endif

