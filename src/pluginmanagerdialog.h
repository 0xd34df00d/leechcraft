#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H
#include <QDialog>
#include "ui_pluginmanagerdialog.h"

namespace Main
{
	class PluginManagerDialog : public QDialog
	{
		Q_OBJECT

		Ui::PluginManagerDialog Ui_;
	public:
		PluginManagerDialog (QWidget* = 0);
		virtual ~PluginManagerDialog ();
	private slots:
		void on_PluginsTree__activated (const QModelIndex&);
	};
};

#endif

