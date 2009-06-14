#ifndef PLUGINS_BITTORRENT_SECONDSTEP_H
#define PLUGINS_BITTORRENT_SECONDSTEP_H
#include <QWizardPage>
#include "ui_newtorrentsecondstep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class SecondStep : public QWizardPage, private Ui::NewTorrentSecondStep
			{
				Q_OBJECT
			public:
				SecondStep (QWidget *parent = 0);
				QStringList GetPaths () const;
			private slots:
				void on_AddPath__released ();
				void on_RemoveSelected__released ();
				void on_Clear__released ();
			};
		};
	};
};

#endif

