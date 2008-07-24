#ifndef TASK_H
#define TASK_H
#include <list>
#include <QObject>
#include <QUrl>
#include <QTime>

class QHttp;
class QFtp;
class Hook;
class QAuthenticator;
class QNetworkProxy;
class QIODevice;

class Task : public QObject
{
	Q_OBJECT
	QHttp *Http_;
	QFtp *Ftp_;
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
	qint64 Done_, Total_;
	double Speed_;
public:
	explicit Task (const QString& = QString ());
	virtual ~Task ();
	void AddHook (const Hook&);
	void RemoveHook (const Hook&);
	void Start (QIODevice*);
	void Stop ();

	double GetSpeed () const;
	qint64 GetDone () const;
	qint64 GetTotal () const;
	QString GetState () const;
	QString GetURL () const;
	int GetTimeFromStart () const;
	bool IsRunning () const;
private:
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
signals:
	void authenticationRequired (const QString&, quint16,
			QAuthenticator*);
	void proxyAuthenticationRequired (const QNetworkProxy&,
			QAuthenticator*);
	void updateInterface ();
	void done (bool);
};

#endif

