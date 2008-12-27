#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

class XmlSettingsManager : public LeechCraft::Util::BaseSettingsManager
{
    Q_OBJECT
public:
    XmlSettingsManager ();
    static XmlSettingsManager* Instance ();
protected:
    virtual QSettings* BeginSettings () const;
    virtual void EndSettings (QSettings*) const;
};

#endif

