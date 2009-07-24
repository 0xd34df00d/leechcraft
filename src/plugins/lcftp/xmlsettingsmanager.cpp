#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			XmlSettingsManager::XmlSettingsManager ()
			{
				Util::BaseSettingsManager::Init ();
			}
			
			XmlSettingsManager& XmlSettingsManager::Instance ()
			{
				static XmlSettingsManager xsm;
				return xsm;
			}
			
			QSettings* XmlSettingsManager::BeginSettings () const
			{
				QSettings *settings = new QSettings (
						LeechCraft::Util::Proxy::Instance ()->GetOrganizationName (),
						LeechCraft::Util::Proxy::Instance ()->GetApplicationName () + "_LCFTP");
				return settings;
			}
			
			void XmlSettingsManager::EndSettings (QSettings*) const
			{
			}
		};
	};
};

