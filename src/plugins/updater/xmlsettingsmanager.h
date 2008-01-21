#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <QObject>
#include <interfaces/interfaces.h>

class XmlSettingsManager : public QObject
{
	Q_OBJECT
public:
	XmlSettingsManager ();
	~XmlSettingsManager ();
	void Release ();
	static XmlSettingsManager* Instance ();
};

#endif

