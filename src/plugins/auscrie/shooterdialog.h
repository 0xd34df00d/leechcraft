/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AUSCRIE_SHOOTERDIALOG_H
#define PLUGINS_AUSCRIE_SHOOTERDIALOG_H
#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_shooterdialog.h"

namespace LC
{
namespace Auscrie
{
	class ShooterDialog : public QDialog
	{
		Q_OBJECT

		Ui::ShooterDialog Ui_;
		const ICoreProxy_ptr Proxy_;
		QPixmap CurrentScreenshot_;
	public:
		struct FilterData
		{
			QObject *Object_;
			QByteArray Variant_;
		};
	private:
		QList<FilterData> Filters_;
	public:
		enum class Action
		{
			Upload,
			Save
		};

		enum class Mode
		{
			LCWindowOverlay,
			LCWindow,
			CurrentScreen,
			WholeDesktop
		};

		ShooterDialog (ICoreProxy_ptr, QWidget* = 0);

		Action GetAction () const;

		Mode GetMode () const;
		bool ShouldHide () const;

		int GetTimeout () const;
		QString GetFormat () const;
		int GetQuality () const;

		FilterData GetDFInfo () const;

		void SetScreenshot (const QPixmap&);
		QPixmap GetScreenshot () const;

		void resizeEvent (QResizeEvent*);
	private:
		void RescaleLabel ();
	private slots:
		void on_Format__currentIndexChanged (const QString&);
	signals:
		void screenshotRequested ();
	};
}
}

#endif
