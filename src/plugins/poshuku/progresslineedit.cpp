#include "progresslineedit.h"
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QtDebug>
#include "urlcompletionmodel.h"

ProgressLineEdit::ProgressLineEdit (QWidget *parent)
: QLineEdit (parent)
, IsCompleting_ (false)
{
	setValue (0);
}

ProgressLineEdit::~ProgressLineEdit ()
{
}

bool ProgressLineEdit::IsCompleting () const
{
	return IsCompleting_;
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

void ProgressLineEdit::setValue (int value)
{
	double sv = static_cast<double> (value) / 100;
	QString ss = QString ("QLineEdit { "
			"background-color: qlineargradient(spread:pad, "
			"x1:0, y1:1, x2:1, y2:1, "
			"stop:%1 rgba(160, 160, 160, 200), "
			"stop:%2 rgba(255, 255, 255, 255)) }")
		.arg (std::max (sv - 0.001, 0.))
		.arg (std::min (sv + 0.05, 1.));

	setStyleSheet (ss);
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

