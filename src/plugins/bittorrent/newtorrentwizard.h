#ifndef PLUGINS_BITTORRENT_NEWTORRENTWIZARD_H
#define PLUGINS_BITTORRENT_NEWTORRENTWIZARD_H
#include <QWizard>
#include "newtorrentparams.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class NewTorrentWizard : public QWizard
			{
				Q_OBJECT
			public:
				enum Page { PageIntro
					, PageFirstStep
					, PageSecondStep
					, PageThirdStep };

				NewTorrentWizard (QWidget *parent = 0);
				virtual void accept ();
				NewTorrentParams GetParams () const;
			};
		};
	};
};

#endif

