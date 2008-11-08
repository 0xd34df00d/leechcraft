#include "addtofavoritesdialog.h"

AddToFavoritesDialog::AddToFavoritesDialog (const QString& title,
		const QString& url, QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.URLLabel_->setText (url);
	Ui_.TitleEdit_->setText (title);
}

AddToFavoritesDialog::~AddToFavoritesDialog ()
{
}

QString AddToFavoritesDialog::GetTitle () const
{
	return Ui_.TitleEdit_->text ();
}

QStringList AddToFavoritesDialog::GetTags () const
{
	return Ui_.TagsEdit_->text ().split (" ", QString::SkipEmptyParts);
}

