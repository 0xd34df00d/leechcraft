#ifndef IPLUGIN2_H
#define IPLUGIN2_H
#include <QtPlugin>
#include <QByteArray>

class IPlugin2
{
public:
	virtual QByteArray GetPluginClass () const = 0;
};

Q_DECLARE_INTERFACE (IPlugin2, "org.Deviant.LeechCraft.IPlugin2/1.0");

#endif

