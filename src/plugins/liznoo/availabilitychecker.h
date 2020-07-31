/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <QFuture>
#include <util/threads/futures.h>

namespace LC
{
namespace Liznoo
{
	template<typename T>
	class IChecker
	{
	public:
		virtual ~IChecker () = default;

		virtual QFuture<bool> Check () = 0;

		virtual std::shared_ptr<T> Make () = 0;
	};

	template<typename T>
	using IChecker_ptr = std::unique_ptr<IChecker<T>>;

	template<typename T>
	class PureChecker final : public IChecker<T>
	{
	public:
		using Checker_t = std::function<QFuture<bool> ()>;
		using Maker_t = std::function<std::shared_ptr<T> ()>;
	private:
		const Checker_t Checker_;
		const Maker_t Maker_;
	public:
		PureChecker (const Checker_t& checker, const Maker_t& maker)
		: Checker_ { checker }
		, Maker_ { maker }
		{
		}

		QFuture<bool> Check () override
		{
			return Checker_ ();
		}

		std::shared_ptr<T> Make () override
		{
			return Maker_ ();
		}
	};

	template<typename T>
	IChecker_ptr<T> MakePureChecker (const typename PureChecker<T>::Checker_t& checker,
			const typename PureChecker<T>::Maker_t& maker)
	{
		return std::make_unique<PureChecker<T>> (checker, maker);
	}

	template<typename T>
	class AvailabilityChecker : public QObject
	{
	public:
		using Result_t = std::optional<std::shared_ptr<T>>;
	private:
		QFutureInterface<Result_t> Iface_;

		const std::vector<IChecker_ptr<T>> Checkers_;
	public:
		template<typename... Checkers>
		AvailabilityChecker (Checkers&&... checkers)
		: Checkers_
		{
			[] (auto&&... checkers)
			{
				std::vector<IChecker_ptr<T>> result;
				std::initializer_list<int> meh { (result.emplace_back (std::move (checkers)), 0)... };
				Q_UNUSED (meh)
				return result;
			} (std::move (checkers)...)
		}
		{
			Iface_.reportStarted ();

			RunChecker (Checkers_.begin ());
		}

		QFuture<Result_t> GetResult ()
		{
			return Iface_.future ();
		}
	private:
		void RunChecker (typename std::vector<IChecker_ptr<T>>::const_iterator it)
		{
			if (it == Checkers_.end ())
			{
				Util::ReportFutureResult (Iface_, Result_t {});

				deleteLater ();
				return;
			}

			Util::Sequence (this, (*it)->Check ()) >>
					[this, it] (bool result) mutable
					{
						qDebug () << Q_FUNC_INFO << result;

						if (result)
						{
							Util::ReportFutureResult (Iface_, (*it)->Make ());
							deleteLater ();
						}
						else
							RunChecker (++it);
					};
		}
	};

}
}
