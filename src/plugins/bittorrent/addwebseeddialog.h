#ifndef PLUGINS_TORRENT_ADDWEBSEEDDIALOG_H
#define PLUGINS_TORRENT_ADDWEBSEEDDIALOG_H
#include <QDialog>
#include "ui_addwebseeddialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class AddWebSeedDialog : public QDialog
			{
				Q_OBJECT

				Ui::AddWebSeedDialog Ui_;
			public:
				AddWebSeedDialog (QWidget* = 0);

				QString GetURL () const;
				// True if URL (BEP 19), false if HTTP (BEP 17).
				bool GetType () const;
			};
		};
	};
};

#endif

