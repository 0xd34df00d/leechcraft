#ifndef STRUCTURES_H
#define STRUCTURES_H

class QMenu;

struct DirectDownloadParams
{
	QString Resource_;
	QString Location_;
	bool Autostart_;
	bool ShouldBeSavedInHistory_;
};

struct MainWindowExternals
{
	QMenu *RootMenu_;
};

#endif

