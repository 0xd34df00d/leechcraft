/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_ADDTOMETACONTACTSDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_ADDTOMETACONTACTSDIALOG_H
#include <QDialog>
#include "ui_addtometacontactsdialog.h"

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace Metacontacts
{
	class MetaEntry;

	class AddToMetacontactsDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::AddToMetacontactsDialog Ui_;
	public:
		AddToMetacontactsDialog (ICLEntry*,
				const QList<MetaEntry*>&, QWidget* = 0);
		
		MetaEntry* GetSelectedMeta () const;
		QString GetNewMetaName () const;
	private slots:
		void on_ExistingMeta__currentIndexChanged (int);
	};
}
}
}

#endif
