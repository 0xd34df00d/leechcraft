/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vmime/security/cert/certificateVerifier.hpp>

namespace LC
{
namespace Snails
{
	class CertificateVerifier : public vmime::security::cert::certificateVerifier
	{
	public:
		void verify (const vmime::shared_ptr<vmime::security::cert::certificateChain>&,
				const vmime::string&) override;
	};
}
}
