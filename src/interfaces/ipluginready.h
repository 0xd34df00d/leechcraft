#ifndef IPLUGINREADY_H
#define IPLUGINREADY_H
#include <QtPlugin>

class IPluginReady
{
public:
	virtual QByteArray GetExpectedPluginClass () const = 0;
	virtual void AddPlugin (QObject*) = 0;
};

Q_DECLARE_INTERFACE (IPluginReady, "org.Deviant.LeechCraft.IPluginReady/1.0");

#endif

