// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.
#pragma once

#include "../backend.h"
#include "win-helpers.h"

#include <mfidl.h>
#include <mfreadwrite.h>
#include <atlcomcli.h>
#include <strmif.h>
#include <Ks.h>
#include <ksproxy.h>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace rsimpl2
{
    namespace uvc
    {
        class wmf_backend;

        struct profile_and_callback
        {
            stream_profile profile;
            frame_callback callback = nullptr;
        };

        typedef std::function<void(const uvc_device_info&, IMFActivate*)>
                enumeration_callback;

        class wmf_uvc_device : public std::enable_shared_from_this<wmf_uvc_device>,
                               public uvc_device
        {
        public:
            wmf_uvc_device(const uvc_device_info& info, std::shared_ptr<const wmf_backend> backend);
            ~wmf_uvc_device();

            void probe_and_commit(stream_profile profile, frame_callback callback, int buffers) override;
            void stream_on(std::function<void(const notification& n)> error_handler = [](const notification& n){}) override;
            void start_callbacks() override;
            void stop_callbacks() override;
            void close(stream_profile profile) override;
            void set_power_state(power_state state) override;
            power_state get_power_state() const override { return _power_state; }
            std::vector<stream_profile> get_profiles() const override;

            static bool is_connected(const uvc_device_info& info);
            static void foreach_uvc_device(enumeration_callback action);

            void init_xu(const extension_unit& xu) override;
            bool set_xu(const extension_unit& xu, uint8_t ctrl, const uint8_t* data, int len) override;
            bool get_xu(const extension_unit& xu, uint8_t ctrl, uint8_t* data, int len) const override;
            control_range get_xu_range(const extension_unit& xu, uint8_t ctrl, int len) const override;

            bool get_pu(rs2_option opt, int32_t& value) const override;
            bool set_pu(rs2_option opt, int value) override;
            control_range get_pu_range(rs2_option opt) const override;

            void lock() const override { _systemwide_lock.lock(); }
            void unlock() const override { _systemwide_lock.unlock(); }

            std::string get_device_location() const override { return _location; }

        private:
            friend class source_reader_callback;

            void play_profile(stream_profile profile, frame_callback callback);
            void stop_stream_cleanup(const stream_profile& profile, std::vector<profile_and_callback>::iterator& elem);
            void flush(int sIndex);
            void check_connection() const;
            IKsControl* get_ks_control(const extension_unit& xu) const;
            std::vector<std::pair<stream_profile, int>> get_stream_profiles_and_indexes() const;
            int get_stream_index_by_profile(const stream_profile& profile) const;

            const uvc_device_info                   _info;
            power_state                             _power_state = D3;

            CComPtr<source_reader_callback>         _callback = nullptr;
            CComPtr<IMFSourceReader>                _reader = nullptr;
            CComPtr<IMFMediaSource>                 _source = nullptr;
            CComPtr<IMFActivate>                    _activate = nullptr;
            CComPtr<IMFAttributes>                  _device_attrs = nullptr;
            CComPtr<IMFAttributes>                  _reader_attrs = nullptr;

            CComPtr<IAMCameraControl>               _camera_control = nullptr;
            CComPtr<IAMVideoProcAmp>                _video_proc = nullptr;
            std::unordered_map<int, CComPtr<IKsControl>>      _ks_controls;

            manual_reset_event                      _is_flushed;
            manual_reset_event                      _has_started;
            HRESULT                                 _readsample_result = S_OK;

            std::vector<profile_and_callback>       _streams;
            std::mutex                              _streams_mutex;

            std::shared_ptr<const wmf_backend>      _backend;

            named_mutex                             _systemwide_lock;
            std::string                             _location;
            std::vector<stream_profile>             _profiles;
            std::vector<frame_callback>             _frame_callbacks;
            bool                                    _streaming = false;
            std::atomic<bool>                       _is_started = false;
        };

        class source_reader_callback : public IMFSourceReaderCallback
        {
        public:
            explicit source_reader_callback(std::weak_ptr<wmf_uvc_device> owner) : _owner(owner)
            {
            };
            virtual ~source_reader_callback() {};
            STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;
            STDMETHODIMP_(ULONG) AddRef() override;
            STDMETHODIMP_(ULONG) Release() override;
            STDMETHODIMP OnReadSample(HRESULT /*hrStatus*/,
                DWORD dwStreamIndex,
                DWORD /*dwStreamFlags*/,
                LONGLONG /*llTimestamp*/,
                IMFSample *sample) override;
            STDMETHODIMP OnEvent(DWORD /*sidx*/, IMFMediaEvent* /*event*/) override;
            STDMETHODIMP OnFlush(DWORD) override;
        private:
            std::weak_ptr<wmf_uvc_device> _owner;
            long _refCount = 0;
        };

    }
}
