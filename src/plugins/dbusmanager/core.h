#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QDBusConnection>
#include <QStringList>

class Core : public QObject
{
	Q_OBJECT

	std::auto_ptr<QDBusConnection> Connection_;

	Core ();
public:
	static Core& Instance ();
	void Release ();
	QString Greeter (const QString&);
	QStringList GetLoadedPlugins ();
private:
	void DumpError ();
signals:
	void aboutToQuit ();
	void someEventHappened (const QString&);
};

#endif

