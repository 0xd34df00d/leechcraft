/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include "fancytrayiconimpl.h"

namespace LC::Util
{
	class FancyTrayIconFreedesktop;
}

namespace LC::Util::detail
{
	struct IconFrame
	{
		int Width_;
		int Height_;
		QByteArray Data_;

		static IconFrame FromPixmap (const QPixmap&);
	};

	struct DBusTooltip
	{
		QString Title_;
		QString Subtitle_;
	};
}

Q_DECLARE_METATYPE (LC::Util::detail::IconFrame)
Q_DECLARE_METATYPE (QList<LC::Util::detail::IconFrame>)
Q_DECLARE_METATYPE (LC::Util::detail::DBusTooltip)

namespace LC::Util::detail
{
	class SNIAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		// KDE seems to expect `org.kde.StatusNotifierItem` contrary to the spec
		Q_CLASSINFO ("D-Bus Interface", "org.kde.StatusNotifierItem")

		Q_PROPERTY (QString Category MEMBER Category_ CONSTANT)
		Q_PROPERTY (QString Status MEMBER Status_ CONSTANT)
		Q_PROPERTY (quint32 WindowId MEMBER WindowId_ CONSTANT)

		Q_PROPERTY (QString Id READ GetId CONSTANT)
		Q_PROPERTY (QString Title READ GetTitle)

		Q_PROPERTY (QString IconName READ GetIconName)
		Q_PROPERTY (QList<LC::Util::detail::IconFrame> IconPixmap READ GetIconPixmap NOTIFY NewIcon)

		Q_PROPERTY (LC::Util::detail::DBusTooltip ToolTip READ GetTooltip NOTIFY NewTooltip)

		FancyTrayIconFreedesktop& Impl_;

		const QString Category_ = QStringLiteral ("ApplicationStatus");
		const QString Status_ = QStringLiteral ("Active");
		const quint32 WindowId_ = 0;
	public:
		explicit SNIAdaptor (FancyTrayIconFreedesktop&);
	public slots:
		void ContextMenu (int x, int y);
		void Activate (int, int);
		void SecondaryActivate (int, int);
	private:
		QString GetId () const;
		QString GetTitle () const;
		QString GetIconName () const;
		QList<IconFrame> GetIconPixmap () const;
		DBusTooltip GetTooltip () const;
	signals:
		void NewIcon ();
		void NewTooltip ();
	};
}

namespace LC::Util
{
	class FancyTrayIconFreedesktop : public FancyTrayIconImpl
	{
		friend class detail::SNIAdaptor;

		FancyTrayIcon& FTI_;

		detail::SNIAdaptor Adaptor_;
	public:
		explicit FancyTrayIconFreedesktop (FancyTrayIcon& icon);

		void UpdateIcon () override;
		void UpdateTooltip () override;
		void UpdateMenu () override;
	};
}
