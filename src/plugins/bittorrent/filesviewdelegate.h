#ifndef PLUGINS_BITTORRENT_FILESVIEWDELEGATE_H
#define PLUGINS_BITTORRENT_FILESVIEWDELEGATE_H
#include <QItemDelegate>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class FilesViewDelegate : public QItemDelegate
			{
				Q_OBJECT
			public:
				FilesViewDelegate (QObject *parent = 0);
				virtual ~FilesViewDelegate ();

				virtual QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
				virtual void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
				virtual void setEditorData (QWidget*, const QModelIndex&) const;
				virtual void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const;
			};
		};
	};
};

#endif

