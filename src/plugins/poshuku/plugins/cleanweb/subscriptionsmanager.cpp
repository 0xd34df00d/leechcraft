#include "subscriptionsmanager.h"
#include "core.h"

using namespace LeechCraft::Plugins::Poshuku::Plugins::CleanWeb;

SubscriptionsManager::SubscriptionsManager (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.Subscriptions_->setModel (Core::Instance ().GetModel ());
}

void SubscriptionsManager::on_RemoveButton__released ()
{
	QModelIndex current = Ui_.Subscriptions_->currentIndex ();
	if (!current.isValid ())
		return;

	Core::Instance ().Remove (current);
}

