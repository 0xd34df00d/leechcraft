#ifndef TASK_H
#define TASK_H
#include <list>
#include <boost/intrusive_ptr.hpp>
#include <memory>
#include <QObject>
#include <QUrl>
#include <QTime>
#include <QNetworkReply>
#include <QStringList>
#include "morphfile.h"

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
	boost::intrusive_ptr<MorphFile> To_;
	int Counter_;
public:
	explicit Task (const QUrl& = QUrl ());
	explicit Task (QNetworkReply*);
	void Start (const boost::intrusive_ptr<MorphFile>&);
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

	void AddRef ();
	void Release ();
private:
	void Reset ();
	void RecalculateSpeed ();
private slots:
	void handleDataTransferProgress (qint64, qint64);
	void redirectedConstruction (const QString&);
	void handleMetaDataChanged ();
	/** Returns true if the reply is at end after this read.
	 */
	bool handleReadyRead ();
	void handleFinished ();
	void handleError ();
signals:
	void updateInterface ();
	void done (bool);
};

void intrusive_ptr_add_ref (Task*);
void intrusive_ptr_release (Task*);

#endif

