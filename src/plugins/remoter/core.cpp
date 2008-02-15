#include "core.h"
#include "server.h"

Core::Core ()
{
    Server::Instance ().setParent (this);
}

Core& Core::Instance ()
{
    static Core inst;
    return inst;
}

void Core::Release ()
{
    Server::Instance ().Release ();
    Server::Instance ().setParent (0);
}

void Core::AddObject (QObject *object, const QString& feature)
{
    if (feature == "remoteable")
        Objects_.append (object);
}

Reply Core::GetReplyFor (const QString& path, const QMap<QString, QString>&)
{
    Reply rep;
    rep.State_ = StateOK;
    rep.Data_ = QString ("<html><head><title>LeechCraft Remoter deep alpha</title></head><body><strong>You requested:</strong> <code>%1</code>, all's okey cuz I haven't even segfaulted :)</body></html>").arg (path).toUtf8 ();
    return rep;
}

