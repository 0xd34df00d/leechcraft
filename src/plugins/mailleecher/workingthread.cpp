#include <QHostInfo>
#include <QCryptographicHash>
#include <QStringList>
#include <QMap>
#include <QFile>
#include <plugininterface/tcpsocket.h>
#include <plugininterface/socketexceptions.h>
#include "workingthread.h"

WorkingThread::WorkingThread (QObject *parent)
: QThread (parent)
{
}

void WorkingThread::SetHost (const QString& host, int port)
{
	Address_ = host;
	Port_ = port;
}

void WorkingThread::SetAuth (const QString& login, const QString& password)
{
	Login_ = login;
	Password_ = password;
}

void WorkingThread::SetDest (const QString& dest)
{
	Destination_ = dest;
	if (!Destination_.endsWith ('/'))
		Destination_.append ('/');
}

void WorkingThread::run ()
{
	QHostInfo info = QHostInfo::fromName (Address_);
	if (info.error () != QHostInfo::NoError)
	{
		emit error (tr ("Host lookup failed"));
		emit finished (false);
		return;
	}

	Socket_ = new TcpSocket;
	Socket_->SetDefaultTimeout (30000);
	try
	{
		Socket_->Connect (Address_, Port_);
	}
	catch (const Exceptions::Socket::BaseSocket& s)
	{
		emit error (tr ("Connect failed: %1").arg (s.GetReason ().c_str ()));
		emit finished (false);
		return;
	}

	Socket_->SetDefaultTimeout (10000);
	bool result = false;
	try
	{
		result = MainLoop ();
	}
	catch (const Exceptions::Socket::BaseSocket& s)
	{
		qWarning () << Q_FUNC_INFO << "Caught an exception";
		emit error (tr ("Main loop failed: %1").arg (s.GetReason ().c_str ()));
	}
	emit finished (result);

	Socket_->SetDefaultTimeout (30000);
	Socket_->Disconnect ();
}

bool WorkingThread::MainLoop ()
{
	if (!ReadReply ().first)
		return false;

//	Socket_->Write ("APOP " + Login_ + " " + QCryptographicHash::hash (Password_.toUtf8 (), QCryptographicHash::Md5).toHex () + "\r\n", false);
	Socket_->Write ("USER " + Login_ + "\r\n");
	ReadReply ();
	Socket_->Write ("PASS " + Password_ + "\r\n", false);
	if (!ReadReply ().first)
		return false;

	Socket_->Write (QString ("STAT\r\n"), false);
	QPair<bool, QString> reply = ReadReply ();
	if (!reply.first)
		return false;
	QStringList parts = reply.second.trimmed ().split (' ');
	int numMessages = parts [1].toInt ();
	emit totalMail (numMessages);

	quint64 totalSize = 0;
	QMap<int, quint64> msg2Size;
	for (int i = 0; i < numMessages; ++i)
	{
		Socket_->Write ("LIST " + QString::number (i + 1) + "\r\n", false);
		QStringList rparts = QString (Socket_->ReadLine ().trimmed ()).split (' ');
		if (rparts [0] [0] != '+')
			return false;

		msg2Size [i] = rparts [2].toInt ();
		totalSize += msg2Size [i];
	}
	emit totalData (totalSize);

	quint64 downloadedSize = 0;
	for (int i = 0; i < numMessages; ++i)
	{
		RetrieveSingleMessage (i + 1);
		downloadedSize += msg2Size [i];
		qDebug () << downloadedSize << msg2Size [i] << totalSize;
		emit mailProgress (i + 1);
		emit dataProgress (downloadedSize);
	}

	Socket_->Write (QString ("QUIT\r\n"), false);
	return true;
}

void WorkingThread::RetrieveSingleMessage (int id)
{
	Socket_->Write ("RETR " + QString::number (id) + "\r\n", false);
	if (!ReadReply ().first)
		throw Exceptions::Socket::BaseSocket (std::string ("could not RETR message"));

	QByteArray data;

	while (true)
	{
		QByteArray line = Socket_->ReadLine ();
		if (line.size () == 1 && line [0] == '.')
			break;
		if (line.size () == 3 && line [0] == '.')
			break;
		if (line [0] == '.')
		{
			line = line.right (line.size () - 1);
			if (line [0] == '.')
				line = line.right (line.size () - 1);
		}

		data.append (line);
	}

	QFile file (Destination_ + QString::number (id) + ".msg");
	if (!file.open (QIODevice::WriteOnly))
		throw Exceptions::Logic (std::string ("Could not open file for write"));

	file.write (data);
}

QPair<bool, QString> WorkingThread::ReadReply ()
{
	QString reply = Socket_->ReadLine ();
	bool r = (reply [0] == '+');
	return qMakePair (r, reply);
}

