#ifndef PLUGINS_DBUSMANAGER_DBUSMANAGER_H
#define PLUGINS_DBUSMANAGER_DBUSMANAGER_H
#include <memory>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class DBusManager : public QObject
							  , public IInfo
							  , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings);

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
			public:
				void Init (ICoreProxy_ptr);
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Uses () const;
				QStringList Needs () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
			};
		};
	};
};

#endif

