#ifndef JOB_H
#define JOB_H
#include <QMap>
#include <QPair>
#include <QAbstractSocket>
#include "impbase.h"

struct JobParams;
struct JobRepresentation;
class QTime;
class QTimer;
class QFile;
class FileExistsDialog;

class Job : public QObject
{
    Q_OBJECT

    bool ErrorFlag_, GetFileSize_;
    double Speed_, CurrentSpeed_;
    quint32 DownloadTime_;
    ImpBase::length_t DownloadedSize_, TotalSize_, RestartPosition_, PreviousDownloadSize_;
    ImpBase *ProtoImp_;
    JobParams* Params_;
    QString ErrorReason_;
    QMap<QAbstractSocket::SocketError, QString> ErrorDictionary_;
    QFile *File_;
    QTime *StartTime_, *UpdateTime_;
    FileExistsDialog *FileExistsDialog_;
    QByteArray Cache_;
    QTimer *FlushTimer_;
public:
    enum State
    {
        StateIdle
        , StateDownloading
        , StateWaiting
    };
    enum JobType { File, Directory };
private:
    State State_;
public:
    Job (JobParams *params = 0, QObject *parent = 0);
    virtual ~Job ();
    void DoDelayedInit ();
    const QString& GetURL () const;
    QString GetLocalName () const;
    long GetSpeed () const;
    long GetCurrentSpeed () const;
    quint64 GetDownloaded () const;
    quint64 GetTotal () const;
    long GetAverageTime () const;
    long GetCurrentTime () const;
    long GetDownloadTime () const;
    bool GetErrorFlag ();
    QString GetErrorReason ();
    void Start ();
    void Stop ();
    void GetFileSize ();
    void Release ();
    State GetState () const;
    void UpdateParams (JobParams*);
    QByteArray Serialized () const;
    void Unserialize (const QByteArray&);
private slots:
    void handleRemoteFileInfo (const ImpBase::RemoteFileInfo&);
    void handleNewFiles (QStringList*);
    void handleClarifyURL (QString);
    void processData (ImpBase::length_t, ImpBase::length_t, QByteArray);
    void handleFinished ();
    void handleShowError (QString);
    void handleStopped ();
    void handleEnqueue ();
    void handleGotFileSize (ImpBase::length_t);
signals:
    void updateDisplays ();
    void started ();
    void finished ();
    void deleteJob ();
    void addJob (JobParams*);
    void showError (QString, QString);
    void stopped ();
    void enqueue ();
    void gotFileSize ();
private:
    JobType JobType_;

    void FillErrorDictionary ();
    void OpenFile ();
    QString MakeUniqueNameFor (const QString&);
    QString MakeFilename () const;
};

#endif

