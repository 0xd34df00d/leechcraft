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
	ImpBase *ProtoImp_;
	JobParams* Params_;
	bool ErrorFlag_;
	QString ErrorReason_;
	ImpBase::length_t DownloadedSize_, TotalSize_, RestartPosition_;
	double Speed_;
	QMap<QAbstractSocket::SocketError, QString> ErrorDictionary_;
	QFile *File_;
	int DataOperations_;
	QTime *StartTime_;
	FileExistsDialog *FileExistsDialog_;
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
	void Run ();
	void Release ();
	enum JobType { File, Directory };
private slots:
	void handleNewFiles (QStringList*);
	void handleClarifyURL (QString);
	void processData (ImpBase::length_t, ImpBase::length_t, QByteArray);
	void reemitFinished ();
	void handleShowError (QString);
	void reemitStopped ();
	void reemitEnqueue ();
signals:
	void updateDisplays (unsigned int);
	void started (unsigned int);
	void finished (unsigned int);
	void deleteJob (unsigned int);
	void addJob (JobParams*);
	void showError (QString, QString);
	void stopped (unsigned int);
	void enqueue (unsigned int);
private:
	JobType JobType_;

	void FillErrorDictionary ();
	void OpenFile ();
	QString MakeUniqueNameFor (const QString&);
	QString MakeFilename (const QString&, QString&) const;
};

#endif

