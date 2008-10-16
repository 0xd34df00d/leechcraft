#include "pluginmanagerdialog.h"
#include "core.h"

Main::PluginManagerDialog::PluginManagerDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.PluginsTree_->setModel (Core::Instance ().GetPluginsModel ());
}

Main::PluginManagerDialog::~PluginManagerDialog ()
{
}

void Main::PluginManagerDialog::on_PluginsTree__activated (const QModelIndex& index)
{
	Core::Instance ().Activated (index);
}

