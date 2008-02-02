#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <plugininterface/basesettingsmanager.h>

class XmlSettingsManager : public BaseSettingsManager
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

