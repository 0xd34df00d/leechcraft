#ifndef PROXY_H
#define PROXY_H
#include <QObject>
#include <QStringList>
#include <QTime>
#include "../mainwindow.h"

class TcpSocket;
class FileWriter;

class Proxy : public QObject
{
	Q_OBJECT

	Proxy ();
	~Proxy ();

	Main::MainWindow *Window_;

	static Proxy *Instance_;
	QStringList Strings_;
	friend class Main::MainWindow;
public:
	static Proxy *Instance ();
	void SetStrings (const QStringList&);
	TcpSocket* MakeSocket () const;
	FileWriter* GetFileWriter () const;
	QString GetApplicationName () const;
	QString GetOrganizationName () const;
	QString MakePrettySize (qint64) const;
	QTime MakeTimeFromLong (ulong) const;
	void AddUploadMessage (const QString&) const;
	void AddDownloadMessage (const QString&) const;
	QMenu* GetRootPluginsMenu () const;
signals:
	void addMessage (const QString&, bool) const;
private:
	void SetMainWindow (Main::MainWindow*);
};

#endif

