#include <QtDebug>
#include "core.h"
#include "itembucket.h"
#include "itemmodel.h"

using namespace LeechCraft::Plugins::Aggregator;

ItemBucket::ItemBucket (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Model_ = Core::Instance ().GetItemModel ();
	Ui_.Items_->setModel (Model_);
	Ui_.Items_->addAction (Ui_.ActionDeleteItem_);
	Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
	Ui_.ItemView_->Construct (Core::Instance ().GetWebBrowser ());

	connect (Ui_.Items_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentItemChanged (const QModelIndex&)));
}

ItemBucket::~ItemBucket ()
{
}

void ItemBucket::on_Items__activated (const QModelIndex& index)
{
	Model_->Activated (index);
}

void ItemBucket::on_ActionDeleteItem__triggered ()
{
	QModelIndexList indexes = Ui_.Items_->selectionModel ()->selectedRows ();
	for (int i = 0; i < indexes.size (); ++i)
		Model_->RemoveItem (indexes.at (i));
}

void ItemBucket::currentItemChanged (const QModelIndex& index)
{
	Ui_.ItemView_->SetHtml (Model_->GetDescription (index));
}


