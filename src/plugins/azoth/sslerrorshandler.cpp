/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorshandler.h"
#include <QSslError>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/visitor.h>
#include <util/sll/prelude.h>
#include "components/dialogs/sslerrorsdialog.h"
#include "sslerrorschoicestorage.h"

namespace LC
{
namespace Azoth
{
	SslErrorsHandler::SslErrorsHandler (const Context_t& context, ICanHaveSslErrors *ichse)
	: QObject { ichse->GetQObject () }
	, Context_ { context }
	, ICHSE_ { ichse }
	{
		connect (ichse->GetQObject (),
				SIGNAL (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)),
				this,
				SLOT (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)));
	}

	namespace
	{
		bool CheckSavedChoice (SslErrorsChoiceStorage& storage,
				const SslErrorsHandler::Context_t& context,
				const QList<QSslError>& errors,
				const ICanHaveSslErrors::ISslErrorsReaction_ptr& reaction)
		{
			return Util::Visit (context,
					[] (SslErrorsHandler::AccountRegistration) { return false; },
					[&] (const SslErrorsHandler::Account& acc)
					{
						const auto choices = Util::Map (errors,
								[&] (QSslError err) { return storage.GetAction (acc.ID_, err.error ()); });

						if (std::any_of (choices.begin (), choices.end (),
								[] (const auto& choice) { return choice == SslErrorsChoiceStorage::Action::Abort; }))
						{
							reaction->Abort ();
							return true;
						}

						if (std::all_of (choices.begin (), choices.end (),
								[] (const auto& choice) { return choice == SslErrorsChoiceStorage::Action::Ignore; }))
						{
							reaction->Ignore ();
							return true;
						}

						return false;
					});
		}
	}

	void SslErrorsHandler::sslErrors (const QList<QSslError>& errors,
			const ICanHaveSslErrors::ISslErrorsReaction_ptr& reaction)
	{
		qDebug () << Q_FUNC_INFO;
		for (const auto& error : errors)
			qDebug () << error.errorString ();

		auto storage = std::make_shared<SslErrorsChoiceStorage> ();
		if (CheckSavedChoice (*storage, Context_, errors, reaction))
			return;

		auto rememberChoice = [storage, errors, ctx = Context_] (SslErrorsChoiceStorage::Action action)
		{
			Util::Visit (ctx,
					[] (SslErrorsHandler::AccountRegistration) {},
					[&] (const SslErrorsHandler::Account& acc)
					{
						for (const auto& error : errors)
							storage->SetAction (acc.ID_, error.error (), action);
					});
		};

		auto dia = new SslErrorsDialog { Context_, errors };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[dia, reaction, rememberChoice]
			{
				reaction->Ignore ();
				if (dia->ShouldRememberChoice ())
					rememberChoice (SslErrorsChoiceStorage::Action::Ignore);
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[dia, reaction, rememberChoice]
			{
				reaction->Abort ();
				if (dia->ShouldRememberChoice ())
					rememberChoice (SslErrorsChoiceStorage::Action::Abort);
			},
			dia,
			SIGNAL (rejected ()),
			dia
		};
		dia->exec ();
	}
}
}
