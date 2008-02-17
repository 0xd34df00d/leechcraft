#ifndef CORE_H
#define CORE_H
#include <QObjectList>
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
    Reply GetReplyFor (const QString&, const QMap<QString, QString>&, const QMap<QString, QString>&);
private:
    Reply DoNotAuthorized ();
    Reply DoMainPage (const QStringList&, const QMap<QString, QString>&);
    Reply DoView (const QStringList&, const QMap<QString, QString>&);
    Reply DoUnhandled (const QStringList&, const QMap<QString, QString>&);

    QString Row (const QVariantList&);
    QString Head (const QString&) const;
    QString Body (const QString&) const;
    QString Link (const QString&, const QString&, bool n = false) const;
    QString Heading (const QString&, int level = 1);
    QString Strong (const QString&);
    QString Table (const QString&);
signals:
    void bindSuccessful (bool);
};

#endif

