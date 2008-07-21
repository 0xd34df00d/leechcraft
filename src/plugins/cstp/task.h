#ifndef TASK_H
#define TASK_H
#include <list>
#include <QObject>
#include <QUrl>

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
public:
	explicit Task (const QString& = QString ());
	virtual ~Task ();
	void AddHook (const Hook&);
	void RemoveHook (const Hook&);
	void Start (QIODevice*);
	void Stop ();
private:
	void Construct ();
	cmd_t FindCommand (int) const;
private slots:
	void handleRequestStart (int);
	void handleRequestFinish (int, bool);
signals:
	void stateChanged (int);
	void authenticationRequired (const QString&, quint16,
			QAuthenticator*);
	void proxyAuthenticationRequired (const QNetworkProxy&,
			QAuthenticator*);
	void dataTransferProgress (qint64, qint64);
	void done (bool);
};

#endif

