#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <QObject>
#include <interfaces/interfaces.h>

class QSettings;

class XmlSettingsManager : public QObject
{
    Q_OBJECT

    QMap<QByteArray, QPair<QObject*, QByteArray> > Properties2Object_;
    bool Initializing_;
public:
    XmlSettingsManager ();
    static XmlSettingsManager* Instance ();
    void Release ();
    void RegisterObject (const QByteArray& propName, QObject* object, const QByteArray& funcName);
    QVariant Property (const QString& propName, const QVariant& def);
protected:
    virtual bool event (QEvent*);
    virtual QSettings* BeginSettings ();
    virtual void EndSettings (QSettings*);
};

#endif

