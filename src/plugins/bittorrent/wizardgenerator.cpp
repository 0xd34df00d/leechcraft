#include "wizardgenerator.h"
#include "xmlsettingsmanager.h"
#include "startupfirstpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			QList<QWizardPage*> WizardGenerator::GetPages ()
			{
				QList<QWizardPage*> result;
				int version = XmlSettingsManager::Instance ()->
					Property ("StartupVersion", 0).toInt ();
				if (version < 1)
				{
					result << new StartupFirstPage ();
					++version;
				}
				XmlSettingsManager::Instance ()->setProperty ("StartupVersion", version);
				return result;
			}
		};
	};
};
