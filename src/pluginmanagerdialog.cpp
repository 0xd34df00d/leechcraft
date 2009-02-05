#include "pluginmanagerdialog.h"
#include "core.h"

using namespace LeechCraft;

LeechCraft::PluginManagerDialog::PluginManagerDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.PluginsTree_->setModel (Core::Instance ().GetPluginsModel ());
}

LeechCraft::PluginManagerDialog::~PluginManagerDialog ()
{
}

void LeechCraft::PluginManagerDialog::on_PluginsTree__activated (const QModelIndex& index)
{
	Core::Instance ().Activated (index);
}

