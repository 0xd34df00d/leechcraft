#ifndef PLUGINS_POSHUKU_PLUGINS_FILESCHEME_FILESCHEME_H
#define PLUGINS_POSHUKU_PLUGINS_FILESCHEME_FILESCHEME_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QNetworkAccessManager>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace FileScheme
				{
					class FileScheme : public QObject
									 , public IInfo
									 , public IPlugin2
									 , public LeechCraft::Plugins::Poshuku::PluginBase
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2 LeechCraft::Plugins::Poshuku::PluginBase)

						std::auto_ptr<QTranslator> Translator_;
					public:
						void Init (ICoreProxy_ptr);
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						QByteArray GetPluginClass () const;

						void Init (IProxyObject*);

						QNetworkReply* CreateRequest (IHookProxy_ptr,
								QNetworkAccessManager*,
								QNetworkAccessManager::Operation*,
								const QNetworkRequest*,
								QIODevice**);
					};
				};
			};
		};
	};
};

#endif

