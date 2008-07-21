#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <plugininterface/basesettingsmanager.h>

class XmlSettingsManager : public BaseSettingsManager
{
	Q_OBJECT
	XmlSettingsManager ();
public:
	static XmlSettingsManager& Instance ();
protected:
	virtual QSettings* BeginSettings () const;
	virtual void EndSettings (QSettings*) const;
};

#endif

