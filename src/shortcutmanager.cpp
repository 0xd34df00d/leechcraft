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
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QtDebug>
#include <interfaces/iinfo.h>
#include <interfaces/ihaveshortcuts.h>
#include "keysequencer.h"

namespace LeechCraft
{
	class SMFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		SMFilterProxyModel (QObject *parent = 0)
		: QSortFilterProxyModel (parent)
		{
		}
	protected:
		bool filterAcceptsRow (int row, const QModelIndex& parent) const
		{
			if (!parent.isValid ())
				return true;

			const QString& filter = filterRegExp ().pattern ();
			if (filter.isEmpty ())
				return true;

			auto checkStr = [row, parent, &filter, this] (int col)
			{
				const QString& content = this->sourceModel ()->
						index (row, col, parent).data ().toString ();
				return content.contains (filter, Qt::CaseInsensitive);
			};
			if (checkStr (0) || checkStr (1))
				return true;
			return false;
		}
	};

	ShortcutManager::ShortcutManager (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	, Filter_ (new SMFilterProxyModel (this))
	{
		Filter_->setDynamicSortFilter (true);
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("Shortcut"));
		Filter_->setSourceModel (Model_);
		Filter_->sort (0);

		Ui_.setupUi (this);
		Ui_.Tree_->setModel (Filter_);
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				Filter_,
				SLOT (setFilterFixedString (QString)));
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				Ui_.Tree_,
				SLOT (expandAll ()));
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
			QObject *o = objectItem->data (Roles::Object).value<QObject*> ();
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
		parentRow.at (0)->setData (	QVariant::fromValue<QObject*> (object), Roles::Object);

		const auto& info = ihs->GetActionInfo ();

		settings.beginGroup (objName);
		Q_FOREACH (const QString& name, info.keys ())
		{
			// FIXME use all the sequences here, not the first one
			const auto& sequences = settings.value (name,
					QVariant::fromValue<QKeySequences_t> (info [name].Seqs_)).value<QKeySequences_t> ();
			const auto& sequence = sequences.value (0);

			QList<QStandardItem*> itemRow;
			itemRow << new QStandardItem (info [name].UserVisibleText_);
			itemRow << new QStandardItem (sequence.toString ());
			itemRow.at (0)->setIcon (info [name].Icon_);
			itemRow.at (0)->setData (name, Roles::OriginalName);
			itemRow.at (0)->setData (QVariant::fromValue<QKeySequences_t> (sequences), Roles::Sequence);
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
			if (objectItem->data (Roles::Object).value<QObject*> () != object)
				continue;

			for (int j = 0, namesSize = objectItem->rowCount (); j < namesSize; ++j)
			{
				auto item = objectItem->child (j);
				if (item->data (Roles::OriginalName).toString () == originalName)
					return item->data (Roles::Sequence).value<QKeySequences_t> ();
			}
			return QKeySequences_t ();
		}
		AddObject (const_cast<QObject*> (object));
		return GetShortcuts (object, originalName);
	}

	void ShortcutManager::on_Tree__activated (const QModelIndex& index)
	{
		auto item = Model_->itemFromIndex (index.sibling (index.row (), 0));
		// Root or something
		if (!item || item->data (Roles::OriginalName).isNull ())
			return;

		KeySequencer dia (this);
		if (dia.exec () == QDialog::Rejected)
			return;

		QKeySequences_t newSeqs;
		newSeqs << dia.GetResult ();
		if (item->data (Roles::OldSequence).isNull ())
			item->setData (item->data (Roles::Sequence), Roles::OldSequence);
		item->setData (QVariant::fromValue<QKeySequences_t> (newSeqs), Roles::Sequence);

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
				QObject *o = objectItem->data (Roles::Object).value<QObject*> ();
				IInfo *ii = qobject_cast<IInfo*> (o);
				IHaveShortcuts *ihs = qobject_cast<IHaveShortcuts*> (o);
				settings.beginGroup (ii->GetName ());

				auto item = objectItem->child (j);
				if (!item->data (Roles::OldSequence).isNull ())
				{
					QString name = item->data (Roles::OriginalName).toString ();
					const auto& sequences = item->data (Roles::Sequence).value<QKeySequences_t> ();

					settings.setValue (name, QVariant::fromValue<QKeySequences_t> (sequences));
					item->setData (QVariant (), Roles::OldSequence);
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
				if (item->data (Roles::OldSequence).isNull ())
					continue;

				const auto& seq = item->data (Roles::OldSequence).value<QKeySequence> ();
				item->setData (seq, Roles::Sequence);
				item->setData (QVariant (), Roles::OldSequence);

				objectItem->child (j, 1)->setText (seq.toString ());
			}
		}
	}
}
