#include "categoryselector.h"
#include <algorithm>
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtDebug>

using namespace LeechCraft::Util;
const int RoleTag = 52;

CategorySelector::CategorySelector (QWidget *parent)
: QTreeWidget (parent)
{
	setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);
	setRootIsDecorated (false);

	QRect avail = QApplication::desktop ()->availableGeometry (this);
	setMinimumHeight (avail.height () / 3 * 2);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));
}

CategorySelector::~CategorySelector ()
{
}

void CategorySelector::SetPossibleSelections (const QStringList& tags)
{
	clear ();

	QList<QTreeWidgetItem*> items;
	for (QStringList::const_iterator i = tags.begin (),
			end = tags.end (); i != end; ++i)
	{
		if (i->isEmpty ())
			continue;

		QTreeWidgetItem *item = new QTreeWidgetItem (QStringList (*i));
		item->setCheckState (0, Qt::Unchecked);
		item->setData (0, RoleTag, *i);
		items << item;
	}
	addTopLevelItems (items);

	setHeaderLabel (QString ());
}

QStringList CategorySelector::GetSelections ()
{
	QStringList tags;

	for (int i = 0; i < topLevelItemCount (); ++i)
		if (topLevelItem (i)->checkState (0) == Qt::Checked)
			tags += topLevelItem (i)->data (0, RoleTag).toString ();

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
	for (int i = 0; i < topLevelItemCount (); ++i)
		topLevelItem (i)->setCheckState (0, Qt::Checked);
}

void CategorySelector::selectNone ()
{
	for (int i = 0; i < topLevelItemCount (); ++i)
		topLevelItem (i)->setCheckState (0, Qt::Unchecked);
}

void CategorySelector::lineTextChanged (const QString& text)
{
	blockSignals (true);
	QStringList tags = text.split ("; ", QString::SkipEmptyParts);
	for (int i = 0; i < topLevelItemCount (); ++i)
	{
		Qt::CheckState state =
			tags.contains (topLevelItem (i)->data (0, RoleTag).toString ()) ?
				Qt::Checked :
				Qt::Unchecked;
		topLevelItem (i)->setCheckState (0, state);
	}
	blockSignals (false);
}

void CategorySelector::buttonToggled ()
{
	emit selectionChanged (GetSelections ());
}

