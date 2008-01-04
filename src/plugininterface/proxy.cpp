#include <QCoreApplication>
#include "../mainwindow.h"
#include "proxy.h"
#include "tcpsocket.h"
#include "filewriter.h"

Proxy *Proxy::Instance_ = 0;

Proxy::Proxy ()
{
	Strings_ << "bytes" << "KB" << "MB" << "GB";
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

void Proxy::SetStrings (const QStringList& str)
{
	Strings_ = str;
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
	QString dString = Strings_ [0];
    long double size = sourcesize;
    if (size >= 1024)
    {
		dString = Strings_ [1];
        size /= 1024;
    }
    if (size >= 1024)
    {
		dString = Strings_ [2];
        size /= 1024;
    }
    if (size >= 1024)
    {
		dString = Strings_ [3];
        size /= 1024;
    }

    return QString::number (size, 'f', 1) + " " + dString;
}

QTime Proxy::MakeTimeFromLong (ulong time) const
{
	int h = time / 3600;
	int m = (time - h * 3600) / 60;
	int s = time - h * 3600 - m * 60;
	return QTime (h, m, s);
}

void Proxy::AddUploadMessage (const QString& msg) const
{
	emit addMessage (msg, true);
}

void Proxy::AddDownloadMessage (const QString& msg) const
{
	emit addMessage (msg, false);
}

