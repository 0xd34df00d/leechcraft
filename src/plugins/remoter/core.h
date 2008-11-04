#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include <QPair>

class QStringList;

namespace Wt
{
	class WServer;
};

class Core : public QObject
{
    Q_OBJECT

    Core ();

	Wt::WServer *Server_;

    bool Initialized_;
public:
    static Core& Instance ();
    void Release ();
    void AddObject (QObject*, const QString& feature);
signals:
};

#endif

