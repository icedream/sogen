#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace sogen
{
    struct audio_format
    {
        uint32_t sample_rate{};
        uint16_t channels{};
        uint16_t bits_per_sample{};
        bool is_float{};

        bool operator==(const audio_format& o) const
        {
            return sample_rate == o.sample_rate && channels == o.channels && bits_per_sample == o.bits_per_sample && is_float == o.is_float;
        }
    };

    class audio_backend
    {
      public:
        virtual ~audio_backend() = default;

        // Open (or reconfigure) the host output device for the given format and begin playback. Returns false
        // when no host audio device is available; the caller then treats submit/stop as no-ops.
        virtual bool start(const audio_format& format) = 0;

        // Enqueue interleaved PCM in the format last passed to start(). The host plays it back at the device
        // rate, buffering as needed.
        virtual void submit(const void* data, size_t size) = 0;

        // Bytes handed to submit() that the device has not played yet, when the backend can report it. Callers
        // use it to derive how much audio actually reached the speakers instead of estimating from wall clock,
        // which lets them apply real backpressure and keep latency bounded. std::nullopt = not supported.
        virtual std::optional<uint64_t> queued_bytes() const
        {
            return std::nullopt;
        }

        virtual void stop() = 0;
    };

    class null_audio_backend final : public audio_backend
    {
      public:
        bool start(const audio_format& /*format*/) override
        {
            return false;
        }

        void submit(const void* /*data*/, size_t /*size*/) override
        {
        }

        void stop() override
        {
        }
    };

    class audio_capture_backend
    {
      public:
        virtual ~audio_capture_backend() = default;

        // Open (or reconfigure) the host input device for the given format and begin recording. Returns
        // false when the host has no capture device; the caller then treats the stream as silence.
        virtual bool start(const audio_format& format) = 0;

        // Take up to size bytes of recorded interleaved PCM in the format last passed to start(). Returns
        // the number of bytes copied, 0 when nothing has been captured yet.
        virtual size_t read(void* destination, size_t size) = 0;

        virtual void stop() = 0;
    };

    class null_audio_capture_backend final : public audio_capture_backend
    {
      public:
        bool start(const audio_format& /*format*/) override
        {
            return false;
        }

        size_t read(void* /*destination*/, size_t /*size*/) override
        {
            return 0;
        }

        void stop() override
        {
        }
    };

    std::unique_ptr<audio_backend> create_default_audio_backend();
    std::unique_ptr<audio_backend> create_sdl_audio_backend();
    std::unique_ptr<audio_backend> create_aaudio_audio_backend();

    std::unique_ptr<audio_capture_backend> create_default_audio_capture_backend();
    std::unique_ptr<audio_capture_backend> create_sdl_audio_capture_backend();

} // namespace sogen
