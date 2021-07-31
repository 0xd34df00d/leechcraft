/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#pragma once

#include <QStateMachine>
#include "ui_kinotifywidget.h"

using QObject_ptr = std::shared_ptr<QObject>;

namespace LC::Kinotify
{
	class KinotifyWidget : public QWidget
	{
		Q_OBJECT
		Q_PROPERTY (qreal opacity READ windowOpacity WRITE SetOpacity)

		Ui::KinotifyWidget Ui_;

		QString ID_;

		QString Title_;
		QString Body_;
		QPixmap Pixmap_;
		QStringList ActionsNames_;

		int Timeout_;
		QStateMachine Machine_;
		QObject_ptr ActionHandler_;
	public:
		explicit KinotifyWidget (int timeout, QWidget *widget = nullptr);

		QString GetTitle () const;
		QString GetBody () const;

		QString GetID () const;
		void SetID (const QString&);

		void SetContent (const QString&, const QString&);
		void SetPixmap (QPixmap);
		void SetActions (const QStringList&, QObject_ptr);

		void PrepareNotification ();
	protected:
		void showEvent (QShowEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
	private:
		void SetData ();
		void SetWidgetPlace ();

		void SetOpacity (qreal);
	signals:
		void initiateCloseNotification ();
	};
}
