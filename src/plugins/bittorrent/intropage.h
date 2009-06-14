#ifndef PLUGINS_BITTORRENT_INTROPAGE_H
#define PLUGINS_BITTORRENT_INTROPAGE_H
#include <QWizardPage>

class QLabel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class IntroPage : public QWizardPage
			{
				Q_OBJECT

				QLabel *Label_;
			public:
				IntroPage (QWidget *parent = 0);
			};
		};
	};
};

#endif

