/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include "entitygeneratingpage.h"
#include "ui_feedssettingsimportpage.h"

namespace LC
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class KTorrentImportPage : public EntityGeneratingPage
	{
		Q_OBJECT

		Ui::FeedsSettingsImportPage Ui_;
	public:
		KTorrentImportPage (const ICoreProxy_ptr&, QWidget* = 0);

		bool CheckValidity (const QString&) const;
		bool isComplete () const override;
		int nextId () const override;
		void initializePage () override;
	private:
		bool GetTorrentSettings (const QString&, QMap<QString, QVariant>&) const;
	private slots:
		void on_Browse__released ();
		void handleAccepted ();
	};
}
}
}
