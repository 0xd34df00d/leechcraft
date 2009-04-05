#include "shortcutmanager.h"
#include <memory>
#include <QSettings>
#include <interfaces/iinfo.h>
#include <interfaces/ihaveshortcuts.h>
#include "keysequencer.h"

using namespace LeechCraft;

ShortcutManager::ShortcutManager (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
}

void ShortcutManager::AddObject (QObject *object)
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("Shortcuts");

	IInfo *ii = qobject_cast<IInfo*> (object);
	IHaveShortcuts *ihs = qobject_cast<IHaveShortcuts*> (object);

	if (!ii || !ihs)
		return;

	QStringList pstrings;
	pstrings << ii->GetName ()
		<< ii->GetInfo ();

	QTreeWidgetItem *parent = new QTreeWidgetItem (Ui_.Tree_, pstrings);
	parent->setIcon (0, ii->GetIcon ());
	parent->setData (0, RoleObject,
			QVariant::fromValue<QObject*> (object));

	QMap<QString, ActionInfo> info = ihs->GetActionInfo ();

	QStringList names = info.keys ();

	settings.beginGroup (ii->GetName ());
	Q_FOREACH (QString name, names)
	{
		QKeySequence sequence = settings.value (name,
				info [name].Default_).value<QKeySequence> ();

		QStringList strings;
		strings << info [name].UserVisibleText_
			<< sequence.toString ();
		QTreeWidgetItem *item = new QTreeWidgetItem (parent, strings);
		item->setIcon (0, info [name].Icon_);
		item->setData (0, RoleOriginalName, name);
		item->setData (0, RoleSequence, sequence);
		parent->setExpanded (true);

		if (sequence != info [name].Default_)
			ihs->SetShortcut (name, sequence);
	}

	settings.endGroup ();
	settings.endGroup ();
}

QKeySequence ShortcutManager::GetShortcut (const QObject *object,
		const QString& originalName) const
{
	for (int i = 0, size = Ui_.Tree_->topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *objectItem = Ui_.Tree_->topLevelItem (i);
		if (objectItem->data (0, RoleObject).value<QObject*> () != object)
			continue;

		for (int j = 0, namesSize = objectItem->childCount ();
				j < namesSize; ++j)
		{
			QTreeWidgetItem *item = objectItem->child (j);
			if (item->data (0, RoleOriginalName).toString () == originalName)
				return item->data (0, RoleSequence).value<QKeySequence> ();
		}
		return QKeySequence ();
	}
	return QKeySequence ();
}

void ShortcutManager::on_Tree__itemActivated (QTreeWidgetItem *item)
{
	// Root or something
	if (item->data (0, RoleOriginalName).toString ().isEmpty ())
		return;

	std::auto_ptr<KeySequencer> dia (new KeySequencer (this));
	if (dia->exec () == QDialog::Rejected)
		return;

	QKeySequence newSeq = dia->GetResult ();
	if (item->data (0, RoleOldSequence).isNull ())
		item->setData (0, RoleOldSequence,
				item->data (0, RoleSequence));
	item->setData (0, RoleSequence, newSeq);
	item->setText (1, newSeq.toString ());
}

void ShortcutManager::accept ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("Shortcuts");
	for (int i = 0, size = Ui_.Tree_->topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *objectItem = Ui_.Tree_->topLevelItem (i);
		for (int j = 0, namesSize = objectItem->childCount ();
				j < namesSize; ++j)
		{
			QObject *o = objectItem->data (0, RoleObject).value<QObject*> ();
			IInfo *ii = qobject_cast<IInfo*> (o);
			IHaveShortcuts *ihs = qobject_cast<IHaveShortcuts*> (o);
			settings.beginGroup (ii->GetName ());

			QTreeWidgetItem *item = objectItem->child (j);
			if (!item->data (0, RoleOldSequence).isNull ())
			{
				QString name = item->data (0, RoleOriginalName).toString ();
				QKeySequence sequence = item->data (0, RoleSequence).value<QKeySequence> ();

				settings.setValue (name, sequence);
				item->setData (0, RoleOldSequence, QVariant ());
				ihs->SetShortcut (name, sequence);
			}

			settings.endGroup ();
		}
	}
	settings.endGroup ();

	QDialog::accept ();
}

void ShortcutManager::reject ()
{
	for (int i = 0, size = Ui_.Tree_->topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *objectItem = Ui_.Tree_->topLevelItem (i);
		for (int j = 0, namesSize = objectItem->childCount ();
				j < namesSize; ++j)
		{
			QTreeWidgetItem *item = objectItem->child (j);
			if (!item->data (0, RoleOldSequence).isNull ())
			{
				QKeySequence seq = item->data (0, RoleOldSequence).value<QKeySequence> ();
				item->setData (0, RoleSequence, seq);
				item->setData (0, RoleOldSequence, QVariant ());
				item->setText (1, seq.toString ());
			}
		}
	}

	QDialog::reject ();
}

