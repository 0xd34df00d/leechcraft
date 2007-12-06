#include <QCoreApplication>
#include "../mainwindow.h"
#include "proxy.h"
#include "tcpsocket.h"
#include "filewriter.h"

Proxy *Proxy::Instance_ = 0;

Proxy::Proxy ()
{
}

Proxy::~Proxy ()
{
}

Proxy* Proxy::Instance ()
{
	if (!Instance_)
		Instance_ = new Proxy;
	return Instance_;
}

TcpSocket* Proxy::MakeSocket () const
{
	return new TcpSocket;
}

FileWriter* Proxy::GetFileWriter () const
{
	return FileWriter::Instance ();
}

QString Proxy::GetApplicationName () const
{
	return QCoreApplication::applicationName ();
}

QString Proxy::GetOrganizationName () const
{
	return QCoreApplication::organizationName ();
}

QString Proxy::MakePrettySize (qint64 sourcesize) const
{
	QString dString = tr (" bytes");
    long double size = sourcesize;
    if (size >= 1024)
    {
        dString = tr (" KB");
        size /= 1024;
    }
    if (size >= 1024)
    {
        dString = tr (" MB");
        size /= 1024;
    }
    if (size >= 1024)
    {
        dString = tr (" GB");
        size /= 1024;
    }

    return QString::number (size, 'f', 1) + dString;
}

void Proxy::AddUploadMessage (const QString& msg) const
{
	emit addMessage (msg, true);
}

void Proxy::AddDownloadMessage (const QString& msg) const
{
	emit addMessage (msg, false);
}

