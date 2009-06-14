#ifndef PLUGINS_BITTORRENT_FIRSTSTEP_H
#define PLUGINS_BITTORRENT_FIRSTSTEP_H
#include <QWizardPage>
#include "ui_newtorrentfirststep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class FirstStep : public QWizardPage, private Ui::NewTorrentFirstStep
			{
				Q_OBJECT
			public:
				FirstStep (QWidget *parent = 0);
			private slots:
				void on_BrowseOutput__released ();
				void on_BrowseRoot__released ();
			};
		};
	};
};

#endif

