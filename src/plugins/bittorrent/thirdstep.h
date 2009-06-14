#ifndef PLUGINS_BITTORRENT_THIRDSTEP_H
#define PLUGINS_BITTORRENT_THIRDSTEP_H
#include <QWizardPage>
#include "ui_newtorrentthirdstep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class ThirdStep : public QWizardPage, private Ui::NewTorrentThirdStep
			{
				Q_OBJECT

				quint64 TotalSize_;
			public:
				ThirdStep (QWidget *parent = 0);
				virtual void initializePage ();
			private slots:
				void on_PieceSize__currentIndexChanged ();
			};
		};
	};
};

#endif

