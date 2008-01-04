#ifndef IMPBASE_H
#define IMPBASE_H
#include <QThread>
#include <QDateTime>
#include "fileexistsdialog.h"

class QString;
class Proxy;

class ImpBase : public QThread
{
	Q_OBJECT

	int CacheSize_;
	QByteArray Cache_;
public:
	typedef quint64 length_t;

	struct RemoteFileInfo
	{
		bool Valid_;
		QDateTime Modification_;
		length_t Size_;
		QString ContentType_;
	};

	ImpBase (QObject *parent = 0);
	virtual ~ImpBase ();
	virtual void SetRestartPosition (length_t) = 0;
	virtual void SetURL (const QString&) = 0;
	virtual void StartDownload ();
	virtual void StopDownload () = 0;
	virtual void ReactedToFileInfo () = 0;
	virtual void ScheduleGetFileSize () = 0;
	virtual void run () = 0;
protected:
	length_t RestartPosition_;
	QString URL_;

	void SetCacheSize (int);
	void Emit (length_t, length_t, QByteArray);
	void EmitFlush (length_t, length_t);
signals:
	void gotRemoteFileInfo (const ImpBase::RemoteFileInfo&);
	void gotFileSize (ImpBase::length_t);
	void dataFetched (ImpBase::length_t, ImpBase::length_t, QByteArray);
	void finished ();
	void stopped ();
	void warning (QString);
	void error (QString);
	void enqueue ();
};

#endif

