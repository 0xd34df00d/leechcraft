/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_ACCOUNT_H
#define PLUGINS_SNAILS_ACCOUNT_H
#include <memory>
#include <QObject>
#include <vmime/net/session.hpp>

namespace LeechCraft
{
namespace Snails
{
	class Account : public QObject
	{
		Q_OBJECT

		vmime::ref<vmime::net::session> Session_;

		QByteArray ID_;
		QString AccName_;

		QString Login_;
		bool UseSASL_;
		bool SASLRequired_;
		bool UseTLS_;
		bool TLSRequired_;

		bool SMTPNeedsAuth_;
		bool APOP_;
		bool APOPFail_;

		QString InHost_;
		int InPort_;

		QString OutHost_;
		int OutPort_;

		QString OutLogin_;

	public:
		enum Direction
		{
			DIn,
			DOut
		};

		enum InType
		{
			ITIMAP,
			ITPOP3,
			ITMaildir
		};

		enum OutType
		{
			OTSMTP,
			OTSendmail
		};
	private:
		InType InType_;
		OutType OutType_;
	public:
		Account (QObject* = 0);

		QByteArray GetID () const;
		QString GetName () const;
		QString GetServer () const;
		QString GetType () const;

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);

		void OpenConfigDialog ();

		bool IsNull () const;

		QString GetInUsername ();
		QString GetOutUsername ();
	private:
		void RebuildSessConfig ();
		vmime::ref<vmime::net::store> MakeStore ();
		vmime::ref<vmime::net::transport> MakeTransport ();
		QString BuildInURL () const;
		QString BuildOutURL () const;
	};

	typedef std::shared_ptr<Account> Account_ptr;
}
}

#endif
