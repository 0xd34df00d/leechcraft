/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "passwordremember.h"
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ipersistentstorageplugin.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	PasswordRemember::PasswordRemember (QWidget *parent)
	: Util::PageNotification (parent)
	{
		Ui_.setupUi (this);
	}

	void PasswordRemember::add (const PageFormsData_t& data)
	{
		TempData_ = data;

		const auto sb = Core::Instance ().GetStorageBackend ();
		for (auto& list : TempData_)
			list.erase (std::remove_if (list.begin (), list.end (),
						[sb] (const auto& ed) { return sb->GetFormsIgnored (ed.PageURL_.toString ()); }),
					list.end ());

		for (auto i = TempData_.begin (); i != TempData_.end (); )
			if (i.value ().isEmpty ())
				i = TempData_.erase (i);
			else
				++i;

		if (!TempData_.isEmpty ())
			show ();
	}

	void PasswordRemember::on_Remember__released ()
	{
		auto hideGuard = Util::MakeScopeGuard ([this]
				{
					TempData_.clear ();
					hide ();
				});

		if (TempData_.isEmpty ())
			return;

		const auto storagePlugin = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<IPersistentStoragePlugin*> ().value (0);
		if (!storagePlugin)
			return;

		const auto& storage = storagePlugin->RequestStorage ();
		if (!storage)
			return;

		for (const auto& pair : Util::Stlize (TempData_))
			storage->Set ("org.LeechCraft.Poshuku.Forms.InputByName/" + pair.first.toUtf8 (),
					Util::Map (pair.second, [] (const ElementData& ed) { return QVariant::fromValue (ed); }));
	}

	void PasswordRemember::on_NotNow__released ()
	{
		TempData_.clear ();
		hide ();
	}

	void PasswordRemember::on_Never__released ()
	{
		const auto sb = Core::Instance ().GetStorageBackend ();
		for (const auto& pair : Util::Stlize (TempData_))
			for (const auto& ed : pair.second)
				sb->SetFormsIgnored (ed.PageURL_.toString (), true);

		TempData_.clear ();
		hide ();
	}
}
}
