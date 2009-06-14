#ifndef PLUGINS_BITTORRENT_XMLSETTINGSMANAGER_H
#define PLUGINS_BITTORRENT_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class XmlSettingsManager : public Util::BaseSettingsManager
			{
				Q_OBJECT

				XmlSettingsManager ();
			public:
				static XmlSettingsManager* Instance ();
			protected:
				virtual QSettings* BeginSettings () const;
				virtual void EndSettings (QSettings*) const;
			};
		};
	};
};

#endif

