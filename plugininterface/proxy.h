#ifndef PROXY_H
#define PROXY_H
#include <QObject>
#include <QStringList>

class TcpSocket;
class FileWriter;

class Proxy : public QObject
{
	Q_OBJECT

	Proxy ();
	~Proxy ();

	static Proxy *Instance_;
	QStringList Strings_;
public:
	static Proxy *Instance ();
	void SetStrings (const QStringList&);
	TcpSocket* MakeSocket () const;
	FileWriter* GetFileWriter () const;
	QString GetApplicationName () const;
	QString GetOrganizationName () const;
	QString MakePrettySize (qint64) const;
	void AddUploadMessage (const QString&) const;
	void AddDownloadMessage (const QString&) const;
signals:
	void addMessage (const QString&, bool) const;
};

#endif

