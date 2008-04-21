#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include <QPair>
#include "reply.h"

class QStringList;

namespace Poco
{
    namespace Net
    {
        class HTTPServerRequest;
    };
};

class Core : public QObject
{
    Q_OBJECT

    QObjectList Objects_;
    QString Login_, Password_;
    Core ();

    bool Initialized_;
public:
    struct PostEntity
    {
        QByteArray Data_;
        QMap<QString, QString> Metadata_;
    };
    static Core& Instance ();
    void Release ();

    void SetPort (int);
    int GetPort () const;
    void SetLogin (const QString&);
    const QString& GetLogin () const;
    void SetPassword (const QString&);
    const QString& GetPassword () const;

    void AddObject (QObject*, const QString& feature);

    bool IsAuthorized (const Poco::Net::HTTPServerRequest&) const;
    Reply GetReplyFor (const QString&, const QMap<QString, QString>&, const QList<PostEntity>&);
private:
    Reply DoMainPage (const QStringList&, const QMap<QString, QString>&);
    Reply DoView (const QStringList&, const QMap<QString, QString>&);
    Reply DoAdd (const QStringList&, const QMap<QString, QString>&, const QByteArray&);
    Reply DoUnhandled (const QStringList&, const QMap<QString, QString>&);
    Reply DoResources (const QStringList&, const QMap<QString, QString>&);
signals:
    void bindSuccessful (bool);
};

#endif

