#include "shortcutmanager.h"
#include <memory>
#include <QtDebug>
#include <QSettings>
#include <interfaces/iinfo.h>
#include <interfaces/ihaveshortcuts.h>
#include "keysequencer.h"

using namespace LeechCraft;

ShortcutManager::ShortcutManager (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
}

void ShortcutManager::AddObject (QObject *object)
{
	IInfo *ii = qobject_cast<IInfo*> (object);
	if (!ii)
	{
		qWarning () << Q_FUNC_INFO
			<< object
			<< "couldn't be casted to IInfo";
		return;
	}
	AddObject (object, ii->GetName (), ii->GetInfo (), ii->GetIcon ());
}

void ShortcutManager::AddObject (QObject *object,
		const QString& objName, const QString& objDescr,
		const QIcon& objIcon)
{
	for (int i = 0, size = Ui_.Tree_->topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *objectItem = Ui_.Tree_->topLevelItem (i);
		QObject *o = objectItem->data (0, RoleObject).value<QObject*> ();
		if (o == object)
			return;
	}

	IHaveShortcuts *ihs = qobject_cast<IHaveShortcuts*> (object);

	if (!ihs)
	{
		qWarning () << Q_FUNC_INFO
			<< object
			<< "could not be casted to IHaveShortcuts";
		return;
	}

	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("Shortcuts");

	QStringList pstrings;
	pstrings << objName
		<< objDescr;

	QTreeWidgetItem *parent = new QTreeWidgetItem (Ui_.Tree_, pstrings);
	parent->setIcon (0, objIcon);
	parent->setData (0, RoleObject,
			QVariant::fromValue<QObject*> (object));

	QMap<int, ActionInfo> info = ihs->GetActionInfo ();

	QList<int> names = info.keys ();

	settings.beginGroup (objName);
	Q_FOREACH (int name, names)
	{
		QKeySequence sequence = settings.value (QString::number (name),
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
		int originalName) const
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
			if (item->data (0, RoleOriginalName).toInt () == originalName)
				return item->data (0, RoleSequence).value<QKeySequence> ();
		}
		return QKeySequence ();
	}
	const_cast<ShortcutManager*> (this)->AddObject (const_cast<QObject*> (object));
	return GetShortcut (object, originalName);
}

void ShortcutManager::on_Tree__itemActivated (QTreeWidgetItem *item)
{
	// Root or something
	if (item->data (0, RoleOriginalName).isNull ())
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
				int name = item->data (0, RoleOriginalName).toInt ();
				QKeySequence sequence = item->data (0, RoleSequence).value<QKeySequence> ();

				settings.setValue (QString::number (name), sequence);
				item->setData (0, RoleOldSequence, QVariant ());
				ihs->SetShortcut (name, sequence);
			}

			settings.endGroup ();
		}
	}
	settings.endGroup ();
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
}

