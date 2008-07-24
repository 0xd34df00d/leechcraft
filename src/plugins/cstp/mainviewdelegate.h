#ifndef MAINVIEWDELEGATE_H
#define MAINVIEWDELEGATE_H
#include <QItemDelegate>

class MainViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	MainViewDelegate (QWidget* = 0);
	virtual void paint (QPainter*,
						const QStyleOptionViewItem&,
						const QModelIndex&) const;
};

#endif

