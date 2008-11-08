#include "tagslineedit.h"
#include <QtDebug>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include "tagscompletionmodel.h"

TagsLineEdit::TagsLineEdit (QWidget *parent)
: QLineEdit (parent)
{
}

void TagsLineEdit::AddSelector ()
{
	CategorySelector_.reset (new CategorySelector (parentWidget ()));
	CategorySelector_->hide ();

	QAbstractItemModel *model = completer ()->model ();

	connect (qobject_cast<TagsCompletionModel*> (model),
			SIGNAL (tagsUpdated (const QStringList&)),
			this,
			SLOT (handleTagsUpdated (const QStringList&)));

	handleTagsUpdated (qobject_cast<TagsCompletionModel*> (model)->GetTags ());

	connect (CategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			this,
			SLOT (handleSelectionChanged (const QStringList&)));

	connect (this,
			SIGNAL (textChanged (const QString&)),
			CategorySelector_.get (),
			SLOT (lineTextChanged (const QString&)));
}

void TagsLineEdit::complete (const QString& completion)
{
    QString wtext = text ();
    int pos = wtext.lastIndexOf (' ');
    if (pos == -1)
        wtext = completion;
    else
        wtext = wtext.left (pos).append (' ').append (completion);
    setText (wtext);
}

void TagsLineEdit::handleTagsUpdated (const QStringList& tags)
{
	CategorySelector_->SetPossibleSelections (tags);
}

void TagsLineEdit::handleSelectionChanged (const QStringList& tags)
{
	setText (tags.join (" "));
}

void TagsLineEdit::focusInEvent (QFocusEvent *e)
{
    QLineEdit::focusInEvent (e);
    if (completer ())
    {
        disconnect (completer (), SIGNAL (activated (const QString&)), this, SLOT (setText (const QString&)));
        disconnect (completer (), SIGNAL (highlighted (const QString&)), this, 0);
        connect (completer (), SIGNAL (highlighted (const QString&)), this, SLOT (complete (const QString&)));
    }
}

void TagsLineEdit::contextMenuEvent (QContextMenuEvent *e)
{
	if (!CategorySelector_.get ())
	{
		QLineEdit::contextMenuEvent (e);
		return;
	}

	CategorySelector_->move (e->globalPos ());
	CategorySelector_->show ();
}

