/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "effectsmanager.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <xmlsettingsdialog/datasourceroles.h>
#include "interfaces/lmp/ifilterconfigurator.h"
#include "engine/path.h"
#include "engine/rgfilter.h"
#include "xmlsettingsmanager.h"

namespace LC::LMP
{
	QDataStream& operator<< (QDataStream& out, const SavedFilterInfo& info)
	{
		out << static_cast<quint8> (1)
				<< info.FilterId_
				<< info.InstanceId_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, SavedFilterInfo& info)
	{
		quint8 version = 0;

		in >> version;
		if (version != 1)
			return in;

		in >> info.FilterId_
				>> info.InstanceId_;

		return in;
	}

	EffectsManager::EffectsManager (Path *path, QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	, Path_ { path }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Effect") });
		Model_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::Enum,
				DataSources::DataSourceRole::FieldType);

		RegisterEffect ({
				"org.LeechCraft.LMP.RG",
				"ReplayGain",
				{},
				true,
				[path] (const QByteArray&, IPath*) { return new RGFilter { path }; }
			});
	}

	QAbstractItemModel* EffectsManager::GetEffectsModel () const
	{
		return Model_;
	}

	void EffectsManager::RegisterEffect (const EffectInfo& info)
	{
		RegisteredEffects_ << info;
	}

	void EffectsManager::RegisteringFinished ()
	{
		const auto& data = XmlSettingsManager::Instance ().property ("AddedFilters").value<QList<SavedFilterInfo>> ();

		for (const auto& filter : data)
		{
			const auto& id = filter.FilterId_;
			const auto effectPos = std::find_if (RegisteredEffects_.cbegin (), RegisteredEffects_.cend (),
					[&id] (const EffectInfo& info) { return info.ID_ == id; });

			if (effectPos == RegisteredEffects_.end ())
			{
				qWarning () << "cannot recover filter"
						<< id
						<< "; not available";
				continue;
			}

			RestoreFilter (effectPos, filter.InstanceId_);
		}

		UpdateHeaders ();

		ReemitEffectsList ();
	}

	IFilterElement* EffectsManager::RestoreFilter (QList<EffectInfo>::const_iterator effectPos, const QByteArray& instanceId)
	{
		auto modelItem = new QStandardItem { effectPos->Name_ };
		modelItem->setEditable (false);
		modelItem->setIcon (effectPos->Icon_);
		Model_->appendRow (modelItem);

		const auto elem = effectPos->EffectFactory_ (instanceId, Path_);
		elem->InsertInto (Path_);

		Filters_ << elem;

		return elem;
	}

	void EffectsManager::UpdateHeaders ()
	{
		QVariantList items;
		for (const auto& effect : RegisteredEffects_)
		{
			const auto& id = effect.ID_;

			if (effect.IsSingleton_ &&
					std::any_of (Filters_.begin (), Filters_.end (),
							[&id] (IFilterElement *elem)
							{
								return elem->GetEffectId () == id;
							}))
				continue;

			items << QVariantMap
				{
					{ "Icon", QVariant::fromValue (effect.Icon_) },
					{ "Name", effect.Name_ },
					{ "ID", id },
				};
		}

		Model_->horizontalHeaderItem (0)->setData (items,
				DataSources::DataSourceRole::FieldValues);
	}

	void EffectsManager::SaveFilters () const
	{
		QList<SavedFilterInfo> data;
		for (const auto filter : Filters_)
		{
			const auto& filterId = filter->GetEffectId ();
			const auto& instanceId = filter->GetInstanceId ();

			data.append ({ filterId, instanceId });
		}

		XmlSettingsManager::Instance ().setProperty ("AddedFilters", QVariant::fromValue (data));
	}

	void EffectsManager::ReemitEffectsList ()
	{
		QStringList result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << Model_->item (i)->text ();
		emit effectsListChanged (result);
	}

	void EffectsManager::showEffectConfig (int row)
	{
		const auto filter = Filters_.value (row);
		if (!filter)
		{
			qWarning () << "invalid row"
					<< row
					<< "of"
					<< Filters_.size ();
			return;
		}

		if (const auto conf = filter->GetConfigurator ())
			conf->OpenDialog ();
		else
		{
			const auto& name = Model_->item (row)->text ();
			QMessageBox::warning (nullptr,
					tr ("Effects configuration"),
					tr ("Seems like %1 doesn't have any settings to configure.")
						.arg ("<em>" + name + "</em>"));
		}
	}

	void EffectsManager::addRequested (const QString&, const QVariantList& datas)
	{
		const auto& id = datas.value (0).toByteArray ();
		const auto effectPos = std::find_if (RegisteredEffects_.cbegin (), RegisteredEffects_.cend (),
				[&id] (const EffectInfo& info) { return info.ID_ == id; });
		if (effectPos == RegisteredEffects_.end ())
		{
			qWarning () << "effect"
					<< id
					<< "not found";
			return;
		}

		const auto elem = RestoreFilter (effectPos, {});
		if (const auto conf = elem->GetConfigurator ())
			conf->OpenDialog ();

		UpdateHeaders ();
		SaveFilters ();
		ReemitEffectsList ();
	}

	void EffectsManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		for (const auto& index : indexes)
		{
			const auto elem = Filters_.takeAt (index.row ());
			if (!elem)
			{
				qWarning () << "invalid row"
						<< index
						<< "of"
						<< Filters_.size ();
				continue;
			}

			elem->RemoveFrom (Path_);
			delete elem;

			Model_->removeRow (index.row ());
		}

		UpdateHeaders ();
		SaveFilters ();
		ReemitEffectsList ();
	}

	void EffectsManager::customButtonPressed (const QString&, const QByteArray&, int row)
	{
		showEffectConfig (row);
	}
}
