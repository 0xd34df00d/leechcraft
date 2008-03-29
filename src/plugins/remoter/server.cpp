#include <QStringList>
#include <QTcpSocket>
#include <QUrl>
#include "server.h"
#include "reply.h"
#include "core.h"

Server::Server ()
{
    if (!listen (QHostAddress::Any, 14600))
        qWarning () << Q_FUNC_INFO << "listen failed";
    connect (this, SIGNAL (newConnection ()), this, SLOT (ready ()));
}

Server& Server::Instance ()
{
    static Server inst;
    return inst;
}

void Server::Release ()
{
    close ();
}

void Server::ready ()
{
    QTcpSocket *socket = nextPendingConnection ();
    if (!socket)
        return;

    if (!socket->bytesAvailable ())
        socket->waitForReadyRead ();
    QString line = socket->readLine ().trimmed ();
    if (line.isEmpty ())
        return;

    QStringList head = line.split (' ');
    QString path = head.at (1);
    QMap<QString, QString> query;
    if (path.contains ('?'))
    {
        qDebug () << "head contains '?'";
        QStringList params = path.split ('?').at (1).split ('&');
        qDebug () << "params:" << params;
        for (int i = 0; i < params.size (); ++i)
        {
            QStringList p = params.at (i).split ('=');
            if (p.size () < 2)
                continue;
            query [p.at (0)] = p.at (1);
        }
    }

    QMap<QString, QString> headers;

    while (!line.isEmpty ())
    {
        if (!socket->bytesAvailable ())
            socket->waitForReadyRead ();
        line = socket->readLine ().trimmed ();
        qDebug () << Q_FUNC_INFO << line;

        QStringList list = line.split (": ");
        if (list.size () != 2)
            continue;
        headers [list [0].trimmed ().toLower ()] = list [1].trimmed ();
    }

    QByteArray other;
    if (head [0].toLower () == "post" && headers.contains ("content-length"))
    {
        quint64 counter = 0, postSize = headers ["content-length"].toULongLong ();
        bool emptyLineEncountered = false;
        while (counter < postSize)
        {
            try
            {
                QByteArray tmp = socket->readAll ();
                counter += tmp.size ();
                other += tmp;
            }
            catch (...)
            {
            }
        }
        QList<QByteArray> splitted = other.split ('\n');
        int position = -1;
        for (int i = 0; i < splitted.size (); ++i)
            if (splitted.at (i).simplified ().isEmpty ())
            {
                position = i;
                break;
            }
        other.clear ();
        for (int i = position; i < splitted.size (); ++i)
            other += splitted.at (i);
        qDebug () << other;
    }
    Reply reply = qobject_cast<Core*> (parent ())->GetReplyFor (path, query, headers, other);
    socket->write (QString ("HTTP/1.0 " + QString::number (reply.State_) + " OK\r\n").toAscii ());
    socket->write ("Server: LeechCraftRemoter/deep_alpha\r\n");
    reply.Type_.isEmpty () ?
        socket->write ("Content-Type: text/html; charset=UTF-8\r\n") :
        socket->write (QString ("Content-Type: " + reply.Type_ + "\r\n").toAscii ());
    socket->write ("WWW-Authenticate: Basic realm=\"LeechCraft Remoter\"");
    socket->write (QString ("Content-Length: %1\r\n").arg (reply.Data_.toUtf8 ().size ()).toAscii ());
    socket->write ("\r\n");
    socket->write (reply.Data_.toUtf8 ());
    socket->disconnectFromHost ();
}

