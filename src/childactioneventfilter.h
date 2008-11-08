#ifndef CHILDACTIONEVENTFILTER_H
#define CHILDACTIONEVENTFILTER_H
#include <QObject>

class ChildActionEventFilter : public QObject
{
	Q_OBJECT
public:
	ChildActionEventFilter (QObject* = 0);
	virtual ~ChildActionEventFilter ();
protected:
	bool eventFilter (QObject*, QEvent*);
};

#endif

