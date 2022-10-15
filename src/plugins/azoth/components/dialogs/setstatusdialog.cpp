/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "setstatusdialog.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "customstatusesmanager.h"
#include "resourcesmanager.h"
#include "util.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		QByteArray BuildSettingName (const QString& context, State st)
		{
			return QString ("FastStatusText_%1_%2")
					.arg (context)
					.arg (st)
					.toUtf8 ();
		}

		State GetStateForIndex (int index)
		{
			switch (index)
			{
				case 1:
					return SChat;
				case 2:
					return SAway;
				case 3:
					return SDND;
				case 4:
					return SXA;
				case 5:
					return SOffline;
				default:
					return SOnline;
			}
		}
	}

	SetStatusDialog::SetStatusDialog (const QString& context, QWidget *parent)
	: QDialog (parent)
	, Context_ (context)
	{
		Ui_.setupUi (this);
		on_StatusBox__currentIndexChanged ();

		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (save ()));

		for (int i = 0; i < Ui_.StatusBox_->count (); ++i)
		{
			const auto state = GetStateForIndex (i);
			Ui_.StatusBox_->setItemIcon (i, ResourcesManager::Instance ().GetIconForState (state));
			Ui_.StatusBox_->setItemData (i, QVariant::fromValue (state), Roles::ItemState);

			const auto& name = BuildSettingName (Context_, state);
			const auto& str = XmlSettingsManager::Instance ().property (name).toString ();
			Ui_.StatusBox_->setItemData (i, str, Roles::StateText);

			if (Ui_.StatusBox_->currentIndex () == i)
				Ui_.StatusText_->setText (str);
		}

		const auto& customs = Core::Instance ().GetCustomStatusesManager ()->GetStates ();
		for (const auto& custom : customs)
		{
			const auto state = custom.State_;
			const auto& name = custom.Name_ + " (" + StateToString (state) + ")";
			Ui_.StatusBox_->addItem (ResourcesManager::Instance ().GetIconForState (state), name);

			const auto pos = Ui_.StatusBox_->count () - 1;
			Ui_.StatusBox_->setItemData (pos,
					QVariant::fromValue (state), Roles::ItemState);
			Ui_.StatusBox_->setItemData (pos, custom.Text_, Roles::StateText);
		}
	}

	State SetStatusDialog::GetState () const
	{
		const auto idx = Ui_.StatusBox_->currentIndex ();
		return Ui_.StatusBox_->itemData (idx, Roles::ItemState).value<State> ();
	}

	QString SetStatusDialog::GetStatusText () const
	{
		return Ui_.StatusText_->toPlainText ();
	}

	void SetStatusDialog::save ()
	{
		const auto& name = BuildSettingName (Context_, GetState ());
		XmlSettingsManager::Instance ().setProperty (name, GetStatusText ());
	}

	void SetStatusDialog::on_StatusBox__currentIndexChanged ()
	{
		const auto idx = Ui_.StatusBox_->currentIndex ();
		Ui_.StatusText_->setText (Ui_.StatusBox_->itemData (idx, Roles::StateText).toString ());
	}
}
}
