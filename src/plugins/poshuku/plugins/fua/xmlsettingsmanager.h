#ifndef PLUGINS_POSHUKU_PLUGINS_FUA_XMLSETTINGSMANAGER_H
#define PLUGINS_POSHUKU_PLUGINS_FUA_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace Fua
				{
					class XmlSettingsManager : public LeechCraft::Util::BaseSettingsManager
					{
						Q_OBJECT
					public:
						XmlSettingsManager ();
						static XmlSettingsManager* Instance ();
					protected:
						virtual QSettings* BeginSettings () const;
						virtual void EndSettings (QSettings*) const;
					};
				};
			};
		};
	};
};

#endif

