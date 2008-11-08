#include "addtofavoritesdialog.h"
#include <plugininterface/tagscompletionmodel.h>

AddToFavoritesDialog::AddToFavoritesDialog (const QString& title,
		const QString& url,
		TagsCompletionModel *model,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.URLLabel_->setText (url);
	Ui_.TitleEdit_->setText (title);
	Ui_.TagsEdit_->setText (tr ("untagged"));

	TagsCompleter_.reset (new TagsCompleter (Ui_.TagsEdit_));
	TagsCompleter_->setModel (model);
	Ui_.TagsEdit_->AddSelector ();
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

