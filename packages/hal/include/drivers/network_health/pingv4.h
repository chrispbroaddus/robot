#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <future>
#include <random>
#include <string>
#include <vector>

#include "ping_statistics.h"
#include "resolve_statistics.h"

namespace hal {
namespace details {

    /// Implements IPv4 / ICMPv4 ping
    ///
    /// @par [Assumptions]
    /// -# This class is only instantiated in processes where root is the effective user. If you run this as an
    ///    unprivileged user, you will see exceptions thrown in the constructure which are due to permissions problems.
    /// -# Ping will block until boost::asio::io_service::run is called on the IO service that was provided to our
    ///    constructor. If the IO service is not running, it will appear as if calls to ping() deadlock.
    /// .
    ///
    /// @par [Thread Safety]
    /// There are two sets of thread-safety concerns:
    /// -# Background threads which may be bound to io_service::run(). The implementation of this class is safe to use with
    ///    multiple worker threads.
    /// -# Concurrent calls to ping(). It is @b{not} safe to make multiple concurrent calls to ping() on a single instance;
    ///    it is, however, safe to do so if each thread has its own instance.
    /// .
    class PingV4 {
    public:
        static constexpr uint32_t maximumPingAttempts = 10;
        static constexpr uint32_t maximumAllowedRTTSeconds = 5;
        static constexpr uint32_t maximumResolveAttempts = 5;

        using clock_type = std::chrono::high_resolution_clock;
        using time_point = clock_type::time_point;
        using duration = clock_type::duration;
        using resolve_statistics = ResolveStatistics<clock_type>;
        using ping_statistics = PingStatistics<maximumPingAttempts, clock_type>;

        /// \param svc
        /// \param destinations
        PingV4(boost::asio::io_service& svc, const std::vector<std::string>& destinations);

        /// -# Choose a target from the list of destinations that were provided at construction uniformly at random.
        /// -# Make up to maximumResolveAttempts attempts to resolve that target to an IPv4 address. If there are multiple
        ///    candidate resolutions, choose one uniformly at random.
        /// -# If resolution is successful, attempt to send up to maximumPingAttempts ICMPv4 Echo Request packets and record
        ///    how many are successfully received within (maximumPingAttempts + 1) * maximumAllowedRTTSeconds seconds.
        /// .
        ///
        /// @note
        /// -# Each call to this function will take a minimum of maximumPingAttempts seconds. This is because we don't issue
        ///    ICMP echo requests at a rate greater than one per second.
        /// .
        /// \return
        /// If the resolve statistics indicate a successful resolve, ping statistics will also be populated.
        std::pair<resolve_statistics, ping_statistics> ping();

    private:
        /// ID that we use in all our echo requests
        uint16_t identifier;

        /// Networking-related stuff @{
        std::vector<std::string> destinations;
        boost::asio::deadline_timer attemptTimer;
        boost::asio::deadline_timer resolveTimer;
        boost::asio::deadline_timer receiveTimer;
        boost::asio::deadline_timer sendTimer;
        boost::asio::ip::icmp::resolver resolver;
        boost::asio::ip::icmp::socket socket;
        boost::asio::ip::icmp::endpoint endpoint;
        /// @}

        /// High quality PRNG
        std::mt19937_64 prng;

        /// State associated with resolving targets @{
        boost::asio::ip::icmp::resolver::query query;
        resolve_statistics resolveStatistics;
        /// @}

        /// State associated with actual pings @{
        boost::asio::streambuf receiveBuffer;
        ping_statistics pingStatistics;
        ///@}

        void startResolve(std::promise<resolve_statistics>& promise);

        void attemptResolve(std::promise<resolve_statistics>& promise);

        void chooseTarget();

        void handleResolveComplete(
            const boost::system::error_code& ec, boost::asio::ip::icmp::resolver::iterator it, std::promise<resolve_statistics>& promise);

        void handleUnsuccessfulResolve(const boost::system::error_code& ec, std::promise<resolve_statistics>& promise);

        ping_statistics sendICMPEchoRequests();

        void startReceiveICMP(std::promise<ping_statistics>& promise);
        void attemptSendICMPEchoRequest();

        void handleReceiveICMP(const boost::system::error_code& ec, size_t bytes);
    };
}
}
