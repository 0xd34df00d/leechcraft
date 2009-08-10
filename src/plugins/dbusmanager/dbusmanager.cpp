#include "dbusmanager.h"
#include <QIcon>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			void DBusManager::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("dbusmanager"));

				Core::Instance ().SetProxy (proxy);

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"dbusmanagersettings.xml");
			}

			void DBusManager::Release ()
			{
				Core::Instance ().Release ();
			}

			QString DBusManager::GetName () const
			{
				return "DBus Manager";
			}

			QString DBusManager::GetInfo () const
			{
				return tr ("DBus support for LeechCraft");
			}

			QStringList DBusManager::Provides () const
			{
				return QStringList ("dbus");
			}

			QStringList DBusManager::Uses () const
			{
				return QStringList ();
			}

			QStringList DBusManager::Needs () const
			{
				return QStringList ();
			}

			void DBusManager::SetProvider (QObject*, const QString&)
			{
			}

			QIcon DBusManager::GetIcon () const
			{
				return QIcon (":/resources/images/dbusmanager.svg");
			}

			boost::shared_ptr<Util::XmlSettingsDialog> DBusManager::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_dbusmanager, LeechCraft::Plugins::DBusManager::DBusManager);

