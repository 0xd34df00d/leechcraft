#include "tagsviewer.h"
#include <QInputDialog>
#include "tagsmanager.h"

using namespace LeechCraft;

TagsViewer::TagsViewer (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.TagsView_->setModel (TagsManager::Instance ().GetModel ());
}

void LeechCraft::TagsViewer::on_Rename__released ()
{
	QModelIndex index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	QString original = TagsManager::Instance ()
		.data (index, Qt::DisplayRole).toString ();

	bool ok = false;

	QString newTag = QInputDialog::getText (this,
			tr ("Rename"),
			tr ("Enter new tag name"),
			QLineEdit::Normal,
			original,
			&ok);

	if (!ok)
		return;

	TagsManager::Instance ().SetTag (index, newTag);
}

void TagsViewer::on_Remove__released ()
{
	QModelIndex index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	TagsManager::Instance ().RemoveTag (index);
}

