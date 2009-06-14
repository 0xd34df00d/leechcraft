#ifndef PLUGINS_BITTORRENT_MOVETORRENTFILES_H
#define PLUGINS_BITTORRENT_MOVETORRENTFILES_H
#include <QDialog>
#include "ui_movetorrentfiles.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class MoveTorrentFiles : public QDialog
			{
				Q_OBJECT

				Ui::MoveTorrentFiles Ui_;
			public:
				MoveTorrentFiles (const QString&, QWidget* = 0);
				QString GetNewLocation () const;
			private slots:
				void on_Browse__released ();
			};
		};
	};
};

#endif

