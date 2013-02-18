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

#include "customstatusesmanager.h"
#include <QStandardItemModel>
#include <QCoreApplication>
#include <xmlsettingsdialog/datasourceroles.h>
#include <util/util.h>
#include "interfaces/azoth/azothcommon.h"
#include "core.h"
#include "proxyobject.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	CustomStatusesManager::CustomStatusesManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setColumnCount (3);

#if QT_VERSION >= 0x040800
		Model_->setHorizontalHeaderLabels ({tr ("Name"), tr ("Status"), tr ("Text") });
#endif

		Model_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::String,
				DataSources::DataSourceRole::FieldType);
		Model_->horizontalHeaderItem (1)->setData (DataSources::DataFieldType::Enum,
				DataSources::DataSourceRole::FieldType);
		QVariantList values;
		auto append = [&values] (State state)
		{
			values << Util::MakeMap<QString, QVariant> ({
					{ "Name", ProxyObject ().StateToString (state) },
					{ "Icon", Core::Instance ().GetIconForState (state) },
					{ "ID", QVariant::fromValue (state) }
				});
		};
		append (State::SOnline);
		append (State::SAway);
		append (State::SXA);
		append (State::SDND);
		append (State::SChat);
		append (State::SOffline);
		Model_->horizontalHeaderItem (1)->setData (values, DataSources::DataSourceRole::FieldValues);

		Model_->horizontalHeaderItem (2)->setData (DataSources::DataFieldType::String,
				DataSources::DataSourceRole::FieldType);

		Load ();
	}

	QAbstractItemModel* CustomStatusesManager::GetModel () const
	{
		return Model_;
	}

	QList<CustomStatusesManager::CustomState> CustomStatusesManager::GetStates () const
	{
		QList<CustomState> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << GetCustom (i);
		return result;
	}

	void CustomStatusesManager::Save ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("CustomStatuses");
		settings.beginWriteArray ("Statuses");
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			settings.setArrayIndex (i);
			const auto& state = GetCustom (i);
			settings.setValue ("Name", state.Name_);
			settings.setValue ("State", static_cast<int> (state.State_));
			settings.setValue ("Text", state.Text_);
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void CustomStatusesManager::Load ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("CustomStatuses");
		for (int i = 0, size = settings.beginReadArray ("Statuses"); i < size; ++i)
		{
			settings.setArrayIndex (i);
			const auto& name = settings.value ("Name").toString ();
			const auto state = static_cast<State> (settings.value ("State").toInt ());
			const auto& text = settings.value ("Text").toString ();

			Add ({ name, state, text });
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void CustomStatusesManager::Add (const CustomState& state)
	{
		ProxyObject proxy;

		QList<QStandardItem*> row;
		row << new QStandardItem (state.Name_);
		row << new QStandardItem (Core::Instance ().GetIconForState (state.State_),
				proxy.StateToString (state.State_));
		row << new QStandardItem (state.Text_);
		Model_->appendRow (row);
		row.at (1)->setData (static_cast<int> (state.State_));
	}

	CustomStatusesManager::CustomState CustomStatusesManager::GetCustom (int i) const
	{
		return
		{
			Model_->item (i, 0)->text (),
			static_cast<State> (Model_->item (i, 1)->data ().toInt ()),
			Model_->item (i, 2)->text (),
		};
	}

	void CustomStatusesManager::addRequested (const QString&, const QVariantList& vars)
	{
		if (vars.size () != Model_->columnCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid vars";
			return;
		}

		const auto& name = vars.at (0).toString ();
		const auto state = vars.at (1).value<State> ();
		const auto& text = vars.at (2).toString ();
		Add ({ name, state, text });

		Save ();
	}

	void CustomStatusesManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		QList<int> rows;
		for (const auto& index : indexes)
			rows << index.row ();

		std::sort (rows.begin (), rows.end (), [] (int l, int r) { return r < l; });

		for (int row : rows)
			Model_->removeRow (row);

		Save ();
	}
}
}
