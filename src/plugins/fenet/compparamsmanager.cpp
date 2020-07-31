/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "compparamsmanager.h"
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <util/sll/containerconversions.h>

namespace LC::Fenet
{
	CompParamsManager::CompParamsManager (QObject *parent)
	: QObject (parent)
	, ParamsModel_ (new QStandardItemModel (this))
	{
		ParamsModel_->setHorizontalHeaderLabels ({ tr ("Option"), tr ("Value"), tr ("Flag") });
		connect (ParamsModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
	}

	QAbstractItemModel* CompParamsManager::GetModel () const
	{
		return ParamsModel_;
	}

	void CompParamsManager::SetCompInfo (const CompInfo& info)
	{
		if (auto rc = ParamsModel_->rowCount ())
			ParamsModel_->removeRows (0, rc);

		CurrentInfo_ = info;
		auto settings = GetSettings ();

		for (const auto& param : CurrentInfo_.Params_)
		{
			auto descItem = new QStandardItem (param.Desc_);
			descItem->setEditable (false);

			const auto value = settings->value (param.Name_, param.Default_).toDouble ();
			const auto valueItem = new QStandardItem (QString::number (value));
			valueItem->setData (value, Qt::EditRole);
			valueItem->setData (QVariant::fromValue (param), Role::Description);

			auto nameItem = new QStandardItem (param.Name_);

			ParamsModel_->appendRow ({ descItem, valueItem, nameItem });
		}

		const auto& enabledFlags = settings->value ("__Flags").toStringList ();

		for (const auto& flag : CurrentInfo_.Flags_)
		{
			auto descItem = new QStandardItem (flag.Desc_);
			descItem->setEditable (false);
			descItem->setCheckable (true);
			descItem->setCheckState (enabledFlags.contains (flag.Name_) ?
					Qt::Checked :
					Qt::Unchecked);
			descItem->setData (QVariant::fromValue (flag), Role::Description);

			auto valueItem = new QStandardItem ();
			valueItem->setEditable (false);

			auto nameItem = new QStandardItem (flag.Name_);
			nameItem->setEditable (false);

			ParamsModel_->appendRow ({ descItem, valueItem, nameItem });
		}
	}

	QStringList CompParamsManager::GetCompParams (const QString& compName) const
	{
		auto settings = GetSettings (compName);

		auto result = settings->value ("__Flags").toStringList ();
		for (const auto& key : settings->childKeys ())
		{
			if (key == "__Flags")
				continue;

			result << key << QString::number (settings->value (key).toDouble ());
		}
		return result;
	}

	std::shared_ptr<QSettings> CompParamsManager::GetSettings (QString compName) const
	{
		if (compName.isEmpty ())
			compName = CurrentInfo_.Name_;

		auto settings = new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Fenet");
		settings->beginGroup ("Compositors");
		settings->beginGroup (compName);

		return std::shared_ptr<QSettings> (settings,
				[] (QSettings *settings)
				{
					settings->endGroup ();
					settings->endGroup ();
					delete settings;
				});
	}

	void CompParamsManager::handleItemChanged (QStandardItem *item)
	{
		const auto& var = item->data (Role::Description);
		if (var.canConvert<Flag> ())
		{
			const auto& flag = var.value<Flag> ();
			const bool enabled = item->checkState () == Qt::Checked;

			ChangedFlags_ [CurrentInfo_.Name_] [flag.Name_] = enabled;
		}
		else if (var.canConvert<Param> ())
		{
			const auto& param = var.value<Param> ();
			const double value = item->data (Qt::EditRole).toDouble ();

			ChangedParams_ [CurrentInfo_.Name_] [param.Name_] = value;
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown item"
					<< var;
	}

	void CompParamsManager::save ()
	{
		QStringList keys = ChangedFlags_.keys () + ChangedParams_.keys ();
		keys.removeDuplicates ();
		if (keys.isEmpty ())
			return;

		bool changed = ChangedParams_.size ();

		for (const auto& key : keys)
		{
			auto settings = GetSettings (key);

			const auto& params = ChangedParams_ [key];
			for (const auto& name : params.keys ())
				settings->setValue (name, params [name]);

			const auto& oldFlags = Util::AsSet (settings->value ("__Flags").toStringList ());
			auto newFlagsSet = oldFlags;

			const auto& flags = ChangedFlags_ [key];
			for (const auto& name : flags.keys ())
				if (flags [name])
					newFlagsSet << name;
				else
					newFlagsSet.remove (name);

			if (newFlagsSet != oldFlags)
			{
				settings->setValue ("__Flags", QStringList (newFlagsSet.values ()));
				changed = true;
			}
		}

		if (changed)
			emit paramsChanged ();
	}

	void CompParamsManager::revert ()
	{
		ChangedFlags_.clear ();
		ChangedParams_.clear ();
	}
}
