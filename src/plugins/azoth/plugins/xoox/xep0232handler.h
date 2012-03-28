/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once
#include <QUrl>

class QXmppDataForm;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
namespace XEP0232Handler
{
	struct SoftwareInformation
	{
		int IconWidth_;
		int IconHeight_;
		QUrl IconURL_;
		QString IconCID_;
		QString IconType_;

		QString OS_;
		QString OSVer_;
		QString Software_;
		QString SoftwareVer_;

		bool IsNull () const;
	};

	SoftwareInformation FromDataForm (const QXmppDataForm&);
	QXmppDataForm ToDataForm (const SoftwareInformation&);
}
}
}
}
