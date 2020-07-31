/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customsitesmanager.h"
#include "xmlsettingsmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <xmlsettingsdialog/datasourceroles.h>

namespace LC::Poshuku::SpeedDial
{
	CustomSitesManager::CustomSitesManager ()
	: Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Site name"), "URL" });
		Model_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::String,
				DataSources::DataSourceRole::FieldType);
		Model_->horizontalHeaderItem (1)->setData (DataSources::DataFieldType::Url,
				DataSources::DataSourceRole::FieldType);

		LoadSettings ();
	}

	QAbstractItemModel* CustomSitesManager::GetModel () const
	{
		return Model_;
	}

	TopList_t CustomSitesManager::GetTopList () const
	{
		TopList_t result;
		for (const auto& addr : GetAddresses ())
			result.append ({ addr.second, addr.first });
		return result;
	}

	AddrList_t CustomSitesManager::GetAddresses () const
	{
		AddrList_t result;

		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			const QUrl url { Model_->item (i, 1)->text () };
			const auto& name = Model_->item (i, 0)->text ();

			result.append ({ name, url });
		}

		return result;
	}

	void CustomSitesManager::LoadSettings ()
	{
		const auto& addrs = XmlSettingsManager::Instance ()
				.property ("Addresses").value<AddrList_t> ();

		for (const auto& addr : addrs)
			Add (addr);
	}

	void CustomSitesManager::SaveSettings ()
	{
		const auto& variant = QVariant::fromValue (GetAddresses ());
		XmlSettingsManager::Instance ().setProperty ("Addresses", variant);
	}

	void CustomSitesManager::Add (const Addr_t& addr)
	{
		QList<QStandardItem*> row
		{
			new QStandardItem (addr.first),
			new QStandardItem (addr.second.toString ())
		};

		for (auto item : row)
			item->setEditable (false);

		Model_->appendRow (row);
	}

	void CustomSitesManager::addRequested (const QString&, const QVariantList& datas)
	{
		Add ({ datas.value (0).toString (), QUrl { datas.value (1).toString () } });
		SaveSettings ();
	}

	void CustomSitesManager::modifyRequested (const QString&, int row, const QVariantList& datas)
	{
		Model_->item (row, 0)->setText (datas.value (0).toString ());
		Model_->item (row, 1)->setText (datas.value (1).toString ());

		SaveSettings ();
	}

	void CustomSitesManager::removeRequested (const QString&, const QModelIndexList& list)
	{
		for (const auto& item : list)
			Model_->removeRow (item.row ());

		SaveSettings ();
	}
}
