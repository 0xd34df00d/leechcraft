#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include "reply.h"

class QStringList;

class Core : public QObject
{
    Q_OBJECT

    QObjectList Objects_;
    QString Login_, Password_;
    Core ();
public:
    static Core& Instance ();
    void Release ();

    void SetPort (int);
    int GetPort () const;
    void SetLogin (const QString&);
    const QString& GetLogin () const;
    void SetPassword (const QString&);
    const QString& GetPassword () const;

    void AddObject (QObject*, const QString& feature);
    Reply GetReplyFor (const QString&, const QMap<QString, QString>&, const QMap<QString, QString>&, const QByteArray&);
private:
    Reply DoNotAuthorized ();
    Reply DoMainPage (const QStringList&, const QMap<QString, QString>&);
    Reply DoView (const QStringList&, const QMap<QString, QString>&);
    Reply DoAdd (const QStringList&, const QMap<QString, QString>&, const QByteArray&);
    Reply DoUnhandled (const QStringList&, const QMap<QString, QString>&);
    Reply DoResources (const QStringList&, const QMap<QString, QString>&);
signals:
    void bindSuccessful (bool);
};

#endif

