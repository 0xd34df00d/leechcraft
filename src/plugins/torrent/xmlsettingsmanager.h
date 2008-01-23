#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <QObject>
#include <interfaces/interfaces.h>
#include <plugininterface/basicsettingsmanager.h>

class XmlSettingsManager : public BasicSettingsManager
{
    Q_OBJECT
public:
    XmlSettingsManager ();
    static XmlSettingsManager* Instance ();
protected:
    virtual QSettings* BeginSettings ();
    virtual void EndSettings (QSettings*);
};

#endif

