#ifndef FEEDSETTINGS_H
#define FEEDSETTINGS_H
#include <memory>
#include <QDialog>
#include <QModelIndex>
#include <plugininterface/tagscompleter.h>
#include "ui_feedsettings.h"

class FeedSettings : public QDialog
{
	Q_OBJECT
	
	Ui::FeedSettings Ui_;
	std::auto_ptr<LeechCraft::Util::TagsCompleter> ChannelTagsCompleter_;
	QModelIndex Index_;
public:
	FeedSettings (const QModelIndex&, QWidget* = 0);
public slots:
	virtual void accept ();
};

#endif

