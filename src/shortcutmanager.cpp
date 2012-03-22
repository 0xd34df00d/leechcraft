/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "shortcutmanager.h"
#include <memory>
#include <QStandardItemModel>
#include <QSettings>
#include <QtDebug>
#include <interfaces/iinfo.h>
#include <interfaces/ihaveshortcuts.h>
#include "keysequencer.h"

namespace LeechCraft
{
	ShortcutManager::ShortcutManager (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("Shortcut"));

		Ui_.setupUi (this);
		Ui_.Tree_->setModel (Model_);
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
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto objectItem = Model_->item (i);
			QObject *o = objectItem->data (RoleObject).value<QObject*> ();
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

		QList<QStandardItem*> parentRow;
		parentRow << new QStandardItem (objName);
		parentRow << new QStandardItem (objDescr);
		parentRow.at (0)->setIcon (objIcon);
		parentRow.at (0)->setData (	QVariant::fromValue<QObject*> (object), RoleObject);

		const auto& info = ihs->GetActionInfo ();

		settings.beginGroup (objName);
		Q_FOREACH (const QString& name, info.keys ())
		{
			// FIXME use all the sequences here, not the first one
			const auto& sequences = settings.value (name,
					QVariant::fromValue<QKeySequences_t> (info [name].Seqs_)).value<QKeySequences_t> ();

			const auto& sequence = sequences.value (0);

			QStringList strings;
			strings << info [name].UserVisibleText_
				<< sequence.toString ();

			QList<QStandardItem*> itemRow;
			itemRow << new QStandardItem (info [name].UserVisibleText_);
			itemRow << new QStandardItem (sequence.toString ());
			itemRow.at (0)->setIcon (info [name].Icon_);
			itemRow.at (0)->setData (name, RoleOriginalName);
			itemRow.at (0)->setData (QVariant::fromValue<QKeySequences_t> (sequences), RoleSequence);
			parentRow.at (0)->appendRow (itemRow);

			if (sequences != info [name].Seqs_)
				ihs->SetShortcut (name, sequences);
		}

		Model_->appendRow (parentRow);

		settings.endGroup ();
		settings.endGroup ();

		Ui_.Tree_->resizeColumnToContents (0);
		Ui_.Tree_->expand (parentRow.at (0)->index ());
	}

	QKeySequences_t ShortcutManager::GetShortcuts (QObject *object,
			const QString& originalName)
	{
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto objectItem = Model_->item (i);
			if (objectItem->data (RoleObject).value<QObject*> () != object)
				continue;

			for (int j = 0, namesSize = objectItem->rowCount (); j < namesSize; ++j)
			{
				auto item = objectItem->child (j);
				if (item->data (RoleOriginalName).toString () == originalName)
					return item->data (RoleSequence).value<QKeySequences_t> ();
			}
			return QKeySequences_t ();
		}
		const_cast<ShortcutManager*> (this)->AddObject (const_cast<QObject*> (object));
		return GetShortcuts (object, originalName);
	}

	void ShortcutManager::on_Tree__activated (const QModelIndex& index)
	{
		auto item = Model_->itemFromIndex (index.sibling (index.row (), 0));
		// Root or something
		if (!item || item->data (RoleOriginalName).isNull ())
			return;

		KeySequencer dia (this);
		if (dia.exec () == QDialog::Rejected)
			return;

		QKeySequences_t newSeqs;
		newSeqs << dia.GetResult ();
		if (item->data (RoleOldSequence).isNull ())
			item->setData (item->data (RoleSequence), RoleOldSequence);
		item->setData (QVariant::fromValue<QKeySequences_t> (newSeqs), RoleSequence);

		Model_->itemFromIndex (index.sibling (index.row (), 1))->
				setText (newSeqs.value (0).toString ());
	}

	void ShortcutManager::accept ()
	{
		QSettings settings ("Deviant", "Leechcraft");
		settings.beginGroup ("Shortcuts");
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto objectItem = Model_->item (i);

			for (int j = 0, namesSize = objectItem->rowCount (); j < namesSize; ++j)
			{
				QObject *o = objectItem->data (RoleObject).value<QObject*> ();
				IInfo *ii = qobject_cast<IInfo*> (o);
				IHaveShortcuts *ihs = qobject_cast<IHaveShortcuts*> (o);
				settings.beginGroup (ii->GetName ());

				auto item = objectItem->child (j);
				if (!item->data (RoleOldSequence).isNull ())
				{
					QString name = item->data (RoleOriginalName).toString ();
					const auto& sequences = item->data (RoleSequence).value<QKeySequences_t> ();

					settings.setValue (name, QVariant::fromValue<QKeySequences_t> (sequences));
					item->setData (QVariant (), RoleOldSequence);
					ihs->SetShortcut (name, sequences);
				}

				settings.endGroup ();
			}
		}
		settings.endGroup ();
	}

	void ShortcutManager::reject ()
	{
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto objectItem = Model_->item (i);
			for (int j = 0, namesSize = objectItem->rowCount (); j < namesSize; ++j)
			{
				auto item = objectItem->child (j);
				if (item->data (RoleOldSequence).isNull ())
					continue;

				const auto& seq = item->data (RoleOldSequence).value<QKeySequence> ();
				item->setData (seq, RoleSequence);
				item->setData (QVariant (), RoleOldSequence);

				objectItem->child (j, 1)->setText (seq.toString ());
			}
		}
	}
}
