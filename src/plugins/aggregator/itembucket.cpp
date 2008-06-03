#include <QtDebug>
#include "itemmodel.h"
#include "core.h"
#include "itembucket.h"
#include "ui_itembucket.h"

ItemBucket::ItemBucket ()
: Model_ (new ItemModel)
, Ui_ (new Ui::ItemBucket)
{
	Ui_->setupUi (this);
	Ui_->Items_->setModel (Model_);
    Ui_->Items_->addAction (Ui_->ActionDeleteItem_);
    Ui_->Items_->setContextMenuPolicy (Qt::ActionsContextMenu);

    connect (Ui_->Items_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentItemChanged (const QModelIndex&)));
}

ItemBucket::~ItemBucket ()
{
}

ItemBucket& ItemBucket::Instance ()
{
	static ItemBucket bucket;
	return bucket;
}

void ItemBucket::Release ()
{
	qDebug () << Q_FUNC_INFO;
	Model_->saveSettings ();
	delete Ui_;
	delete Model_;
}

void ItemBucket::AddItem (const boost::shared_ptr<Item>& item)
{
	Model_->AddItem (item);
}

void ItemBucket::on_Items__activated (const QModelIndex& index)
{
	Model_->Activated (index);
}

void ItemBucket::on_ActionDeleteItem__triggered ()
{
    QModelIndexList indexes = Ui_->Items_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Model_->RemoveItem (indexes.at (i));
}

void ItemBucket::currentItemChanged (const QModelIndex& index)
{
    Ui_->ItemView_->setHtml (Model_->GetDescription (index));
	connect (Ui_->ItemView_->page ()->networkAccessManager (),
			SIGNAL (sslErrors (QNetworkReply*, const QList<QSslError>&)),
			&Core::Instance (),
			SLOT (handleSslError (QNetworkReply*)));
}

