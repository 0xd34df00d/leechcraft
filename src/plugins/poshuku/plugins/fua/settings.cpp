#include "settings.h"

Settings::Settings (QAbstractItemModel *model, QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.Items_->setModel (model);
}

