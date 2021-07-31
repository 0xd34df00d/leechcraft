/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#pragma once

#include <variant>
#include <QStateMachine>
#include <util/sll/void.h>
#include "interfaces/structures.h"
#include "kinotify.h"
#include "ui_kinotifywidget.h"

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Kinotify
{
	using ImageVar_t = std::variant<Util::Void, QPixmap>;

	class KinotifyWidget : public QWidget
	{
		Q_OBJECT
		Q_PROPERTY (qreal opacity READ windowOpacity WRITE SetOpacity)

		Ui::KinotifyWidget Ui_;

		QString ID_;

		QString Title_;
		QString Body_;
		QStringList ActionsNames_;

		int Timeout_;
		QStateMachine Machine_;
		ImageVar_t OverridePixmap_;
		QObject_ptr ActionHandler_;
	public:
		explicit KinotifyWidget (int timeout, QWidget *widget = nullptr);

		QString GetTitle () const;
		QString GetBody () const;

		QString GetID () const;
		void SetID (const QString&);

		void SetContent (const QString&, const QString&);
		void OverrideImage (const ImageVar_t&);
		void PrepareNotification ();
		void SetActions (const QStringList&, QObject_ptr);
	protected:
		void showEvent (QShowEvent*) override;
	private:
		void SetData ();
		void SetWidgetPlace ();

		void SetOpacity (qreal);
	signals:
		void initiateCloseNotification ();
	};
}
}
