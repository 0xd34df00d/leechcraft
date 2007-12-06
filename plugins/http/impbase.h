#ifndef IMPBASE_H
#define IMPBASE_H
#include <QThread>

class QString;
class Proxy;

class ImpBase : public QThread
{
	Q_OBJECT

	int CacheSize_;
	QByteArray Cache_;
public:
	typedef quint64 length_t;
	ImpBase (QObject *parent = 0);
	virtual ~ImpBase ();
	virtual void SetRestartPosition (length_t) = 0;
	virtual void SetURL (const QString&) = 0;
	virtual void StartDownload ();
	virtual void StopDownload () = 0;
	virtual void run () = 0;
protected:
	length_t RestartPosition_;
	QString URL_;

	void SetCacheSize (int);
	void Emit (length_t, length_t, QByteArray);
	void EmitFlush (length_t, length_t);
signals:
	void dataFetched (ImpBase::length_t ready, ImpBase::length_t total, QByteArray data);
	void finished ();
	void stopped ();
	void warning (QString);
	void error (QString);
};

#endif

