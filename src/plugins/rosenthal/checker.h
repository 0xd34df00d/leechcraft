/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <QObject>
#include <QSet>
#include <interfaces/ispellcheckprovider.h>
#include "hunspell/hunspell.hxx"

namespace LC::Util
{
	class EncodingConverter;
}

namespace LC
{
namespace Rosenthal
{
	class KnownDictsManager;

	class Checker : public QObject
				  , public ISpellChecker
	{
		Q_OBJECT
		Q_INTERFACES (ISpellChecker)

		struct HunspellItem;
		std::vector<HunspellItem> Hunspells_;

		const KnownDictsManager * const KnownMgr_;

		QSet<QString> LearntWords_;
	public:
		Checker (const KnownDictsManager*);
		~Checker () override;

		QStringList GetPropositions (const QString&) const override;
		bool IsCorrect (const QString&) const override;
		void LearnWord (const QString&) override;
	public slots:
		void setLanguages (const QStringList&);
	};
}
}
