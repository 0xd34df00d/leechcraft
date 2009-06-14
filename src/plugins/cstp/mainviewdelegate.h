#ifndef PLUGINS_CSTP_MAINVIEWDELEGATE_H
#define PLUGINS_CSTP_MAINVIEWDELEGATE_H
#include <QItemDelegate>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class MainViewDelegate : public QItemDelegate
			{
				Q_OBJECT
			public:
				MainViewDelegate (QWidget* = 0);
				virtual void paint (QPainter*,
									const QStyleOptionViewItem&,
									const QModelIndex&) const;
			};
		};
	};
};

#endif

