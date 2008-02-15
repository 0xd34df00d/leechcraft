#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QMap>
#include <QString>
#include "reply.h"

class Core : public QObject
{
    Q_OBJECT

    QObjectList Objects_;

    Core ();
public:
    static Core& Instance ();
    void Release ();

    void AddObject (QObject*, const QString& feature);
    Reply GetReplyFor (const QString&, const QMap<QString, QString>&);
};

#endif

