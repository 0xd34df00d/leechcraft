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

#include "chatstyleoptionmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "interfaces/azoth/ichatstyleresourcesource.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatStyleOptionManager::ChatStyleOptionManager (const QByteArray& optionName, QObject *parent)
	: QObject (parent)
	, OptionName_ (optionName)
	, OptionsModel_ (new SourceTrackingModel<IChatStyleResourceSource> (QStringList (tr ("Chat style")), this))
	, VariantModel_ (new QStandardItemModel (this))
	{
		XmlSettingsManager::Instance ().RegisterObject (optionName, this,
				"handleChatStyleSelected", XmlSettingsManager::EventFlag::Select);
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
		OptionsModel_->AddSource (src);

		const QString& option = XmlSettingsManager::Instance ()
				.property (OptionName_).toString ();
		auto model = src->GetOptionsModel ();
		for (int i = 0, size = model->rowCount (); i < size; ++i)
			if (model->data (model->index (i, 0)).toString () == option)
			{
				handleChatStyleSelected (option);
				break;
			}
	}

	void ChatStyleOptionManager::handleChatStyleSelected (const QVariant& val)
	{
		VariantModel_->clear ();

		const QString& style = val.toString ();
		auto source = OptionsModel_->GetSourceForOption (style);
		if (!source)
			return;

		Q_FOREACH (const QString& var, source->GetVariantsForPack (style))
			VariantModel_->appendRow (new QStandardItem (var));
	}
}
}
