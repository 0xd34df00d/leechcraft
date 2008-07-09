#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>
#include <QtConcurrentRun>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include "core.h"
#include "server.h"

Core::Core ()
{
    Server::Instance ().setParent (this);
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("Remoter");
    int port = settings.value ("Port", 0).toInt ();
    Login_ = settings.value ("Login", "default").toString ();
    Password_ = settings.value ("Password", "default").toString ();
    settings.endGroup ();
}

Core& Core::Instance ()
{
    static Core inst;
    return inst;
}

void Core::Release ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("Remoter");
    settings.setValue ("Port", GetPort ());
    settings.setValue ("Login", GetLogin ());
    settings.setValue ("Password", GetPassword ());
    settings.endGroup ();
    Server::Instance ().Release ();
    Server::Instance ().setParent (0);
}

void Core::SetPort (int port)
{
    qWarning () << Q_FUNC_INFO << Q_FUNC_INFO << "Currently setting port requires restart.";
//    emit bindSuccessful (Server::Instance ().Listen (port));
}

int Core::GetPort () const
{
    if (!Initialized_)
        return -1;
    int port = Server::Instance ().GetPort ();
    return port ? port : 14600;
}

void Core::SetLogin (const QString& login)
{
    Login_ = login;
}

const QString& Core::GetLogin () const
{
    if (!Initialized_)
        tr ("Systems failed to initialize");
    return Login_;
}

void Core::SetPassword (const QString& password)
{
    Password_ = password;
}

const QString& Core::GetPassword () const
{
    return Password_;
}

void Core::AddObject (QObject *object, const QString& feature)
{
    if (feature == "remoteable" && qobject_cast<IRemoteable*> (object) && qobject_cast<IInfo*> (object))
        Objects_.append (object);
}

