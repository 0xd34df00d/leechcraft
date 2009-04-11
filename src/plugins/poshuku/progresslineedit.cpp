#include "progresslineedit.h"
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QToolBar>
#include <QtDebug>
#include "urlcompletionmodel.h"

ProgressLineEdit::ProgressLineEdit (QWidget *parent)
: QLineEdit (parent)
, IsCompleting_ (false)
{
}

ProgressLineEdit::~ProgressLineEdit ()
{
}

bool ProgressLineEdit::IsCompleting () const
{
	return IsCompleting_;
}

void ProgressLineEdit::AddAction (QAction *act)
{
	addAction (act);
}

void ProgressLineEdit::focusInEvent (QFocusEvent *e)
{
	QLineEdit::focusInEvent (e);

	disconnect (completer (),
			0,
			this,
			0);
	connect (completer (),
			SIGNAL (activated (const QModelIndex&)),
			this,
			SLOT (handleActivated (const QModelIndex&)));
	connect (completer (),
			SIGNAL (highlighted (const QModelIndex&)),
			this,
			SLOT (handleHighlighted (const QModelIndex&)));
}

void ProgressLineEdit::keyPressEvent (QKeyEvent *e)
{
	QLineEdit::keyPressEvent (e);
	IsCompleting_ = false;
}

void ProgressLineEdit::handleActivated (const QModelIndex& index)
{
	QString url = qobject_cast<URLCompletionModel*> (completer ()->
			model ())->index (index.row (), 0)
		.data (URLCompletionModel::RoleURL).toString ();

	setText (url);
	IsCompleting_ = false;
	emit returnPressed ();
}

void ProgressLineEdit::handleHighlighted (const QModelIndex& index)
{
	IsCompleting_ = index.isValid ();

	QString url = qobject_cast<URLCompletionModel*> (completer ()->
			model ())->index (index.row (), 0)
		.data (URLCompletionModel::RoleURL).toString ();

	setText (url);
}

