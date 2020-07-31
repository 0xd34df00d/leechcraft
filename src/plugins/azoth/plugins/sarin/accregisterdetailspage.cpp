/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accregisterdetailspage.h"
#include <random>
#include <QtDebug>
#include <sodium.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	AccRegisterDetailsPage::AccRegisterDetailsPage (QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);
	}

	QString AccRegisterDetailsPage::GetId () const
	{
		return Ui_.IdEdit_->text ();
	}

	QString AccRegisterDetailsPage::GetNickname () const
	{
		return Ui_.NickEdit_->text ();
	}

	void AccRegisterDetailsPage::on_GenerateButton__released ()
	{
		unsigned char pk [crypto_box_PUBLICKEYBYTES];
		unsigned char sk [crypto_box_SECRETKEYBYTES];
		crypto_box_keypair (pk, sk);

		std::random_device dev;
		std::uniform_int_distribution<uint32_t> distr { 0, std::numeric_limits<uint32_t>::max () };
		const auto antispam = distr (dev);

		QString idStr;

		uint16_t checksum = 0;
		for (uint i = 0; i < crypto_box_PUBLICKEYBYTES; i += 2)
		{
			const uint16_t halfWord = (pk [i] << 8) + pk [i + 1];
			checksum ^= halfWord;
			idStr += QString { "%1" }.arg (static_cast<uint> (halfWord), 4, 16, QChar { '0' });
		}

		checksum ^= (antispam & 0xffff0000) >> 16;
		checksum ^= (antispam & 0xffff);
		idStr += QString { "%1" }.arg (antispam, 8, 16, QChar { '0' });
		idStr += QString { "%1" }.arg (checksum, 4, 16, QChar { '0' });

		Ui_.IdEdit_->setText (idStr.toUpper ());
	}
}
}
}
