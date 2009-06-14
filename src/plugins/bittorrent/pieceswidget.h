#ifndef PLUGINS_BITTORRENT_PIECESWIDGET_H
#define PLUGINS_BITTORRENT_PIECESWIDGET_H
#include <QLabel>
#include <vector>
#include <libtorrent/bitfield.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class PiecesWidget : public QLabel
			{
				Q_OBJECT

				libtorrent::bitfield Pieces_;
			public:
				PiecesWidget (QWidget *parent = 0);
			public slots:
				void setPieceMap (const libtorrent::bitfield&);
			private:
				void paintEvent (QPaintEvent*);
			};
		};
	};
};

#endif

