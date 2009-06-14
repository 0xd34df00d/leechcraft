#ifndef PLUGINS_BITTORRENT_TRACKERSCHANGER_H
#define PLUGINS_BITTORRENT_TRACKERSCHANGER_H
#include <QDialog>
#include "ui_trackerschanger.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class TrackersChanger : public QDialog
			{
				Q_OBJECT

				Ui::TrackersChanger Ui_;
			public:
				TrackersChanger (QWidget *parent = 0);
				void SetTrackers (const QStringList&);
				QStringList GetTrackers () const;
			};
		};
	};
};

#endif

