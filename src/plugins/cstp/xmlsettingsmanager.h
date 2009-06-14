#ifndef PLUGINS_CSTP_XMLSETTINGSMANAGER_H
#define PLUGINS_CSTP_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class XmlSettingsManager : public LeechCraft::Util::BaseSettingsManager
			{
				Q_OBJECT
				XmlSettingsManager ();
			public:
				static XmlSettingsManager& Instance ();
			protected:
				virtual QSettings* BeginSettings () const;
				virtual void EndSettings (QSettings*) const;
			};
		};
	};
};

#endif

