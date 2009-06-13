#ifndef FAVORITESDELEGATE_H
#define FAVORITESDELEGATE_H
#include <memory>
#include <QItemDelegate>
#include <plugininterface/tagscompleter.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FavoritesDelegate : public QItemDelegate
			{
				Q_OBJECT

				mutable std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsCompleter_;
			public:
				FavoritesDelegate (QObject* = 0);

				QWidget* createEditor (QWidget*, const QStyleOptionViewItem&,
						const QModelIndex&) const;
				void setEditorData (QWidget*, const QModelIndex&) const;
				void setModelData (QWidget*, QAbstractItemModel*,
						const QModelIndex&) const;
				void updateEditorGeometry (QWidget*, const QStyleOptionViewItem&,
						const QModelIndex&) const;
			};
		};
	};
};

#endif

