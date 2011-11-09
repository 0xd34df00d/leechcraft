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
#include "message.h"
#include "progresslistener.h"

class QMutex;

namespace LeechCraft
{
namespace Snails
{
	class AccountThread;
	class AccountThreadWorker;

	class Account : public QObject
	{
		Q_OBJECT

		friend class AccountThreadWorker;
		AccountThread *Thread_;
		QMutex *AccMutex_;

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

		void FetchNewHeaders (int);
		void FetchWholeMessage (const QByteArray&);

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);

		void OpenConfigDialog ();

		bool IsNull () const;

		QString GetInUsername ();
		QString GetOutUsername ();
	private:
		QMutex* GetMutex () const;

		QString BuildInURL ();
		QString BuildOutURL ();
		QString GetPassImpl ();
	private slots:
		void buildInURL (QString*);
		void buildOutURL (QString*);
		void getPassword (QString*);
		void handleMsgHeaders (QList<Message_ptr>);
	signals:
		void mailChanged ();
		void gotNewMessages (QList<Message_ptr>);
		void gotProgressListener (ProgressListener_g_ptr);
		void accountChanged ();
	};

	typedef std::shared_ptr<Account> Account_ptr;
}
}

#endif
