#ifndef FTPIMP_H
#define FTPIMP_H
#include <QString>
#include <QPair>
#include <QDateTime>
#include <plugininterface/guarded.h>
#include "impbase.h"

class TcpSocket;
class QUrlInfo;
class QStringList;
class QMutex;
class QWaitCondition;
class QFileInfo;

class FtpImp : public ImpBase
{
    Q_OBJECT

    TcpSocket *ControlSocket_, *DataSocket_;
    QString LastReply_;
    length_t Size_;
    int Result_;
    Guarded<bool> Stop_, GetFileSize_;
    QPair<QWaitCondition*, QMutex*> AwaitFileInfoReaction_;
    QDateTime Modification_;
public:
    FtpImp (QObject *parent = 0);
    virtual ~FtpImp ();
    virtual void SetRestartPosition (length_t);
    virtual void SetURL (const QString&);
    virtual void StopDownload ();
    virtual void ReactedToFileInfo ();
    virtual void ScheduleGetFileSize (); 
private:
    virtual void run ();
    virtual bool Negotiate ();
    bool HandleWelcome ();
    void DoCwd (const QString&);
    bool DoSize (const QString&);
    void DoGetFileInfo (const QString&);
    void DoQuery (const QFileInfo&);
    void DoInitTransfer (const QString&);
    virtual bool Login (const QString&, const QString&);
    virtual int ReadCtrlResponse ();
    virtual void DoPasv ();
    virtual void PasvHandler ();
    void Finalize ();
signals:
    void gotNewFiles (QStringList*);
};

#endif

