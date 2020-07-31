/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_AFFILIATIONSELECTORDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_AFFILIATIONSELECTORDIALOG_H
#include <QDialog>
#include "ui_affiliationselectordialog.h"
#include <QXmppMucIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class AffiliationSelectorDialog : public QDialog
	{
		Q_OBJECT

		Ui::AffiliationSelectorDialog Ui_;
	public:
		AffiliationSelectorDialog (QWidget* = 0);

		QString GetJID () const;
		void SetJID (const QString&);

		QXmppMucItem::Affiliation GetAffiliation () const;
		void SetAffiliation (QXmppMucItem::Affiliation);

		QString GetReason () const;
		void SetReason (const QString&);
	};
}
}
}

#endif
