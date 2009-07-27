#ifndef PLUGINS_TORRENT_ADDPEERDIALOG_H
#define PLUGINS_TORRENT_ADDPEERDIALOG_H
#include <QDialog>
#include "ui_addpeerdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class AddPeerDialog : public QDialog
			{
				Q_OBJECT

				Ui::AddPeerDialog Ui_;
			public:
				AddPeerDialog (QWidget* = 0);

				QString GetIP () const;
				int GetPort () const;
			};
		};
	};
};

#endif

