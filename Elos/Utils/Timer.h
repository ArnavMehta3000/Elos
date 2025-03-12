#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/FunctionMacros.h>
#include <chrono>
#include <functional>
#include <limits>

namespace Elos
{
	class Timer
	{
	public:
		struct TimeInfo
		{
			f64 DeltaTime        = 0.0f;
			f64 TotalTime        = 0.0f;
			u64 FrameCount       = 0;
			u32 FPS              = 0;
			bool IsFixedTimeStep = false;
			f64 TargetDeltaTime  = 0.0f;
			f64 TargetFPS        = 0.0f;
		};

		using ClockType    = std::chrono::high_resolution_clock;
		using TimePoint    = ClockType::time_point;
		using Duration     = std::chrono::duration<f64>;
		using TickFunction = std::function<void(const TimeInfo&)>;

	public:
		Timer(const bool startPaused = false, const f64 targetFPS = -1)
			: m_totalTime(0.0)
			, m_deltaTime(0.0)
			, m_frameCount(0)
			, m_framesThisSecond(0)
			, m_framesPerSecond(0)
			, m_secondCounter(0.0)
			, m_isFixedTimeStep(targetFPS > 0.0)
			, m_targetElapsedTime(targetFPS > 0.0 ? FPSToSeconds(targetFPS) : 0.0)
			, m_maxDeltaTime(std::numeric_limits<f64>::max())
			, m_isPaused(startPaused)
			, m_accumulatedTime(0.0)
		{
			m_lastTime = ClockType::now();
			if (startPaused) 
			{
				m_pauseTime = m_lastTime;
			}
		}

		static constexpr inline NODISCARD f64 FPSToSeconds(f64 fps) noexcept { return fps > 0.0 ? 1.0 / fps : 0.0; }
		static constexpr inline NODISCARD f64 FPSToMilliseconds(f64 fps) noexcept { return FPSToSeconds(fps) * 1000.0; }
		static constexpr inline NODISCARD f64 SecondsToFPS(f64 seconds) noexcept { return seconds > 0.0 ? 1.0 / seconds : 0.0; }
		static constexpr inline NODISCARD f64 MillisecondsToFPS(f64 ms) noexcept { return ms > 0.0 ? 1000.0 / ms : 0.0; }
		static constexpr inline NODISCARD f64 SecondsToMilliseconds(f64 seconds) noexcept { return seconds * 1000.0; }
		static constexpr inline NODISCARD f64 MillisecondsToSeconds(f64 ms) noexcept { return ms / 1000.0; }

		static inline NODISCARD TimePoint Now() noexcept { return ClockType::now(); }
		static inline NODISCARD f64 DurationInSeconds(TimePoint start, TimePoint end) noexcept { return std::chrono::duration<f64>(end - start).count(); }
		static inline NODISCARD f64 DurationInMilliseconds(TimePoint start, TimePoint end) noexcept { return std::chrono::duration<f64, std::milli>(end - start).count(); }
		static inline NODISCARD f64 DurationInMicroseconds(TimePoint start, TimePoint end) noexcept { return std::chrono::duration<f64, std::micro>(end - start).count(); }

		inline NODISCARD f64 GetDeltaTime() const noexcept { return m_deltaTime; }
		inline NODISCARD f64 GetTotalTime() const noexcept { return m_totalTime; }
		inline NODISCARD u64 GetFrameCount() const noexcept { return m_frameCount; }
		inline NODISCARD u32 GetFPS() const noexcept { return m_framesPerSecond; }
		inline NODISCARD f64 GetTargetFPS() const noexcept { return m_isFixedTimeStep ? SecondsToFPS(m_targetElapsedTime) : -1.0; }
		inline NODISCARD f64 GetElapsedTimeSinceStart() const noexcept { return m_isPaused ? DurationInSeconds(m_lastTime, m_pauseTime) : DurationInSeconds(m_lastTime, ClockType::now()); }
		inline NODISCARD bool IsPaused() const noexcept { return m_isPaused; }

		inline void SetFixedTimeStep(const bool isFixedTimestep) noexcept { m_isFixedTimeStep = isFixedTimestep; }
		inline void SetMaxDeltaTime(const f64 maxDeltaTime) noexcept { m_maxDeltaTime = maxDeltaTime; }
		
		inline void SetTargetFPS(const f64 targetFPS) noexcept 
		{
			m_isFixedTimeStep = (targetFPS > 0.0);
			m_targetElapsedTime = targetFPS > 0.0 ? FPSToSeconds(targetFPS) : 0.0;
		}

		void Start()
		{
			if (m_isPaused)
			{
				// Calculate the time we were paused and adjust m_lastTime
				auto currentTime = ClockType::now();
				auto pauseDuration = currentTime - m_pauseTime;
				m_lastTime += pauseDuration;
				m_isPaused = false;
			}
		}

