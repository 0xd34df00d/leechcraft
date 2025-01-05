/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountsmanager.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <xmlsettingsdialog/datasourceroles.h>

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	AccountsManager::AccountsManager (QObject *parent)
	: QObject (parent)
	, AccModel_ (new QStandardItemModel (this))
	{
		AccModel_->setHorizontalHeaderLabels ({ tr ("Account name") });
		AccModel_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::String, DataSources::DataSourceRole::FieldType);

		LoadAccounts ();
	}

	QAbstractItemModel* AccountsManager::GetAccModel () const
	{
		return AccModel_;
	}

	QStringList AccountsManager::GetAccounts () const
	{
		QStringList result;
		for (int i = 0; i < AccModel_->rowCount (); ++i)
			result << AccModel_->item (i)->text ();
		return result;
	}

	void AccountsManager::SaveAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_MP3Tunes");
		settings.beginGroup ("Accounts");
		settings.beginWriteArray ("List");
		for (int i = 0; i < AccModel_->rowCount (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("AccName", AccModel_->item (i)->text ());
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void AccountsManager::LoadAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_MP3Tunes");
		settings.beginGroup ("Accounts");
		const int size = settings.beginReadArray ("List");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const auto& str = settings.value ("AccName").toString ();
			AccModel_->appendRow (new QStandardItem (str));
		}
		settings.endArray ();
		settings.endGroup ();

		emit accountsChanged ();
	}

	void AccountsManager::addRequested (const QString&, const QVariantList& variants)
	{
		const auto& str = variants.value (0).toString ();
		if (str.isEmpty ())
			return;

		AccModel_->appendRow (new QStandardItem (str));

		SaveAccounts ();
		emit accountsChanged ();
	}

	void AccountsManager::removeRequested (const QString&, const QModelIndexList& indices)
	{
		QList<QStandardItem*> items;
		for (const auto& idx : indices)
			items << AccModel_->itemFromIndex (idx);
		items.removeAll (nullptr);

		for (auto item : items)
			AccModel_->removeRow (item->row ());

		SaveAccounts ();

		emit accountsChanged ();
	}
}
}
}
