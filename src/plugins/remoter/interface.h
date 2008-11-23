#ifndef INTERFACE_H
#define INTERFACE_H

class DownloadersWidget;

namespace Wt
{
	class WApplication;
	class WContainerWidget;
	class WEnvironment;
	class WTreeView;
};

class Interface
{
	DownloadersWidget *DownloadersWidget_;
public:
	Interface (Wt::WApplication*, const Wt::WEnvironment&);
private:
	void BuildInterface (Wt::WContainerWidget*, const Wt::WEnvironment&);
	void SetupHistoryView (Wt::WTreeView*);
};

#endif

