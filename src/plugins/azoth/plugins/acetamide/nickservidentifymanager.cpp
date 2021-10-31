/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nickservidentifymanager.h"
#include <QtDebug>
#include <util/models/flatitemsmodel.h>
#include "localtypes.h"
#include "nickservidentifywidget.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	NickServIdentifyManager *NickServIdentifyManager::Instance_ = nullptr;

	NickServIdentifyManager::NickServIdentifyManager ()
	: Model_ { std::make_unique<IdentifyModel> (QStringList
		{
			tr ("Server"),
			tr ("Nickname"),
			tr ("NickServ nickname"),
			tr ("NickServ auth string"),
			tr ("Auth message")
		}) }
	, ConfigWidget_ { std::make_unique<NickServIdentifyWidget> (*Model_) }
	{
		ReadSettings ();

		QObject::connect (ConfigWidget_.get (),
				&NickServIdentifyWidget::saveSettings,
				[this] { WriteSettings(); });

		QObject::connect (ConfigWidget_.get (),
				&NickServIdentifyWidget::identifyAdded,
				Model_.get (),
				&IdentifyModel::AddItem);
		QObject::connect (ConfigWidget_.get (),
				&NickServIdentifyWidget::identifyEdited,
				Model_.get (),
				&IdentifyModel::EditItem);
		QObject::connect (ConfigWidget_.get (),
				&NickServIdentifyWidget::identifyRemoved,
				Model_.get (),
				&IdentifyModel::RemoveItem);

		Instance_ = this;
	}

	NickServIdentifyManager::~NickServIdentifyManager () = default;

	QWidget *NickServIdentifyManager::GetConfigWidget () const
	{
		return ConfigWidget_.get ();
	}

	QList<NickServIdentify> NickServIdentifyManager::GetIdentifies (const QString& server,
			const QString& nick,
			const QString& nickserv) const
	{
		QList<NickServIdentify> list;
		for (const auto& nsi : Model_->GetItems ())
			if (nsi.Server_ == server &&
					nsi.Nick_ == nick &&
					QRegExp { nsi.NickServNick_, Qt::CaseInsensitive, QRegExp::Wildcard }.indexIn (nickserv) == 0)
				list << nsi;
		return list;
	}

	void NickServIdentifyManager::ReadSettings ()
	{
		auto list = XmlSettingsManager::Instance ().property ("NickServIdentify").value<QList<QStringList>> ();

		QVector<NickServIdentify> idents;
		idents.reserve (list.size ());

		for (auto& subList : list)
		{
			if (subList.size () != 5)
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping unknown"
						<< subList;
				continue;
			}

			idents.push_back ({
					.Server_ = std::move (subList [0]),
					.Nick_ = std::move (subList [1]),
					.NickServNick_ = std::move (subList [2]),
					.AuthString_ = std::move (subList [3]),
					.AuthMessage_ = std::move (subList [4]),
				});
		}

		Model_->SetItems (std::move (idents));
	}

	void NickServIdentifyManager::WriteSettings ()
	{
		const auto& items = Model_->GetItems ();
		QList<QStringList> list;
		list.reserve (items.size ());
		for (const auto& item : items)
			list.push_back ({
					item.Server_,
					item.Nick_,
					item.NickServNick_,
					item.AuthString_,
					item.AuthMessage_,
				});

		XmlSettingsManager::Instance ().setProperty ("NickServIdentify", QVariant::fromValue (list));
	}
}
