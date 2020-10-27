/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QUrl>
#include <xmlsettingsdialog/basesettingsmanager.h>

class QQmlContext;

namespace LC::SB2
{
	class QuarkSettingsManager : public Util::BaseSettingsManager
	{
		Q_OBJECT

		const QUrl QuarkURL_;
		QQmlContext * const Ctx_;
	public:
		QuarkSettingsManager (QUrl, QQmlContext*);
	public slots:
		void setSettingsValue (const QString& key, const QVariant& value);
	protected:
		QSettings* BeginSettings () const override;
		void EndSettings (QSettings*) const override;
		void PropertyChanged (const QString&, const QVariant&) override;
	};
}
