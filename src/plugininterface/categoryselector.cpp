#include "categoryselector.h"
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>

CategorySelector::CategorySelector (QWidget *parent)
: QWidget (parent, Qt::Dialog)
{
	setLayout (new QVBoxLayout ());
}

void CategorySelector::SetPossibleSelections (const QStringList& tags)
{
	QList<QCheckBox*> boxes = findChildren<QCheckBox*> ();
	qDeleteAll (boxes);

	for (QStringList::const_iterator i = tags.begin (),
			end = tags.end (); i != end; ++i)
	{
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

void CategorySelector::buttonToggled ()
{
	emit selectionChanged (GetSelections ());
}

