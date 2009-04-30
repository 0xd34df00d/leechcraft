#ifndef PLUGINS_POSHUKU_PLUGINS_NETWORKMONITOR_H
#define PLUGINS_POSHUKU_PLUGINS_NETWORKMONITOR_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>

namespace LeechCraft
{
	namespace Poshuku
	{
		namespace Plugins
		{
			namespace NetworkMonitor
			{
				class Settings;

				class Plugin : public QObject
							 , public IInfo
							 , public IPlugin2
							 , public LeechCraft::Poshuku::PluginBase
				{
					Q_OBJECT
					Q_INTERFACES (IInfo IPlugin2 LeechCraft::Poshuku::PluginBase)

				public:
					void Init ();
					void Release ();
					QString GetName () const;
					QString GetInfo () const;
					QIcon GetIcon () const;
					QStringList Provides () const;
					QStringList Needs () const;
					QStringList Uses () const;
					void SetProvider (QObject*, const QString&);

					QByteArray GetPluginClass () const;

					void Init (LeechCraft::Poshuku::IProxyObject*);
				};
			};
		};
	};
};

#endif

