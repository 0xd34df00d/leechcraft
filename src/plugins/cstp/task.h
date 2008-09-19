#ifndef TASK_H
#define TASK_H
#include <list>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <QObject>
#include <QUrl>
#include <QTime>
#include <QHttp>

class QHttp;
class QFtp;
class Hook;
class QAuthenticator;
class QNetworkProxy;
class QFile;
class QIODevice;

class Task : public QObject
{
	Q_OBJECT
	std::auto_ptr<QHttp> Http_;
	std::auto_ptr<QFtp> Ftp_;
	QUrl URL_;
	QTime StartTime_;

	enum Type
	{
		TInvalid
		, THttp
		, THttps
		, TFtp
	};
	Type Type_;

	enum Command
	{
		CInvalid
		, CUnknown
		, CLogin
		, CTypeI
		, CRest
		, CConnect
		, CCD
		, CTransfer
		, CDisconnect
	};
public:
	typedef std::pair<int, Command> cmd_t;
private:
	typedef std::list<cmd_t> cmds_t;
	cmds_t Commands_;
	cmd_t CurrentCmd_;

	std::list<Hook> Hooks_;
	qint64 Done_, Total_, FileSizeAtStart_;
	double Speed_;
	QStringList RedirectHistory_;
public:
	explicit Task (const QString& = QString ());
	virtual ~Task ();
	void AddHook (const Hook&);
	void RemoveHook (const Hook&);
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
	void SetProxy (const QNetworkProxy&);
private:
	void Start (QIODevice*);
	void Reset ();
	void Construct ();
	void ConstructFTP (const QString& = QString ("ftp"));
	void ConstructHTTP (const QString& = QString ("http"));
	cmd_t FindCommand (int) const;
	void RecalculateSpeed ();
	QString GetHTTPState () const;
	QString GetFTPState () const;
private slots:
	void handleRequestStart (int);
	void handleRequestFinish (int, bool);
	void handleDataTransferProgress (qint64, qint64);
	void handleDataTransferProgress (int, int);
	void responseHeaderReceived (const QHttpResponseHeader&);
	void redirectedConstruction (QIODevice*, const QString&);
signals:
	void authenticationRequired (const QString&, quint16,
			QAuthenticator*);
	void proxyAuthenticationRequired (const QNetworkProxy&,
			QAuthenticator*);
	void updateInterface ();
	void done (bool);
};

#endif

