#ifndef TASK_H
#define TASK_H
#include <list>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <QObject>
#include <QUrl>
#include <QTime>
#include <QNetworkReply>
#include <QStringList>

class Hook;
class QAuthenticator;
class QNetworkProxy;
class QIODevice;
class QFile;

class Task : public QObject
{
	Q_OBJECT
	std::auto_ptr<QNetworkReply> Reply_;
	QUrl URL_;
	QTime StartTime_;
	qint64 Done_, Total_, FileSizeAtStart_;
	double Speed_;
	QStringList RedirectHistory_;
	QIODevice *To_;
public:
	explicit Task (const QString& = QString ());
	virtual ~Task ();
	void Start (const boost::shared_ptr<QFile>&);
	void Stop ();

	QByteArray Serialize () const;
	void Deserialize (QByteArray&);

	double GetSpeed () const;
	qint64 GetDone () const;
	qint64 GetTotal () const;
	QString GetState () const;
	QString GetURL () const;
	int GetTimeFromStart () const;
	bool IsRunning () const;
	QString GetErrorString () const;
private:
	void Start (QIODevice*);
	void Reset ();
	void RecalculateSpeed ();
private slots:
	void handleDataTransferProgress (qint64, qint64);
	void redirectedConstruction (QIODevice*, const QString&);
	void handleMetaDataChanged ();
	void handleReadyRead ();
	void handleFinished ();
	void handleError ();
signals:
	void updateInterface ();
	void done (bool);
};

#endif

