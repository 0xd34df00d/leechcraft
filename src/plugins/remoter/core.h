#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include <QPair>
#include <QFuture>
#include "reply.h"

class QStringList;

class Core : public QObject
{
    Q_OBJECT

    QObjectList Objects_;
    QString Login_, Password_;
    Core ();

    bool Initialized_;
	QFuture<int> Future_;
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
signals:
    void bindSuccessful (bool);
};

#endif

