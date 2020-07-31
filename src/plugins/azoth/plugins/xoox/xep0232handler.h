/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSize>
#include <QUrl>
#include <QMimeType>

class QXmppDataForm;

namespace LC::Azoth::Xoox::XEP0232Handler
{
	struct SoftwareInformation
	{
		QSize IconSize_;
		QUrl IconURL_;
		QByteArray IconCID_;
		QMimeType IconType_;

		QString OS_;
		QString OSVer_;
		QString Software_;
		QString SoftwareVer_;

		bool IsNull () const;
	};

	SoftwareInformation FromDataForm (const QXmppDataForm&);
	QXmppDataForm ToDataForm (const SoftwareInformation&);
}
