#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QDBusConnection>

class Adaptor;

class Core : public QObject
{
	Q_OBJECT

	Adaptor *Adaptor_;
	std::auto_ptr<QDBusConnection> Connection_;

	Core ();
public:
	static Core& Instance ();
public slots:
	void Greeter (const QString&);
private:
	void DumpError ();
signals:
	void aboutToQuit ();
};

#endif

