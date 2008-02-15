#include <QStringList>
#include <interfaces/interfaces.h>
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
    qDebug () << Q_FUNC_INFO << feature << object;
    if (feature == "remoteable" && qobject_cast<IRemoteable*> (object))
        Objects_.append (object);
}

Reply Core::GetReplyFor (const QString& p, const QMap<QString, QString>& query)
{
    QStringList parts = QString (p).remove (0, 1).split ('/'); // Trim first slash
    Reply rep;
    if (parts.size () == 0 || parts.at (0).isEmpty () || parts.at (0) == "index")
        rep = DoMainPage (parts, query);
    else if (parts.at (0) == "view")
        rep = DoView (parts, query);
    else
        rep = DoUnhandled (parts, query);

    return rep;
}

Reply Core::DoMainPage (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateOK;
    rep.Data_ = Head ("LeechCraft Remoter");
    QString body = "Here are possible options for you:<br />";
    body += Link ("View current jobs", "/view");
    rep.Data_ += Body (body);
    return rep;
}

Reply Core::DoView (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateOK;
    rep.Data_ = Head ("LeechCraft Remoter: View");
    rep.Data_ += Body ("this is not implemented yet");
    return rep;
}

Reply Core::DoUnhandled (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateOK;
    rep.Data_ = Head ("LeechCraft Remoter: unhandled page") + Body ("You should never see this, report to bugtracker");
    return rep;
}

QString Core::Head (const QString& title) const
{
    return QString ("<html><head><title>%1</title></head>").arg (title);
}

QString Core::Body (const QString& body) const
{
    return QString ("<body><a href=\"/index\">Main page</a><br /><hr /><br />%2</body></html>").arg (body);
}

QString Core::Link (const QString& title, const QString& where, bool newWindow) const
{
    QString string = QString ("<a href=\"%1\")").arg (where);
    if (newWindow)
        string.append (" target=\"_blank\">");
    else
        string.append (">");
    string.append (title + "</a>");
    return string;
}

