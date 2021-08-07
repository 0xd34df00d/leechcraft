/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_tracksharedialog.h"

namespace LC::Azoth::Xtazy
{
	class TrackShareDialog : public QDialog
	{
		Ui::TrackShareDialog Ui_;
	public:
		TrackShareDialog (const QString&, const QStringList&, QObject*, QWidget* = nullptr);

		QString GetVariantName () const;
	};
}
