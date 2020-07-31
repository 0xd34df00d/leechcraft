/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_radiotracksgrabdialog.h"

class QStandardItemModel;

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	class RadioTracksGrabDialog : public QDialog
	{
		Q_OBJECT

		Ui::RadioTracksGrabDialog Ui_;

		QStandardItemModel * const NamesPreviewModel_;
		QStringList Names_;
	public:
		RadioTracksGrabDialog (const QList<Media::AudioInfo>&, QWidget* = nullptr);
		RadioTracksGrabDialog (const QList<MediaInfo>&, QWidget* = nullptr);

		const QStringList& GetNames () const;
		QString GetDestination () const;

		static QString SelectDestination (QString dir = {}, QWidget *parent = nullptr);
	private:
		bool IsComplete () const;
	private slots:
		void on_Browse__released ();
		void checkCompleteness ();
	};
}
}
