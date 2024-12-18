/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <functional>
#include <memory>
#include <optional>
#include <QFutureInterface>
#include <QFutureWatcher>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/detector.h>
#include "threadsconfig.h"
#include "concurrentexception.h"

namespace LC::Util
{
	template<typename R, typename F, typename... Args>
	void ReportFutureResult (QFutureInterface<R>& iface, F&& f, Args&&... args)
	{
		try
		{
			constexpr bool isVoid = std::is_same_v<R, void>;
			if constexpr (!isVoid && !std::is_invocable_v<std::decay_t<F>, Args...>)
			{
				static_assert (std::is_constructible_v<R, F>);
				static_assert (sizeof... (Args) == 0,
						"Extra args when a value is passed. Perhaps you wanted to pass in a function?");

				const R result { std::forward<F> (f) };
				iface.reportResult (result);
			}
			else if constexpr (!isVoid)
			{
				const auto result = std::invoke (std::forward<F> (f), std::forward<Args> (args)...);
				iface.reportResult (result);
			}
			else
				std::invoke (std::forward<F> (f), std::forward<Args> (args)...);
		}
		catch (const QtException_t& e)
		{
			iface.reportException (e);
		}
		catch (const std::exception& e)
		{
			iface.reportException (ConcurrentStdException { e });
		}

		iface.reportFinished ();
	}

	namespace detail
	{
		template<typename T>
		struct UnwrapFutureTypeBase {};

		template<typename T>
		struct UnwrapFutureTypeBase<QFuture<T>>
		{
			using type = T;
		};

		template<typename T>
		struct UnwrapFutureType : UnwrapFutureTypeBase<std::decay_t<T>>
		{
		};
	}

	template<typename T>
	using UnwrapFutureType_t = typename detail::UnwrapFutureType<T>::type;

	namespace detail
	{
		/** @brief Incapsulates the sequencing logic of asynchronous
		 * actions.
		 *
		 * The objects of this class are expected to be created on heap.
		 * They will delete themselves automatically after the chain is
		 * walked (or an exception is thrown).
		 *
		 * @tparam Future The type of the initial future.
		 */
		template<typename Future>
		class Sequencer final : public QObject
		{
		public:
			/** @brief The type instantinating the QFuture returned by the
			 * \em Executor.
			 */
			using RetType_t = UnwrapFutureType_t<Future>;
		private:
			Future Future_;
			QFutureWatcher<RetType_t> BaseWatcher_;
			QFutureWatcherBase *LastWatcher_ = &BaseWatcher_;
		public:
			/** @brief Constructs the sequencer.
			 *
			 * @param[in] future The initial future in the chain.
			 * @param[in] parent The parent object for the sequencer.
			 */
			Sequencer (const Future& future, QObject *parent)
			: QObject { parent }
			, Future_ { future }
			, BaseWatcher_ { this }
			{
			}

			/** @brief Starts the first action in the chain.
			 *
			 * All the actions should be chained before calling this
			 * method to avoid a race condition.
			 */
			void Start ()
			{
				connect (LastWatcher_,
						&QFutureWatcherBase::finished,
						this,
						&QObject::deleteLater);
				BaseWatcher_.setFuture (Future_);
			}

			/** @brief Chains the given asynchronous action.
			 *
			 * The \em action is a functor callable with a single
			 * parameter of type \em ArgT and returning a value of type
			 * <code>QFuture<RetT></code> for some \em RetT.
			 *
			 * The parameter type \em ArgT should match exactly the
			 * "unwrapped" \em RetT for the previous call of Then() (or
			 * RetType_t if this is the second action in the asynchronous
			 * chain). Otherwise, an exception will be thrown at runtime.
			 *
			 * @note The SequenceProxy class takes care of compile-time
			 * type-checking of arguments and return types.
			 *
			 * @param[in] action The action to add to the sequence chain.
			 * @tparam RetT The type instantiating the return type
			 * <code>QFuture<RetT></code> of the \em action.
			 * @tparam ArgT The type of the argument passed to the
			 * \em action.
			 */
			template<typename RetT, typename ArgT>
			void Then (const std::function<QFuture<RetT> (ArgT)>& action)
			{
				const auto last = dynamic_cast<QFutureWatcher<ArgT>*> (LastWatcher_);
				if (!last)
				{
					deleteLater ();
					throw std::runtime_error { std::string { "invalid type in " } + Q_FUNC_INFO };
				}

				const auto watcher = new QFutureWatcher<RetT> { this };
				LastWatcher_ = watcher;

				new SlotClosure<DeleteLaterPolicy>
				{
					[this, last, watcher, action]
					{
						if (static_cast<QObject*> (last) != &BaseWatcher_)
							last->deleteLater ();
						watcher->setFuture (action (last->result ()));
					},
					last,
					SIGNAL (finished ()),
					last
				};
			}

