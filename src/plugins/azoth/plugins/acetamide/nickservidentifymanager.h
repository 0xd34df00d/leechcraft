/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QCoreApplication>

namespace LC::Util
{
	template<typename>
	class FlatItemsModel;
}

namespace LC::Azoth::Acetamide
{
	struct NickServIdentify;
	class NickServIdentifyWidget;

	class NickServIdentifyManager final
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Acetamide::NickServIdentifyManager)

		using IdentifyModel = Util::FlatItemsModel<NickServIdentify>;

		const std::unique_ptr<IdentifyModel> Model_;
		const std::unique_ptr<NickServIdentifyWidget> ConfigWidget_;
	public:
		explicit NickServIdentifyManager ();
		~NickServIdentifyManager ();

		QWidget* GetConfigWidget () const;

		QList<NickServIdentify> GetIdentifies (const QString& server,
				const QString& nick,
				const QString& nickserv) const;
	private:
		void ReadSettings ();
		void WriteSettings ();
	};
}
