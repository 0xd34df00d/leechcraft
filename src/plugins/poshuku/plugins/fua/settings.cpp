#include "settings.h"
#include <QStandardItemModel>
#include "fua.h"
#include "changer.h"

using namespace LeechCraft::Poshuku::Plugins::Fua;

Settings::Settings (QStandardItemModel *model, FUA *parent)
: Fua_ (parent)
, Model_ (model)
{
	Ui_.setupUi (this);
	Ui_.Items_->setModel (model);
}

void Settings::on_Add__released ()
{
	Changer changer (Fua_->GetBrowser2ID ());
	if (changer.exec () != QDialog::Accepted)
		return;

	QString domain = changer.GetDomain ();
	QString identification = changer.GetID ();
	QList<QStandardItem*> items;
	items << new QStandardItem (domain)
		<< new QStandardItem (Fua_->GetBrowser2ID ().key (identification))
		<< new QStandardItem (identification);
	Model_->appendRow (items);
	Fua_->Save ();
}

void Settings::on_Modify__released ()
{
	QModelIndex cur = Ui_.Items_->currentIndex ();
	if (!cur.isValid ())
		return;

	QString domain = Model_->item (cur.row (), 0)->text ();
	QString identification = Model_->item (cur.row (), 2)->text ();

	Changer changer (Fua_->GetBrowser2ID (), domain, identification);
	if (changer.exec () != QDialog::Accepted)
		return;

	domain = changer.GetDomain ();
	identification = changer.GetID ();
	QList<QStandardItem*> items;
	Model_->item (cur.row (), 0)->setText (domain);
	Model_->item (cur.row (), 1)->setText (Fua_->GetBrowser2ID ().key (identification));
	Model_->item (cur.row (), 2)->setText (identification);
	Fua_->Save ();
}

void Settings::on_Remove__released ()
{
	QModelIndex cur = Ui_.Items_->currentIndex ();
	if (!cur.isValid ())
		return;

	qDeleteAll (Model_->takeRow (cur.row ()));
	Fua_->Save ();
}

