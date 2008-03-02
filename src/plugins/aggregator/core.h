#ifndef CORE_H
#define CORE_H
#include <QObject>

class Core : public QObject
{
    Q_OBJECT

    Core ();
public:
    static Core& Instance ();
    void Release ();

    void SetProvider (QObject*, const QString&);
};

#endif

