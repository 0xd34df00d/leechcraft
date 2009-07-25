#ifndef PLUGINS_AGGREGATOR_CHANNELSMODEL_H
#define PLUGINS_AGGREGATOR_CHANNELSMODEL_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include "channel.h"

class QToolBar;
class QMenu;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ChannelsModel : public QAbstractItemModel
			{
				Q_OBJECT

				QStringList Headers_;
				typedef QList<ChannelShort> Channels_t;
				Channels_t Channels_;
				QToolBar *Toolbar_;
				QWidget *TabWidget_;
				QMenu *Menu_;
			public:
				enum Columns
				{
					ColumnTitle,
					ColumnUnread,
					ColumnLastBuild
				};
				ChannelsModel (QObject *parent = 0);
				virtual ~ChannelsModel ();

				void SetWidgets (QToolBar*, QWidget*);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void AddChannel (const ChannelShort&);
				void Update (const channels_container_t&);
				void UpdateChannelData (const ChannelShort&);
				ChannelShort& GetChannelForIndex (const QModelIndex&);
				void RemoveChannel (const ChannelShort&);
				QModelIndex GetUnreadChannelIndex () const;
				int GetUnreadChannelsNumber () const;
				int GetUnreadItemsNumber () const;
				void SetMenu (QMenu*);
			signals:
				void channelDataUpdated ();
			};
		};
	};
};

#endif

