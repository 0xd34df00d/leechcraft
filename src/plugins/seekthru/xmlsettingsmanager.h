#ifndef PLUGINS_SEEKTHRU_XMLSETTINGSMANAGER_H
#define PLUGINS_SEEKTHRU_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class XmlSettingsManager : public LeechCraft::Util::BaseSettingsManager
			{
				Q_OBJECT

				XmlSettingsManager ();
			public:
				virtual ~XmlSettingsManager ();
				static XmlSettingsManager& Instance ();
			protected:
				virtual QSettings* BeginSettings () const;
				virtual void EndSettings (QSettings*) const;
			};
		};
	};
};

#endif

