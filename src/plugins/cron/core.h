#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QList>
#include <QMap>
#include <QVector>

class QDateTime;
class QTimer;

class Core : public QObject
{
    Q_OBJECT

    QVector<int> FreeIDs_;
    QMap<QTimer*, int> Singleshots_,
        Intervals_;
public:
    Core (QObject *parent = 0);
    void Release ();
    int AddSingleshot (const QDateTime&);
    int AddInterval (int);
private:
    void ReadSettings ();
private slots:
    void writeSettings ();
    void timeout ();
signals:
    void shot (int);
};

#endif

