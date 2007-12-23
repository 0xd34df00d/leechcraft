#ifndef JOB_H
#define JOB_H
#include <QMap>
#include <QPair>
#include <QAbstractSocket>
#include "impbase.h"

struct JobParams;
struct JobRepresentation;
class QTime;
class QFile;
class FileExistsDialog;

class Job : public QObject
{
	Q_OBJECT

	unsigned int ID_;
	bool ErrorFlag_, GetFileSize_;
	double Speed_, CurrentSpeed_;
	long DownloadTime_;
	ImpBase::length_t DownloadedSize_, TotalSize_, RestartPosition_, PreviousDownloadSize_;
	ImpBase *ProtoImp_;
	JobParams* Params_;
	QString ErrorReason_;
	QMap<QAbstractSocket::SocketError, QString> ErrorDictionary_;
	QFile *File_;
	QTime *StartTime_, *UpdateTime_;
	FileExistsDialog *FileExistsDialog_;
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
	void SetID (unsigned int);
	unsigned int GetID () const;
	JobRepresentation* GetRepresentation () const;
	bool GetErrorFlag ();
	QString GetErrorReason ();
	void Start ();
	void Stop ();
	void GetFileSize ();
	void Release ();
	State GetState () const;
private slots:
	void handleRemoteFileInfo (const ImpBase::RemoteFileInfo&);
	void handleNewFiles (QStringList*);
	void handleClarifyURL (QString);
	void processData (ImpBase::length_t, ImpBase::length_t, QByteArray);
	void reemitFinished ();
	void handleShowError (QString);
	void reemitStopped ();
	void reemitEnqueue ();
	void reemitGotFileSize (ImpBase::length_t);
signals:
	void updateDisplays (unsigned int);
	void started (unsigned int);
	void finished (unsigned int);
	void deleteJob (unsigned int);
	void addJob (JobParams*);
	void showError (QString, QString);
	void stopped (unsigned int);
	void enqueue (unsigned int);
	void gotFileSize (unsigned int);
private:
	JobType JobType_;

	void FillErrorDictionary ();
	void OpenFile ();
	QString MakeUniqueNameFor (const QString&);
	QString MakeFilename (const QString&, QString&) const;
};

#endif

