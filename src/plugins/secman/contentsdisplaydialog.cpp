/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contentsdisplaydialog.h"
#include <QStandardItemModel>
#include <util/sll/qtutil.h>
#include <util/xpc/introspectable.h>
#include <interfaces/iinfo.h>
#include "interfaces/secman/istorageplugin.h"
#include "core.h"

namespace LC
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
				for (const auto& pair : Util::Stlize (var.toMap ()))
					subvars << "{ " + pair.first + " -> " + Unserialize (pair.second) + "}";
				return "Map { " + subvars.join ("; ") + " }";
			}
			else
			{
				try
				{
					return Unserialize (Util::Introspectable::Instance () (var));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
				}

				qWarning () << Q_FUNC_INFO
						<< "unsupported data type"
						<< var
						<< var.typeName ()
						<< var.userType ()
						<< var.toString ();
				return QString ("unsupported datatype %1 (%2) (%3), loading the matching plugin may help")
						.arg (var.typeName ())
						.arg (var.userType ())
						.arg (var.toString ());
			}
		}
	}

	ContentsDisplayDialog::ContentsDisplayDialog (QWidget *parent)
	: QDialog { parent }
	, ContentsModel_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);

		ContentsModel_->setHorizontalHeaderLabels ({ tr ("Key index"), tr ("Value") });
		Ui_.ContentsTree_->setModel (ContentsModel_);

		auto makePair = [] (const QString& leftText, const QString& rightText)
		{
			const QList<QStandardItem*> result
			{
				new QStandardItem { leftText },
				new QStandardItem { rightText }
			};
			for (const auto item : result)
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
				const auto& keyPair = makePair (QString::fromUtf8 (key), {});
				pluginItems.first ()->appendRow (keyPair);
				int valIndex = 0;
				for (const auto& valVar : valList.toList ())
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
