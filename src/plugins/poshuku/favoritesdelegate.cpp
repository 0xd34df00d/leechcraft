#include "favoritesdelegate.h"
#include <plugininterface/tagslineedit.h>
#include "core.h"
#include "tagscompletionmodel.h"
#include "filtermodel.h"
#include "favoritesmodel.h"

FavoritesDelegate::FavoritesDelegate (QObject *parent)
: QItemDelegate (parent)
{
}

QWidget* FavoritesDelegate::createEditor (QWidget *parent,
		const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
	if (index.column () != FavoritesModel::ColumnTags)
		return QItemDelegate::createEditor (parent, opt, index);

	TagsLineEdit *tle = new TagsLineEdit (parent);
	TagsCompleter_.reset (new TagsCompleter (tle));
	TagsCompleter_->setModel (Core::Instance ().GetFavoritesTagsCompletionModel ());
	tle->AddSelector ();
	return tle;
}

void FavoritesDelegate::setEditorData (QWidget *editor,
		const QModelIndex& index) const
{
	if (index.column () != FavoritesModel::ColumnTags)
	{
		QItemDelegate::setEditorData (editor, index);
		return;
	}

	static_cast<TagsLineEdit*> (editor)->setText (Core::Instance ()
			.GetFavoritesModel ()->	data (index, FavoritesModel::TagsRole)
			.toStringList ().join (" "));
}

void FavoritesDelegate::setModelData (QWidget *editor,
		QAbstractItemModel *model, const QModelIndex& index) const
{
	if (index.column () != FavoritesModel::ColumnTags)
	{
		QItemDelegate::setModelData (editor, model, index);
		return;
	}

	QStringList tags = static_cast<TagsLineEdit*> (editor)->
		text ().split (" ", QString::SkipEmptyParts);
	model->setData (index, tags);
}

void FavoritesDelegate::updateEditorGeometry (QWidget *editor,
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const
{
	if (index.column () != FavoritesModel::ColumnTags)
	{
		QItemDelegate::updateEditorGeometry (editor, option, index);
		return;
	}

	editor->setGeometry (option.rect);
}

