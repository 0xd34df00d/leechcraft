/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <variant>
#include <QDialog>
#include "ui_mucinvitedialog.h"

namespace LC::Azoth
{
	class ICLEntry;

	class MUCInviteDialogBase : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::MUCInviteDialogBase)
	protected:
		Ui::MUCInviteDialog Ui_;
	public:
		explicit MUCInviteDialogBase (const ICLEntry& source, QWidget* = nullptr);

		QString GetInviteMessage () const;
	};

	enum class AddressInvitations : bool
	{
		Disallowed,
		Allowed
	};

	template<AddressInvitations AI>
	class MUCInviteDialog : public MUCInviteDialogBase
	{
	public:
		explicit MUCInviteDialog (const ICLEntry& source, QWidget* = nullptr);

		using TargetEntry_t = std::conditional_t<
				AI == AddressInvitations::Allowed,
				std::variant<ICLEntry*, QString>,
				ICLEntry*
			>;

		TargetEntry_t GetTargetEntry () const;
	};

	extern template class MUCInviteDialog<AddressInvitations::Disallowed>;
	extern template class MUCInviteDialog<AddressInvitations::Allowed>;
}