			/** @brief Chains the given asynchronous action and closes the
			 * chain.
			 *
			 * The \em action is a functor callable with a single
			 * parameter of type \em ArgT and returning <code>void</code>.
			 *
			 * No more functors may be chained after adding a
			 * <code>void</code>-returning functor.
			 *
			 * The parameter type \em ArgT should match exactly the
			 * "unwrapped" \em RetT for the previous call of Then() (or
			 * RetType_t if this is the second action in the asynchronous
			 * chain). Otherwise, an exception will be thrown at runtime.
			 *
			 * @note The SequenceProxy class takes care of compile-time
			 * type-checking of arguments and return types.
			 *
			 * @tparam ArgT The type of the argument passed to the
			 * \em action.
			 */
			template<typename ArgT>
			void Then (const std::function<void (ArgT)>& action)
			{
				const auto last = dynamic_cast<QFutureWatcher<ArgT>*> (LastWatcher_);
				if (!last)
				{
					deleteLater ();
					throw std::runtime_error { std::string { "invalid type in " } + Q_FUNC_INFO };
				}

				new SlotClosure<DeleteLaterPolicy>
				{
					[last, action]
					{
						action (last->result ());
					},
					LastWatcher_,
					SIGNAL (finished ()),
					LastWatcher_
				};
			}

			void Then (const std::function<void ()>& action)
			{
				const auto last = dynamic_cast<QFutureWatcher<void>*> (LastWatcher_);
				if (!last)
				{
					deleteLater ();
					throw std::runtime_error { std::string { "invalid type in " } + Q_FUNC_INFO };
				}

				new SlotClosure<DeleteLaterPolicy>
				{
					action,
					LastWatcher_,
					SIGNAL (finished ()),
					LastWatcher_
				};
			}

			template<typename Handler>
			void MultipleResults (const Handler& handler,
					const std::function<void ()>& finishHandler = {},
					const std::function<void ()>& startHandler = {})
			{
				if (LastWatcher_ != &BaseWatcher_)
				{
					qWarning () << Q_FUNC_INFO
							<< "multiple results handler should be chained directly to the source";
					throw std::runtime_error { "invalid multiple results handler chaining" };
				}

				connect (&BaseWatcher_,
						&QFutureWatcherBase::resultReadyAt,
						&BaseWatcher_,
						[handler, this] (int index) { handler (BaseWatcher_.resultAt (index)); });

				if (finishHandler)
					new Util::SlotClosure<Util::DeleteLaterPolicy>
					{
						finishHandler,
						&BaseWatcher_,
						SIGNAL (finished ()),
						&BaseWatcher_
					};

				if (startHandler)
					new Util::SlotClosure<Util::DeleteLaterPolicy>
					{
						startHandler,
						&BaseWatcher_,
						SIGNAL (started ()),
						&BaseWatcher_
					};

				connect (&BaseWatcher_,
						SIGNAL (finished ()),
						this,
						SLOT (deleteLater ()));
			}
		};

		template<typename T>
		using SequencerRetType_t = typename Sequencer<T>::RetType_t;

		struct EmptyDestructionTag;

		/** @brief A proxy object allowing type-checked sequencing of
		 * actions and responsible for starting the initial action.
		 *
		 * SequenceProxy manages a Sequencer object, which itself is
		 * directly responsible for walking the chain of sequenced
		 * actions.
		 *
		 * Internally, objects of this class are reference-counted. As
		 * soon as the last instance is destroyed, the initial action is
		 * started.
		 *
		 * @tparam Ret The type \em T that <code>QFuture<T></code>
		 * returned by the last chained executor is specialized with.
		 * @tparam E0 The type of the first executor.
		 * @tparam A0 The types of the arguments to the executor \em E0.
		 */
		template<typename Ret, typename Future, typename DestructionTag>
		class SequenceProxy
		{
			template<typename, typename, typename>
			friend class SequenceProxy;

