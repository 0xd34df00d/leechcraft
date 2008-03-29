#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include "core.h"
#include "server.h"
#include "documentgenerator.h"

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

Reply Core::GetReplyFor (const QString& p, const QMap<QString, QString>& query, const QMap<QString, QString>& headers, const QByteArray& postData)
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
    else if (parts.at (0) == "add")
        rep = DoAdd (parts, query, postData);
    else if (parts.at (0) == "resources")
        rep = DoResources (parts, query);
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

    DocumentGenerator::CreateDocument ();
    DocumentGenerator::CreateHead (QString ("Main Page : LeechCraft Remoter"));

    QDomDocument document = DocumentGenerator::GetDocument ();
    QDomNode body = document.elementsByTagName ("body").at (0);
    body.appendChild (DocumentGenerator::CreateDefaultHeading ());
    body.appendChild (DocumentGenerator::CreateLink ("/view", "View running jobs from all plugins"));

    rep.Data_ += document.toByteArray ();
    return rep;
}

Reply Core::DoView (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateOK;

    DocumentGenerator::CreateDocument ();
    DocumentGenerator::CreateHead (QString ("View : LeechCraft Remoter"));
    DocumentGenerator::AddAutorefresh (10);

    QDomDocument document = DocumentGenerator::GetDocument ();
    QDomNode body = document.elementsByTagName ("body").at (0);
    body.appendChild (DocumentGenerator::CreateDefaultHeading ());

    for (int i = 0; i < Objects_.size (); ++i)
    {
        IRemoteable *ir = qobject_cast<IRemoteable*> (Objects_.at (i));
        QList<QVariantList> datas = ir->GetAll ();
        if (!datas.size ())
            continue;

        IInfo *ii = qobject_cast<IInfo*> (Objects_.at (i));
        QDomElement strongName = DocumentGenerator::CreateInlineText (ii->GetName ());
        DocumentGenerator::ApplyClass (strongName, "heading");
        QDomElement descr = DocumentGenerator::CreateInlineText (" (" + ii->GetInfo () + ")");
        DocumentGenerator::ApplyClass (descr, "emed");
        QDomElement text = DocumentGenerator::CreateText ("");
        text.appendChild (strongName);
        text.appendChild (descr);
        body.appendChild (text);

        if (!datas.size ())
            continue;

        // Draw new job form
        QDomElement form = DocumentGenerator::CreateForm (QString ("/add/%1").arg (i), true);
        DocumentGenerator::InputType type;
        QDomElement addEntity = DocumentGenerator::CreateInputField (DocumentGenerator::TypeTextbox, "entity");
        QDomElement where = DocumentGenerator::CreateInputField (DocumentGenerator::TypeText, "where");
        QDomElement addHolder = DocumentGenerator::CreateText (),
                    whereHolder = DocumentGenerator::CreateText (),
                    submitHolder = DocumentGenerator::CreateText ();
        addHolder.appendChild (DocumentGenerator::CreateInlineText ("What to add:"));
        addHolder.appendChild (addEntity);
        whereHolder.appendChild (DocumentGenerator::CreateInlineText ("Where to save:"));
        whereHolder.appendChild (where);
        submitHolder.appendChild (DocumentGenerator::CreateSubmitButton ("Add!"));
        form.appendChild (addHolder);
        form.appendChild (whereHolder);
        form.appendChild (submitHolder);
        body.appendChild (form);

        // Create view for current jobs

        QVariantList heading = datas.at (0);
        heading.removeAt (0);

        for (int j = 1; j < datas.size (); ++j)
        {
            QString name = datas.at (j).at (0).toString ();
            QVariantList other = datas.at (j);
            other.removeAt (0);
            QDomElement n = DocumentGenerator::CreateText (name);
            DocumentGenerator::ApplyClass (n, "jobName");
            QDomElement table = DocumentGenerator::CreateTable ();
            QDomElement headrow = DocumentGenerator::CreateRow (heading);
            DocumentGenerator::ApplyClass (headrow, "heading");
            QDomElement row = DocumentGenerator::CreateRow (other);
            table.appendChild (headrow);
            table.appendChild (row);
            body.appendChild (n);
            body.appendChild (table);
        }
    }

    rep.Data_ += document.toByteArray ();
    return rep;
}

Reply Core::DoAdd (const QStringList& path, const QMap<QString, QString>& query, const QByteArray& postData)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateFound;
    rep.RedirectTo_ = "/view";

    int number = path [1].toInt ();
    if (number >= Objects_.size ())
        return rep;

    QByteArray entity = QUrl::fromPercentEncoding (query ["entity"].toAscii ()).toUtf8 ();
    QString where = query ["where"];

    IRemoteable *ir = qobject_cast<IRemoteable*> (Objects_.at (number));
//    ir->AddJob (postData, where);

    return rep;
}

Reply Core::DoUnhandled (const QStringList&, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply rep;
    rep.State_ = StateNotFound;
    DocumentGenerator::CreateDocument ();
    DocumentGenerator::CreateHead (QString ("Not Found : LeechCraft Remoter"));

    QDomDocument document = DocumentGenerator::GetDocument ();
    QDomNode body = document.elementsByTagName ("body").at (0);
    body.appendChild (DocumentGenerator::CreateHeading ("LeechCraft Remoter deep alpha", 2));
    body.appendChild (DocumentGenerator::CreateLink ("/", "Go back to main"));
    rep.Data_ = document.toByteArray ();
    return rep;
}

Reply Core::DoResources (const QStringList& path, const QMap<QString, QString>&)
{
    qDebug () << Q_FUNC_INFO;
    Reply result;
    if (path [1] == "stylesheet")
    {
        result.State_ = StateOK;
        result.Type_ = "text/css";
        result.Data_ = DocumentGenerator::GetStylesheet ();
    }
    return result;
}

