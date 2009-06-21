#ifndef PLUGINS_BITTORRENT_PEERSTABLINKER_H
#define PLUGINS_BITTORRENT_PEERSTABLINKER_H
#include <QObject>
#include <QModelIndex>

class QSortFilterProxyModel;

namespace Ui
{
	class TabWidget;
};

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class PeersTabLinker : public QObject
			{
				Q_OBJECT

				Ui::TabWidget *Ui_;
				QSortFilterProxyModel *ProxyModel_;
				QModelIndex Current_;
			public:
				PeersTabLinker (Ui::TabWidget*,
						QSortFilterProxyModel*, QObject* = 0);
			private slots:
				void handleNewRow (const QModelIndex&);
				void update ();
			};
		};
	};
};

#endif

