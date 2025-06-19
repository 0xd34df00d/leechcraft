/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chatstyleoptionmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "interfaces/azoth/ichatstyleresourcesource.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	ChatStyleOptionManager::ChatStyleOptionManager (const QByteArray& optionName, QObject *parent)
	: QObject { parent }
	, OptionName_ { optionName }
	, CoreStylesModel_ { Core::Instance ().GetChatStyleSourceModel () }
	, EmptyOptModel_ { new QStandardItemModel { 0, 1, this } }
	, OptionsModel_ { new Util::MergeModel { { {} }, this } }
	, VariantModel_ { new QStandardItemModel { this } }
	{
		if (!optionName.isEmpty ())
			XmlSettingsManager::Instance ().RegisterObject (optionName, this,
					&ChatStyleOptionManager::StyleSelected, XmlSettingsManager::EventFlag::Select);

		OptionsModel_->AddModel (EmptyOptModel_);
		OptionsModel_->AddModel (CoreStylesModel_);
	}

	void ChatStyleOptionManager::AddEmptyVariant ()
	{
		EmptyOptModel_->appendRow (new QStandardItem {});
	}

	QAbstractItemModel* ChatStyleOptionManager::GetStyleModel () const
	{
		return OptionsModel_;
	}

	QAbstractItemModel* ChatStyleOptionManager::GetVariantModel () const
	{
		return VariantModel_;
	}

	void ChatStyleOptionManager::AddChatStyleResourceSource (IChatStyleResourceSource *src)
	{
		if (OptionName_.isEmpty ())
			return;

		const auto& option = XmlSettingsManager::Instance ()
				.property (OptionName_).toString ();
		auto model = src->GetOptionsModel ();
		for (int i = 0, size = model->rowCount (); i < size; ++i)
			if (model->data (model->index (i, 0)).toString () == option)
			{
				StyleSelected (option);
				break;
			}
	}

	void ChatStyleOptionManager::StyleSelected (const QString& style)
	{
		VariantModel_->clear ();

		if (style.isEmpty ())
			return;

		const auto source = CoreStylesModel_->GetSourceForOption (style);
		if (!source)
			return;

		for (const auto& var : source->GetVariantsForPack (style))
			VariantModel_->appendRow (new QStandardItem { var });
	}
}
}