			std::shared_ptr<void> ExecuteGuard_;
			Sequencer<Future> * const Seq_;

			std::optional<QFuture<Ret>> ThisFuture_;

			std::function<DestructionTag ()> DestrHandler_;

			SequenceProxy (const std::shared_ptr<void>& guard, Sequencer<Future> *seq,
					const std::function<DestructionTag ()>& destrHandler)
			: ExecuteGuard_ { guard }
			, Seq_ { seq }
			, DestrHandler_ { destrHandler }
			{
			}

			template<typename F1, typename Ret1>
			using ReturnsFutureDetector_t = UnwrapFutureType_t<std::invoke_result_t<F1, Ret1>>;

			template<typename F, typename... Args>
			using ReturnsVoidDetector_t = std::invoke_result_t<F, Args...>;
		public:
			using Ret_t = Ret;

			/** @brief Constructs a sequencer proxy managing the given
			 * \em sequencer.
			 *
			 * @param[in] sequencer The sequencer to manage.
			 */
			SequenceProxy (Sequencer<Future> *sequencer)
			: ExecuteGuard_ { nullptr, [sequencer] (void*) { sequencer->Start (); } }
			, Seq_ { sequencer }
			{
			}

			/** @brief Copy-constructs from \em proxy.
			 *
			 * @param[in] proxy The proxy object to share the managed
			 * sequencer with.
			 */
			SequenceProxy (const SequenceProxy& proxy) = delete;

			/** @brief Move-constructs from \em proxy.
			 *
			 * @param[in] proxy The proxy object from which the sequencer
			 * should be borrowed.
			 */
			SequenceProxy (SequenceProxy&& proxy) = default;

			/** @brief Adds the functor \em f to the chain of actions.
			 *
			 * @param[in] f The functor to add to the chain.
			 * @return A new SequenceProxy if the chain can be continued further on, \code void otherwise..
			 * @tparam F The type of the functor to chain.
			 */
			template<typename F>
			auto Then (F&& f)
			{
				if (ThisFuture_)
					throw std::runtime_error { "SequenceProxy::Then(): cannot chain more after being converted to a QFuture" };

				if constexpr (IsDetected_v<ReturnsFutureDetector_t, F, Ret>)
				{
					using Next_t = UnwrapFutureType_t<decltype (f (std::declval<Ret> ()))>;
					Seq_->template Then<Next_t, Ret> (f);
					return SequenceProxy<Next_t, Future, DestructionTag> { ExecuteGuard_, Seq_, DestrHandler_ };
				}
				else if constexpr (std::is_same<IsDetected_t<struct Dummy, ReturnsVoidDetector_t, F, Ret>, void> {})
					Seq_->template Then<Ret> (f);
				else if constexpr (std::is_same<void, Ret>::value &&
						std::is_same<IsDetected_t<struct Dummy, ReturnsVoidDetector_t, F>, void> {})
					Seq_->Then (std::function<void ()> { f });
				else
					static_assert (std::is_same<F, struct Dummy> {}, "Invalid functor passed to SequenceProxy::Then()");
			}

			template<typename F>
			auto operator>> (F&& f) -> decltype (this->Then (std::forward<F> (f)))
			{
				return Then (std::forward<F> (f));
			}

			template<typename F>
			SequenceProxy<Ret, Future, std::invoke_result_t<F>> DestructionValue (F&& f)
			{
				static_assert (std::is_same<DestructionTag, EmptyDestructionTag>::value,
						"Destruction handling function has been already set.");

				return { ExecuteGuard_, Seq_, std::forward<F> (f) };
			}

			template<typename F>
			void MultipleResults (F&& f)
			{
				Seq_->MultipleResults (std::forward<F> (f));
			}

			template<typename F, typename Finish>
			void MultipleResults (F&& f, Finish&& finish)
			{
				Seq_->MultipleResults (std::forward<F> (f),
						std::forward<Finish> (finish));
			}

			template<typename F, typename Finish, typename Start>
			void MultipleResults (F&& f, Finish&& finish, Start&& start)
			{
				Seq_->MultipleResults (std::forward<F> (f),
						std::forward<Finish> (finish),
						std::forward<Start> (start));
			}

