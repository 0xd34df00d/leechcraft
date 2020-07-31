/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <stdexcept>
#include <type_traits>
#include <memory>
#include <vector>
#include <QIcon>
#include <QString>
#include <QFile>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include <interfaces/azoth/iprotocol.h>

extern "C"
{
#include <libotr/privkey.h>
}

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	QIcon GetAccountIcon (IAccount *account)
	{
		const auto accObj = account->GetQObject ();

		const auto extSelf = qobject_cast<IExtSelfInfoAccount*> (accObj);
		auto icon = extSelf ? extSelf->GetAccountIcon () : QIcon {};

		if (icon.isNull ())
			icon = qobject_cast<IProtocol*> (account->GetParentProtocol ())->GetProtocolIcon ();

		return icon;
	}

	namespace
	{
		void SexpWrite (QFile& file, gcry_sexp_t sexp)
		{
			const auto buflen = gcry_sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, nullptr, 0);
			QByteArray buf (buflen, 0);
			gcry_sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, buf.data (), buflen);
			file.write (buf);
		}

		void WriteAcc (QFile& file, const char *accName, const char *proto, gcry_sexp_t privkey)
		{
			file.write (" (account\n");

			gcry_sexp_t names;
			if (gcry_sexp_build (&names, nullptr, "(name %s)", accName))
				throw std::runtime_error ("cannot save keys");
			std::shared_ptr<std::remove_pointer<gcry_sexp_t>::type> namesGuard { names, gcry_sexp_release };
			SexpWrite (file, names);

			gcry_sexp_t protos;
			if (gcry_sexp_build (&protos, nullptr, "(protocol %s)", proto))
				throw std::runtime_error ("cannot save keys");
			std::shared_ptr<std::remove_pointer<gcry_sexp_t>::type> protosGuard { protos, gcry_sexp_release };
			SexpWrite (file, protos);

			SexpWrite (file, privkey);

			file.write (" )\n");
		}
	}

	void WriteKeys (OtrlUserState state, const QString& filename)
	{
		const auto& tmpFilename = filename + ".new";
		QFile file { tmpFilename };
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
			throw std::runtime_error ("cannot open keys file");

		file.write ("(privkeys\n");

		for (auto pkey = state->privkey_root; pkey; pkey = pkey->next)
			WriteAcc (file, pkey->accountname, pkey->protocol, pkey->privkey);

		file.write (")\n");
		file.flush ();

		std::shared_ptr<std::remove_pointer<OtrlUserState>::type> testState
		{
			otrl_userstate_create (),
			&otrl_userstate_free
		};
		if (otrl_privkey_read (testState.get (), tmpFilename.toUtf8 ().constData ()))
			throw std::runtime_error ("failed to save the keys");

		QFile::remove (filename);
		file.rename (filename);
	}
}
}
}
