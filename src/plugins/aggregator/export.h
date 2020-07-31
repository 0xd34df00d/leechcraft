/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QSet>
#include "ui_export.h"
#include "channel.h"

namespace LC
{
namespace Aggregator
{
	class Export : public QDialog
	{
		Q_OBJECT

		Ui::Export Ui_;
		QString Title_;
		QString Choices_;
	public:
		Export (const QString&, const QString&, const QString&, QWidget* = 0);

		QString GetDestination () const;
		QString GetTitle () const;
		QString GetOwner () const;
		QString GetOwnerEmail () const;
		QSet<IDType_t> GetSelectedFeeds () const;

		void SetFeeds (const channels_shorts_t&);
	private slots:
		void on_File__textEdited (const QString&);
		void on_Browse__released ();
	};
}
}
