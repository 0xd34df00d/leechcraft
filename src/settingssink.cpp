/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "settingssink.h"
#include <QPushButton>
#include <QtDebug>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

Q_DECLARE_METATYPE (LeechCraft::Util::XmlSettingsDialog*);

namespace LeechCraft
{
	using namespace Util;

	SettingsSink::SettingsSink (const QString& name,
			XmlSettingsDialog* dialog,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		// Because Qt Designer inserts one.
		Ui_.Dialogs_->removeWidget (Ui_.Dialogs_->currentWidget ());

		Add (name, windowIcon (), dialog);

		PluginsItem_ = new QTreeWidgetItem (Ui_.Tree_, QStringList (tr ("Plugins")));
		PluginsItem_->setIcon (0, QIcon (":/resources/images/defaultpluginicon.svg"));
	}

	SettingsSink::~SettingsSink ()
	{
	}

	void SettingsSink::AddDialog (const QObject *object)
	{
		IInfo *info = qobject_cast<IInfo*> (object);
		IHaveSettings *ihs = qobject_cast<IHaveSettings*> (object);

		Add (info->GetName (), info->GetIcon (), ihs->GetSettingsDialog ().get ());

		Ui_.Dialogs_->setCurrentIndex (0);
	}

	void SettingsSink::Add (const QString& name, const QIcon& wicon,
			XmlSettingsDialog *widget)
	{
		Ui_.Dialogs_->addWidget (widget);
		adjustSize ();

		QTreeWidgetItem *parent = name == "LeechCraft" ?
				new QTreeWidgetItem (Ui_.Tree_, QStringList (name)) :
				new QTreeWidgetItem (PluginsItem_, QStringList (name));
		parent->setData (0, RDialog, QVariant::fromValue<XmlSettingsDialog*> (widget));
		parent->setIcon (0,
				wicon.isNull () ?
					QIcon (":/resources/images/defaultpluginicon.svg") :
					wicon);
		if (widget->GetPages ().size () > 1)
			Q_FOREACH (QString page, widget->GetPages ())
				new QTreeWidgetItem (parent, QStringList (page));

		connect (this,
				SIGNAL (accepted ()),
				widget,
				SLOT (accept ()));
		connect (Ui_.Buttons_->button (QDialogButtonBox::Apply),
				SIGNAL (released ()),
				widget,
				SLOT (accept ()));
		connect (this,
				SIGNAL (rejected ()),
				widget,
				SLOT (reject ()));

		Ui_.Tree_->expandAll ();
		Ui_.Tree_->sortItems (0, Qt::AscendingOrder);
	}

	void SettingsSink::on_Tree__currentItemChanged (QTreeWidgetItem *current)
	{
		if (current == Ui_.Tree_->invisibleRootItem () ||
				current == PluginsItem_)
			return;

		int pindex = 0;
		XmlSettingsDialog *dialog = current->data (0, RDialog).value<XmlSettingsDialog*> ();

		if (!dialog)
		{
			QTreeWidgetItem *parent = current->parent ();
			dialog = parent->data (0, RDialog).value<XmlSettingsDialog*> ();
			pindex = parent->indexOfChild (current);
		}

		if (!dialog)
			return;

		Ui_.Dialogs_->setCurrentWidget (dialog);
		dialog->SetPage (pindex);
	}
};

