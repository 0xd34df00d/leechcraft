#ifndef PLUGINS_POSHUKU_PLUGINS_NETWORKMONITOR_H
#define PLUGINS_POSHUKU_PLUGINS_NETWORKMONITOR_H
#include <QDialog>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>
#include "ui_networkmonitor.h"

namespace LeechCraft
{
	namespace Poshuku
	{
		class IProxyObject;

		namespace Plugins
		{
			namespace NetworkMonitor
			{
				class Settings;

				class Plugin : public QDialog
							 , public IInfo
							 , public IPlugin2
							 , public LeechCraft::Poshuku::PluginBase
				{
					Q_OBJECT
					Q_INTERFACES (IInfo IPlugin2 LeechCraft::Poshuku::PluginBase)

					Ui::NetworkMonitor Ui_;
					IProxyObject *Object_;
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

