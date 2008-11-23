#ifndef QTOWTOOLBARADAPTOR_H
#define QTOWTOOLBARADAPTOR_H
#include <vector>
#include <Ext/ToolBar>

class QToolBar;

class QToWToolbarAdaptor : public Wt::Ext::ToolBar
{
public:
	QToWToolbarAdaptor (const QToolBar*, Wt::WContainerWidget* = 0);
	virtual ~QToWToolbarAdaptor ();
};

#endif