			operator QFuture<Ret> ()
			{
				constexpr bool isEmptyDestr = std::is_same<DestructionTag, EmptyDestructionTag>::value;
				static_assert (std::is_same<DestructionTag, Ret>::value || isEmptyDestr,
						"Destruction handler's return type doesn't match expected future type.");

				if (ThisFuture_)
					return *ThisFuture_;

				QFutureInterface<Ret> iface;
				iface.reportStarted ();

				SlotClosure<DeleteLaterPolicy> *deleteGuard = nullptr;
				if constexpr (!isEmptyDestr)
				{
					deleteGuard = new SlotClosure<DeleteLaterPolicy>
					{
						[destrHandler = DestrHandler_, iface] () mutable
						{
							if (iface.isFinished ())
								return;

							const auto res = destrHandler ();
							iface.reportFinished (&res);
						},
						Seq_->parent (),
						SIGNAL (destroyed ()),
						Seq_
					};
				}

				Then ([deleteGuard, iface] (const Ret& ret) mutable
						{
							iface.reportFinished (&ret);

							delete deleteGuard;
						});

				const auto& future = iface.future ();
				ThisFuture_ = future;
				return future;
			}
		};
	}

	/** @brief Creates a sequencer that allows chaining multiple futures.
	 *
	 * This function creates a sequencer object that starts with the
	 * passed \em future, and, after this future being completed, passes
	 * its result to the next function in chain, and so on, until either
	 * there are no more functions in the chain or a function returns
	 * something different from <code>QFuture<T></code>
	 * and so on.
	 *
	 * Each function in the chain may return a <code>QFuture<T></code> for
	 * some <code>T != void</code>, in which case it will be unwrapped and
	 * passed along to the next function in the chain.
	 *
	 * The functors may also return <code>QFuture<void></code>, meaning
	 * that the next function in the chain will be invoked without
	 * arguments when this future is completed.
	 *
	 * If a functor returns <code>void</code>, no further chaining is
	 * possible.
	 *
	 * The functions are chained via the detail::SequenceProxy::Then()
	 * method or via the <code>operator>>()</code> (leading to a nice
	 * somewhat monadic-like syntax).
	 *
	 * The \em parent QObject controls the lifetime of the sequencer: as
	 * soon as it is destroyed, the sequencer is destroyed as well, and
	 * all pending actions are cancelled (the currently executing action
	 * will still continue to execute, though). This parameter is optional
	 * and may be <code>nullptr</code>.
	 *
	 * A sample usage may look like:
	 * \code
		Util::Sequence (this,
				QtConcurrent::run ([this, &]
						{
							const auto& contents = file->readAll ();
							file->close ();
							file->remove ();
							return DoSomethingWith (contents);
						})) >>
				[this, url, script] (const QString& contents)
				{
					const auto& result = Parse (contents);
					if (result.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "empty result for"
								<< url;
						return;
					}

					const auto id = DoSomethingSynchronouslyWith (result);
					emit gotResult (id);
				};
	   \endcode
	 *
	 * @param[in] parent The parent object of the sequencer (may be
	 * <code>nullptr</code>.
	 * @param[in] future The future to pass to the sequencer.
	 * @return The sequencer object.
	 * @tparam T The underlying type of the passed future (the async
	 * computation result type).
	 *
	 * @sa detail::SequenceProxy
	 */
	template<typename T>
	detail::SequenceProxy<
			detail::SequencerRetType_t<QFuture<T>>,
			QFuture<T>,
			detail::EmptyDestructionTag
		>
		Sequence (QObject *parent, const QFuture<T>& future)
	{
		return { new detail::Sequencer<QFuture<T>> { future, parent } };
	}

	/** @brief Creates a ready future holding the given value.
	 *
	 * This function creates a ready future containing the value \em t.
	 * That is, calling <code>QFuture<T>::get()</code> on the returned
	 * future will not block.
	 *
	 * @param[in] t The value to keep in the future.
	 * @return The ready future with the value \em t.
	 *
	 * @tparam T The type of the value in the future.
	 */
	template<typename T>
	QFuture<T> MakeReadyFuture (const T& t)
	{
		QFutureInterface<T> iface;
		iface.reportStarted ();
		iface.reportFinished (&t);
		return iface.future ();
	}
}
