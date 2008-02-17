#include <QStringList>
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
    emit bindSuccessful (Server::Instance ().listen (QHostAddress::Any, port ? port : 14600));
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
    emit bindSuccessful (Server::Instance ().listen (QHostAddress::Any, port));
}

int Core::GetPort () const
{
    int port = Server::Instance ().serverPort ();
    return port ? port : 14600;
}

void Core::SetLogin (const QString& login)
{
    Login_ = login;
}

const QString& Core::GetLogin () const
{
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

Reply Core::GetReplyFor (const QString& p, const QMap<QString, QString>& query, const QMap<QString, QString>& headers)
{
    QStringList parts = QString (p).remove (0, 1).split ('/'); // Trim first slash
    Reply rep;

    QStringList base64 = headers ["authorization"].split (' ');
    if (base64.size () != 2 || base64 [0].toLower () != "basic" || QByteArray::fromBase64 (base64 [1].toAscii ()) != QString ("%1:%2").arg (Login_).arg (Password_))
        rep = DoNotAuthorized ();
    else if (parts.size () == 0 || parts.at (0).isEmpty () || parts.at (0) == "index")
        rep = DoMainPage (parts, query);
    else if (parts.at (0) == "view")
        rep = DoView (parts, query);
    else
        rep = DoUnhandled (parts, query);

    return rep;
}

Reply Core::DoNotAuthorized ()
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateUnauthorized;
    rep.Data_ = "401 Unauthorized";
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
    QString body;
    for (int i = 0; i < Objects_.size (); ++i)
    {
        IRemoteable *ir = qobject_cast<IRemoteable*> (Objects_.at (i));
        QList<QVariantList> datas = ir->GetAll ();
        if (!datas.size ())
            continue;

        IInfo *ii = qobject_cast<IInfo*> (Objects_.at (i));
        body += Strong (Heading (ii->GetName ()));
        body += Heading (ii->GetInfo (), 3);

        QString text = Strong (Row (datas.at (0)));
        for (int i = 1; i < datas.size (); ++i)
        {
            text += Row (datas.at (i));
        }

        body += Table (text);

        body += "<hr />";
    }
    rep.Data_ = Head ("LeechCraft Remoter: View") + Body (body);
    return rep;
}

Reply Core::DoUnhandled (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateNotFound;
    rep.Data_ = Head ("LeechCraft Remoter: not found") + Body ("You should never see this until you type something yourself in the address bar.");
    return rep;
}

QString Core::Row (const QVariantList& list)
{
    QString result = "<tr>";
    for (int i = 0; i < list.size (); ++i)
    {
        result += "<td>";
        if (list.at (i).canConvert <QString> ())
            result += list.at (i).value<QString> ();
        else
            result += "(unconvertable, that sucks)";
        result += "</td>";
    }
    result += "</tr>";
    return result;
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

QString Core::Heading (const QString& text, int level)
{
    return QString ("<h%1>%2</h%1>").arg (level).arg (text);
}

QString Core::Strong (const QString& text)
{
    return QString ("<strong>%1</strong>").arg (text);
}

QString Core::Table (const QString& text)
{
    return QString ("<table border=\"1\">%1</table>").arg (text);
}

