/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customstatusesmanager.h"
#include <QStandardItemModel>
#include <QCoreApplication>
#include <xmlsettingsdialog/datasourceroles.h>
#include "interfaces/azoth/azothcommon.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "resourcesmanager.h"
#include "util.h"

namespace LC
{
namespace Azoth
{
	CustomStatusesManager::CustomStatusesManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setColumnCount (3);

		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Status"), tr ("Text") });

		Model_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::String,
				DataSources::DataSourceRole::FieldType);
		Model_->horizontalHeaderItem (1)->setData (DataSources::DataFieldType::Enum,
				DataSources::DataSourceRole::FieldType);
		QVariantList values;
		auto append = [&values] (State state)
		{
			values << QVariant::fromValue<DataSources::EnumValueInfo> ({
					.Icon_ = ResourcesManager::Instance ().GetIconForState (state),
					.Name_ = StateToString (state),
					.UserData_ = QVariant::fromValue (state),
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

	QList<CustomStatus> CustomStatusesManager::GetStates () const
	{
		QList<CustomStatus> result;
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

	void CustomStatusesManager::Add (const CustomStatus& state, int after)
	{
		QList<QStandardItem*> row;
		row << new QStandardItem (state.Name_);
		row << new QStandardItem (ResourcesManager::Instance ().GetIconForState (state.State_),
				StateToString (state.State_));
		row << new QStandardItem (state.Text_);
		row.at (1)->setData (static_cast<int> (state.State_));

		if (after == -1)
			Model_->appendRow (row);
		else
			Model_->insertRow (after, row);
	}

	CustomStatus CustomStatusesManager::GetCustom (int i) const
	{
		return
		{
			Model_->item (i, 0)->text (),
			static_cast<State> (Model_->item (i, 1)->data ().toInt ()),
			Model_->item (i, 2)->text (),
		};
	}

	namespace
	{
		CustomStatus GetState (const QVariantList& vars)
		{
			return
			{
				vars.at (0).toString (),
				vars.at (1).value<State> (),
				vars.at (2).toString ()
			};
		}
	}

	void CustomStatusesManager::addRequested (const QString&, const QVariantList& vars)
	{
		if (vars.size () != Model_->columnCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid vars";
			return;
		}

		Add (GetState (vars));
		Save ();
	}

	void CustomStatusesManager::modifyRequested (const QString&, int row, const QVariantList& vars)
	{
		if (vars.size () != Model_->columnCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid vars";
			return;
		}

		Model_->removeRow (row);
		Add (GetState (vars), row);
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
