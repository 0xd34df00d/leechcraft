/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
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

#include "contentsdisplaydialog.h"
#include <QStandardItemModel>
#include <interfaces/iinfo.h>
#include "interfaces/secman/istorageplugin.h"
#include "core.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
	namespace
	{
		QString Unserialize (const QVariant& var)
		{
			if (var.canConvert<QString> ())
				return var.toString ();
			else if (var.canConvert<QByteArray> ())
				return var.toByteArray ();
			else if (var.canConvert<int> ())
				return QString::number (var.toInt ());
			else if (var.canConvert<double> ())
				return QString::number (var.toDouble ());
			else if (var.canConvert<QVariantList> ())
			{
				QStringList subvars;
				for (const auto& sv : var.toStringList ())
					subvars << Unserialize (sv);
				return "ByteArray { " + subvars.join ("; ") + " }";
			}
			else if (var.canConvert<QVariantMap> ())
			{
				QStringList subvars;
				const auto& map = var.toMap ();
				for (const auto& key : map.keys ())
					subvars << "{ " + key + " -> " + Unserialize (map [key]) + "}";
				return "Map { " + subvars.join ("; ") + " }";
			}
			else
				return QString ("unsupported datatype %1 (%2) (%3)")
						.arg (var.typeName ())
						.arg (var.userType ())
						.arg (var.toString ());
		}
	}

	ContentsDisplayDialog::ContentsDisplayDialog (QWidget *parent)
	: QDialog (parent)
	, ContentsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ContentsTree_->setModel (ContentsModel_);

		auto makePair = [] (const QString& leftText, const QString& rightText) -> QList<QStandardItem*>
		{
			QList<QStandardItem*> result;
			result << new QStandardItem (leftText)
				<< new QStandardItem (rightText);
			for (auto item : result)
				item->setEditable (false);
			return result;
		};

		for (auto pluginObj : Core::Instance ().GetStoragePlugins ())
		{
			auto ii = qobject_cast<IInfo*> (pluginObj);
			auto isp = qobject_cast<IStoragePlugin*> (pluginObj);

			auto pluginItems = makePair (ii->GetName (), ii->GetInfo ());

			for (const auto& key : isp->ListKeys ())
			{
				const auto& valList = isp->Load (key);
				const auto& keyPair = makePair (QString::fromUtf8 (key), QString ());
				pluginItems.first ()->appendRow (keyPair);
				int valIndex = 0;
				for (const auto& valVar : valList)
				{
					const auto& str = Unserialize (valVar);
					keyPair.first ()->appendRow (makePair (QString::number (++valIndex), str));
				}
			}

			ContentsModel_->appendRow (pluginItems);
		}

		Ui_.ContentsTree_->expandAll ();
	}
}
}
}
