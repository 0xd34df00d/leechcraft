#ifndef DOWNLOADERSWIDGET_H
#define DOWNLOADERSWIDGET_H
#include <WCompositeWidget>

namespace Wt
{
	class WContainerWidget;
	class WTreeView;
	class WAbstractItemModel;
};

class DownloadersWidget : public Wt::WCompositeWidget
{
	Wt::WContainerWidget *Container_;
	Wt::WTreeView *DownloadersView_;
public:
	DownloadersWidget (Wt::WContainerWidget* = 0);
	virtual ~DownloadersWidget ();

	void SetModel (Wt::WAbstractItemModel*);
};

#endif