		void Stop()
		{
			if (!m_isPaused)
			{
				m_pauseTime = ClockType::now();
				m_isPaused = true;
			}
		}

		void Reset()
		{
			m_lastTime = ClockType::now();
			m_accumulatedTime = 0.0;
			m_frameCount = 0;
			m_framesPerSecond = 0;
			m_framesThisSecond = 0;
			m_secondCounter = 0.0;
			m_totalTime = 0.0;
			m_deltaTime = 0.0;
		}

		void TogglePause()
		{
			if (m_isPaused) Start();
			else Stop();
		}

		inline NODISCARD TimeInfo GetTimeInfo() const noexcept
		{
			return TimeInfo
			{
				.DeltaTime = m_deltaTime,
				.TotalTime = m_totalTime,
				.FrameCount = m_frameCount,
				.FPS = m_framesPerSecond,
				.IsFixedTimeStep = m_isFixedTimeStep,
				.TargetDeltaTime = m_targetElapsedTime,
				.TargetFPS = SecondsToFPS(m_targetElapsedTime)
			};
		}

		// Process one frame of the game loop
		// For variable timestep, update is called once with the actual elapsed time
		// For fixed timestep, update may be called multiple times with fixed intervals
		void Tick(const TickFunction& tickFunc)
		{
			if (m_isPaused)
			{
				return;
			}

			TimePoint currentTime = ClockType::now();
			Duration elapsed      = currentTime - m_lastTime;
			m_lastTime            = currentTime;
			f64 elapsedSeconds    = elapsed.count();

			// Clamp to avoid spiral of death after breakpoints or long loads
			if (elapsedSeconds > m_maxDeltaTime)
			{
				elapsedSeconds = m_maxDeltaTime;
			}

			// Track time for FPS calculation
			m_secondCounter += elapsedSeconds;

			if (m_isFixedTimeStep)
			{
				m_accumulatedTime += elapsedSeconds;

				constexpr f64 epsilon = std::numeric_limits<f64>::epsilon() * 100.0;
				if (std::abs(m_accumulatedTime - m_targetElapsedTime) < 
					epsilon * std::max(1.0, std::abs(m_targetElapsedTime)))
				{
					m_accumulatedTime = m_targetElapsedTime;
				}

				while (m_accumulatedTime >= m_targetElapsedTime)
				{
					m_deltaTime = m_targetElapsedTime;
					m_totalTime += m_targetElapsedTime;
					m_accumulatedTime -= m_targetElapsedTime;
					m_frameCount++;

					tickFunc(GetTimeInfo());

					m_framesThisSecond++;
				}
			}
			else
			{
				m_deltaTime = elapsedSeconds;
				m_totalTime += elapsedSeconds;
				m_frameCount++;

				tickFunc(GetTimeInfo());

				m_framesThisSecond++;
			}

			// Update FPS counter once per second
			if (m_secondCounter >= 1.0)
			{
				m_framesPerSecond = m_framesThisSecond;
				m_framesThisSecond = 0;
				m_secondCounter -= 1.0;
			}
		}

	private:
		TimePoint m_lastTime;
		TimePoint m_pauseTime;
		f64       m_totalTime;
		f64       m_deltaTime;
		f64       m_accumulatedTime;
		f64       m_maxDeltaTime;
		u64       m_frameCount;
		u32       m_framesPerSecond;
		u32       m_framesThisSecond;
		f64       m_secondCounter;
		bool      m_isPaused;
		bool      m_isFixedTimeStep;
		f64       m_targetElapsedTime;
	};

	class ScopedTimer
	{
	public:
		using TimeInfoCallback = std::function<void(const Timer::TimeInfo&)>;

		explicit ScopedTimer(TimeInfoCallback callback = nullptr) noexcept
			: m_callback(std::move(callback))
			, m_startTime(Timer::Now())
			, m_active(true)
		{
		}

		ScopedTimer(const ScopedTimer&) = delete;
		ScopedTimer& operator=(const ScopedTimer&) = delete;
		ScopedTimer(ScopedTimer&& other) noexcept = default;
		ScopedTimer& operator=(ScopedTimer&& other) noexcept = default;

		~ScopedTimer()
		{
			if (m_active && m_callback)
			{
				auto endTime = Timer::Now();
				double elapsedSeconds = Timer::DurationInSeconds(m_startTime, endTime);

				Timer::TimeInfo info{};
				info.DeltaTime       = elapsedSeconds;
				info.TotalTime       = elapsedSeconds;
				info.FrameCount      = 1;
				info.FPS             = 0;
				info.IsFixedTimeStep = false;
				info.TargetDeltaTime = 0.0;
				info.TargetFPS       = 0.0;

				m_callback(info);
			}
		}

	private:
		TimeInfoCallback m_callback;
		Timer::TimePoint m_startTime;
		bool m_active;
	};
}