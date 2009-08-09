#ifndef PLUGINS_TORRENT_WIZARDGENERATOR_H
#define PLUGINS_TORRENT_WIZARDGENERATOR_H
#include <QList>

class QWizardPage;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class WizardGenerator
			{
			public:
				QList<QWizardPage*> GetPages ();
			};
		};
	};
};

#endif

