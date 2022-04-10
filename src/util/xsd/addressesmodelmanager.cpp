/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addressesmodelmanager.h"
#include <QStandardItemModel>
#include <QNetworkInterface>
#include <xmlsettingsdialog/datasourceroles.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC
{
namespace Util
{
	namespace
	{
		auto GetAddrInfos ()
		{
			const auto& addrs = QNetworkInterface::allAddresses ();
			QList<DataSources::EnumValueInfo> hosts;
			hosts.reserve (addrs.size ());
			for (const auto& addr : addrs)
			{
				if (!addr.scopeId ().isEmpty ())
					continue;

				const auto& str = addr.toString ();
				hosts.push_back ({
						.Name_ = str,
						.UserData_ = str
					});
			}
			return hosts;
		}
	}

	AddressesModelManager::AddressesModelManager (BaseSettingsManager *bsm, int defaultPort, QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	, BSM_ { bsm }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Host"), tr ("Port") });

		using namespace DataSources;

		const auto hostHeader = Model_->horizontalHeaderItem (0);
		hostHeader->setData (DataFieldType::Enum, DataSourceRole::FieldType);
		hostHeader->setData (QVariant::fromValue<EnumValueInfoGenerator> (GetAddrInfos), DataSourceRole::FieldValuesGenerator);

		Model_->horizontalHeaderItem (1)->setData (DataFieldType::Integer, DataSourceRole::FieldType);

		const auto& addrs = BSM_->Property ("ListenAddresses",
				QVariant::fromValue (GetLocalAddresses (defaultPort))).value<AddrList_t> ();
		qDebug () << Q_FUNC_INFO << addrs;
		for (const auto& addr : addrs)
			AppendRow (addr);
	}

	void AddressesModelManager::RegisterTypes ()
	{
		qRegisterMetaType<AddrList_t> ("LC::Util::AddrList_t");
		qRegisterMetaTypeStreamOperators<AddrList_t> ();
	}

	QAbstractItemModel* AddressesModelManager::GetModel () const
	{
		return Model_;
	}

	AddrList_t AddressesModelManager::GetAddresses () const
	{
		AddrList_t addresses;
		for (auto i = 0; i < Model_->rowCount (); ++i)
		{
			auto hostItem = Model_->item (i, 0);
			auto portItem = Model_->item (i, 1);
			addresses.push_back ({ hostItem->text (), portItem->text () });
		}
		return addresses;
	}

	void AddressesModelManager::SaveSettings () const
	{
		BSM_->setProperty ("ListenAddresses",
				QVariant::fromValue (GetAddresses ()));
	}

	void AddressesModelManager::AppendRow (const QPair<QString, QString>& pair)
	{
		QList<QStandardItem*> items
		{
			new QStandardItem { pair.first },
			new QStandardItem { pair.second }
		};
		for (const auto item : items)
			item->setEditable (false);
		Model_->appendRow (items);

		emit addressesChanged ();
	}

	void AddressesModelManager::addRequested (const QString&, const QVariantList& data)
	{
		const auto port = data.value (1).toInt ();
		if (port < 1024 || port > 65535)
			return;

		AppendRow ({ data.value (0).toString (), QString::number (port) });
		SaveSettings ();
	}

	void AddressesModelManager::removeRequested (const QString&, const QModelIndexList& list)
	{
		for (const auto& item : list)
			Model_->removeRow (item.row ());

		SaveSettings ();
		emit addressesChanged ();
	}
}
}
