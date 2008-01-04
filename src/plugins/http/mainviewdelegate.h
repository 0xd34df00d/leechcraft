#ifndef MAINVIEWDELEGATE_H
#define MAINVIEWDELEGATE_H
#include <QItemDelegate>

class HttpPlugin;
class QPainter;
class QStyleOptionViewItem;
class QModelIndex;

class MainViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	MainViewDelegate (HttpPlugin*);
	virtual void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
};

#endif

