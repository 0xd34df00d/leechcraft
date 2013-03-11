/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QString>
#include <QUrl>
#include <QVariant>

class QDeclarativeImageProvider;

namespace LeechCraft
{
	struct QuarkComponent
	{
		QUrl Url_;
		QList<QPair<QString, QObject*>> DynamicProps_;
		QList<QPair<QString, QVariant>> StaticProps_;
		QList<QPair<QString, QDeclarativeImageProvider*>> ImageProviders_;
	};

	typedef QList<QuarkComponent> QuarkComponents_t;
}

class Q_DECL_EXPORT IQuarkComponentProvider
{
public:
	virtual ~IQuarkComponentProvider () {}

	virtual LeechCraft::QuarkComponents_t GetComponents () const = 0;
};

Q_DECLARE_INTERFACE (IQuarkComponentProvider, "org.Deviant.LeechCraft.IQuarkComponentProvider/1.0");
