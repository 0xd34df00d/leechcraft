#ifndef WORKINGTHREAD_H
#define WORKINGTHREAD_h
#include <QThread>
#include <QPair>
#include <QString>

class TcpSocket;

class WorkingThread : public QThread
{
 Q_OBJECT

 QString Address_, Login_, Password_, Destination_;
 int Port_;

 TcpSocket *Socket_;
public:
 WorkingThread (QObject *parent = 0);
 void SetHost (const QString&, int);
 void SetAuth (const QString&, const QString&);
 void SetDest (const QString&);
protected:
 virtual void run ();
private:
 bool MainLoop ();
 void RetrieveSingleMessage (int);
 QPair<bool, QString> ReadReply ();
signals:
 void error (const QString&);
 void log (const QString&);
 void finished (bool);
 void mailProgress (int);
 void dataProgress (int);
 void totalMail (int);
 void totalData (int);
};

#endif

