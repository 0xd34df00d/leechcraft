/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <functional>
#include <QObject>
#include <QQueue>
#include <QPair>
#include <QDomElement>
#include <QNetworkRequest>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJXmlRPC : public QObject
	{
		Q_OBJECT

		QQueue<std::function<void (const QString&)>> ApiCallQueue_;
	public:
		LJXmlRPC (QObject *parent = 0);

		void Validate (const QString& login, const QString& pass);
	private:
		void GenerateChallenge () const;
		void ValidateAccountData (const QString& login,
				const QString& pass, const QString& challenge);

		QPair<QDomElement, QDomElement> GetStartPart (const QString& name, QDomDocument doc);
		QDomElement GetMemberElement (const QString& name,
				const QString& type, const QString& value, QDomDocument doc);
		QNetworkRequest CreateNetworkRequest () const;

	private slots:
		void handleChallengeReplyFinished ();
		void handleValidateReplyFinished ();

	signals:
		void validatingFinished (bool success);
	};
}
}
}
