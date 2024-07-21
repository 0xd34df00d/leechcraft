/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <util/sll/bitflags.h>
#include "interfaces/monocle/idocument.h"
#include "linkexecutioncontext.h"

namespace LC::Monocle
{
	class DocumentLoader;
	class FileWatcher;
	struct NavigationAction;
	class NavigationHistory;
	class PagesLayoutManager;

	class Navigator : public QObject
	{
		Q_OBJECT

		DocumentLoader& Loader_;
		const PagesLayoutManager& Layout_;
		NavigationHistory& History_;
		FileWatcher& Watcher_;
		QString CurrentPath_;

		struct NavCtx final : LinkExecutionContext
		{
			Navigator& Nav_;

			explicit NavCtx (Navigator& tab);

			void Navigate (const NavigationAction& act) override;
			void Navigate (const ExternalNavigationAction& act) override;
		} NavCtx_ { *this };
	public:
		enum class DocumentOpenOption : std::uint8_t
		{
			None = 0x0,
			IgnoreErrors = 0x1,
			NoHistoryRecord = 0x2,
		};
		using DocumentOpenOptions = Util::BitFlags<DocumentOpenOption>;

		explicit Navigator (const PagesLayoutManager&, DocumentLoader&, QObject* = nullptr);

		LinkExecutionContext& GetNavigationContext ();
		const NavigationHistory& GetNavigationHistory () const;

		void Navigate (const NavigationAction&);

		void OpenDocument (const QString&);
	private:
		void StartLoading (QString, DocumentOpenOptions, const std::optional<NavigationAction>&);
		void HandleLoaderReady (DocumentOpenOptions,
				const IDocument_ptr&,
				const QString&,
				const std::optional<NavigationAction>&);

		ExternalNavigationAction GetCurrentPosition () const;
	signals:
		void loadingFailed (const QString& path);

		void loaded (const IDocument_ptr& doc, const QString& path);

		void positionRequested (const NavigationAction&);
	};

	DECLARE_BIT_FLAGS (Navigator::DocumentOpenOption)
}
