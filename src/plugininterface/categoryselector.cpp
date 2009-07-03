#include "categoryselector.h"
#include <algorithm>
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>

using namespace LeechCraft::Util;

CategorySelector::CategorySelector (QWidget *parent)
: QWidget (parent, Qt::Tool | Qt::WindowStaysOnTopHint)
{
	setLayout (new QVBoxLayout ());
}

CategorySelector::~CategorySelector ()
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();
	qDeleteAll (boxes);
}

void CategorySelector::SetPossibleSelections (const QStringList& tags)
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();
	qDeleteAll (boxes);

	for (QStringList::const_iterator i = tags.begin (),
			end = tags.end (); i != end; ++i)
	{
		if (i->isEmpty ())
			continue;

		QCheckBox *box = new QCheckBox (*i, this);
		layout ()->addWidget (box);
		box->setCheckState (Qt::Unchecked);
		box->setProperty ("Tag", *i);

		connect (box,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (buttonToggled ()));
	}
}

QStringList CategorySelector::GetSelections ()
{
	QStringList tags;

	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();
	for (QList<QCheckBox*>::const_iterator i = boxes.begin (),
			end = boxes.end (); i != end; ++i)
		if ((*i)->checkState () == Qt::Checked)
			tags += (*i)->property ("Tag").toString ();

	return tags;
}

void CategorySelector::moveEvent (QMoveEvent *e)
{
	QWidget::moveEvent (e);
	QPoint pos = e->pos ();
	QRect avail = QApplication::desktop ()->availableGeometry (this);
	int dx = 0, dy = 0;
	if (pos.x () + width () > avail.width ())
		dx = width () + pos.x () - avail.width ();
	if (pos.y () + height () > avail.height () &&
			height () < avail.height ())
		dy = height () + pos.y () - avail.height ();

	if (dx || dy)
		move (pos - QPoint (dx, dy));
}

void CategorySelector::selectAll ()
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();

	for (QList<QCheckBox*>::iterator box = boxes.begin (),
			end = boxes.end (); box != end; ++box)
		(*box)->setCheckState (Qt::Checked);
}

void CategorySelector::selectNone ()
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();

	for (QList<QCheckBox*>::iterator box = boxes.begin (),
			end = boxes.end (); box != end; ++box)
		(*box)->setCheckState (Qt::Unchecked);
}

void CategorySelector::lineTextChanged (const QString& text)
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();

	QStringList tags = text.split ("; ", QString::SkipEmptyParts);
	for (QList<QCheckBox*>::iterator box = boxes.begin (),
			end = boxes.end (); box != end; ++box)
	{
		QStringList::const_iterator tag = std::find (tags.begin (),
				tags.end (), (*box)->property ("Tag").toString ());

		(*box)->blockSignals (true);
		if (tag == tags.end ())
			(*box)->setCheckState (Qt::Unchecked);
		else
			(*box)->setCheckState (Qt::Checked);
		(*box)->blockSignals (false);
	}
}

void CategorySelector::buttonToggled ()
{
	emit selectionChanged (GetSelections ());
}

