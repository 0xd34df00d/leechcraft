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

#include "showconfigdialog.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QIcon>
#include <QtDebug>

namespace LeechCraft
{
namespace Sidebar
{
	ShowConfigDialog::ShowConfigDialog (const QString& context, QWidget *parent)
	: QDialog (parent)
	, Context_ (context)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ActionsView_->setModel (Model_);

		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (saveSettings ()));
		connect (this,
				SIGNAL (rejected ()),
				this,
				SLOT (reloadSettings ()));

		reloadSettings ();
	}

	bool ShowConfigDialog::CheckAction (const QString& id, QAction *act, bool def)
	{
		AllActions_ [id] << act;
		connect (act,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleActionDestroyed ()));

		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto item = Model_->item (i);
			if (item->data (Roles::ActionID).toString () == id)
			{
				const bool result = item->checkState () == Qt::Checked;
				if (!result)
					HiddenActions_ [id] << act;
				return result;
			}
		}

		QStandardItem *item = new QStandardItem (act->icon (), act->text ());
		item->setCheckState (def ? Qt::Checked : Qt::Unchecked);
		item->setToolTip (act->toolTip ());
		item->setData (id, Roles::ActionID);
		item->setCheckable (true);
		item->setEditable (false);
		Model_->appendRow (item);

		return def;
	}

	void ShowConfigDialog::saveSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Sidebar");
		settings.beginGroup (Context_);
		settings.beginWriteArray ("Actions");

		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto item = Model_->item (i);

			const bool enabled = item->checkState () == Qt::Checked;
			const QString& id = item->data (Roles::ActionID).toString ();

			if (enabled && HiddenActions_.contains (id))
				emit showActions (HiddenActions_.take (id));
			else if (!enabled &&
					!HiddenActions_.contains (id) &&
					AllActions_.contains (id))
			{
				const auto& acts = AllActions_ [id];
				HiddenActions_ [id] = acts;
				emit hideActions (acts);
			}

			settings.setArrayIndex (i);
			settings.setValue ("Enabled", enabled);
			settings.setValue ("ID", id);
			settings.setValue ("Text", item->text ());
			settings.setValue ("Tooltip", item->toolTip ());
			settings.setValue ("Icon", item->icon ());
		}

		settings.endArray ();
		settings.endGroup ();
	}

	void ShowConfigDialog::reloadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Sidebar");
		settings.beginGroup (Context_);
		const int size = settings.beginReadArray ("Actions");

		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);

			const auto& icon = settings.value ("Icon").value<QIcon> ();
			QStandardItem *item = new QStandardItem (icon,
					settings.value ("Text").toString ());
			item->setCheckState (settings.value ("Enabled").toBool () ?
						Qt::Checked :
						Qt::Unchecked);
			item->setToolTip (settings.value ("Tooltip").toString ());
			item->setData (settings.value ("ID"), Roles::ActionID);
			item->setCheckable (true);
			item->setEditable (false);
			Model_->appendRow (item);
		}

		settings.endArray ();
		settings.endGroup ();
	}

	void ShowConfigDialog::handleActionDestroyed ()
	{
		QAction *act = static_cast<QAction*> (sender ());

		auto remove = [act] (ID2Actions_t& actions)
		{
			Q_FOREACH (const QString& key, actions.keys ())
				if (actions [key].contains (act))
				{
					actions [key].removeAll (act);
					break;
				}
		};

		remove (HiddenActions_);
		remove (AllActions_);
	}
}
}
