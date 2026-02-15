/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <interfaces/ijobholder.h>

namespace LC::Util
{
	class ProgressManager;
}

namespace LC::LMP::BrainSlugz
{
	class Checker;

	class ProgressModelManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::BrainSlugz::ProgressModelManager)

		Util::ProgressManager * const Progress_;
	public:
		explicit ProgressModelManager (QObject* = nullptr);

		IJobHolderRepresentationHandler_ptr CreateReprHandler ();

		void AddChecker (Checker*);
	};
}
