#ifndef PLUGINS_TORRENT_STARTUPFIRSTPAGE_H
#define PLUGINS_TORRENT_STARTUPFIRSTPAGE_H
#include <QWizardPage>
#include "ui_startupfirstpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class StartupFirstPage : public QWizardPage
			{
				Q_OBJECT

				Ui::StartupFirstPageWidget Ui_;
			public:
				StartupFirstPage (QWidget* = 0);

				void initializePage ();
			private slots:
				void handleAccepted ();
			};
		};
	};
};

#endif

