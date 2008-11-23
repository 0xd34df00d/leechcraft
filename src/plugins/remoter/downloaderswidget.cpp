#include "downloaderswidget.h"
#include <WContainerWidget>
#include <WText>
#include <WTreeView>

DownloadersWidget::DownloadersWidget (Wt::WContainerWidget *parentc)
: Wt::WCompositeWidget (parentc)
{
	setImplementation ((Container_ = new Wt::WContainerWidget ()));

	DownloadersView_ = new Wt::WTreeView (Container_);
}

DownloadersWidget::~DownloadersWidget ()
{
}

void DownloadersWidget::SetModel (Wt::WAbstractItemModel *model)
{
	DownloadersView_->setModel (model);
}

