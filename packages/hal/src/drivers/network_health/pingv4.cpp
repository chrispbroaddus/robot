#include "packages/hal/include/drivers/network_health/pingv4.h"
#include "glog/logging.h"
#include "packages/hal/include/drivers/network_health/icmpv4_header.h"
#include "packages/hal/include/drivers/network_health/ipv4_header.h"

#include <unistd.h>

using icmp = boost::asio::ip::icmp;
namespace posix_time = boost::posix_time;

namespace {
constexpr int DEBUG = 2;
constexpr int VERBOSE = 1;
}

namespace hal {
namespace details {

    PingV4::PingV4(boost::asio::io_service& svc, const std::vector<std::string>& dst)
        : identifier(getpid())
        , destinations(dst)
        , attemptTimer(svc)
        , resolveTimer(svc)
        , receiveTimer(svc)
        , sendTimer(svc)
        , resolver(svc)
        , socket(svc, icmp::v4())
        , query("") {
        if (dst.empty()) {
            throw std::invalid_argument("Require at least one candidate destination");
        }

        std::random_device dev;
        prng.seed(dev());
    }

    std::pair<PingV4::resolve_statistics, PingV4::ping_statistics> PingV4::ping() {
        std::pair<resolve_statistics, ping_statistics> result;

        VLOG(VERBOSE) << "Starting monitor attempt";

        resolveStatistics.clear();
        pingStatistics.clear();

        std::promise<resolve_statistics> resolvePromise;
        auto resolveFuture = resolvePromise.get_future();
        startResolve(resolvePromise);

        result.first = resolveFuture.get();

        if (result.first.successfulAttempts) {
            result.second = sendICMPEchoRequests();
        }

        LOG(INFO) << "Done with monitor attempt";
        return result;
    }

    void PingV4::chooseTarget() {
        // Pick a victim.
        resolveStatistics.targetHost = destinations.at(prng() % destinations.size());

        // Start the resolve.
        VLOG(VERBOSE) << "Chose target [" << resolveStatistics.targetHost << "]";
    }

    void PingV4::handleResolveComplete(
        const boost::system::error_code& ec, boost::asio::ip::icmp::resolver::iterator it, std::promise<resolve_statistics>& promise) {
        resolveTimer.cancel();

        if (ec) {
            handleUnsuccessfulResolve(ec, promise);
        } else {
            // Resolve succeeded, now open the socket
            ++resolveStatistics.successfulAttempts;

            std::vector<icmp::resolver::iterator::value_type> candidateEndpoints(it, icmp::resolver::iterator());

            VLOG(VERBOSE) << "Resolved [" << candidateEndpoints.size() << "] candidate end points.";

            for (const auto& entry : candidateEndpoints) {
                VLOG(DEBUG) << "RESOLVED CANDIDATE: TARGET [" << resolveStatistics.targetHost << "] --> HOST [" << entry.host_name()
                            << "], SERVICE NAME [" << entry.service_name() << "], ADDRESS [" << entry.endpoint().address().to_string()
                            << "], PORT [" << entry.endpoint().port() << "]";
            }

            endpoint = candidateEndpoints.at(prng() % candidateEndpoints.size());
            resolveStatistics.targetAddress = endpoint.address().to_string();

            VLOG(VERBOSE) << "Using endpoint ADDRESS [" << endpoint.address().to_string() << "], PORT [" << endpoint.port() << "]";
            promise.set_value(resolveStatistics);
        }
    }

    void PingV4::startResolve(std::promise<resolve_statistics>& promise) {
        resolveStatistics.maximumAttempts = maximumResolveAttempts;
        chooseTarget();

        query = icmp::resolver::query(icmp::v4(), resolveStatistics.targetHost, "", icmp::resolver::query::flags::canonical_name);
        resolveStatistics.resolveStart = clock_type::now();
        attemptResolve(promise);
    }

    void PingV4::attemptResolve(std::promise<resolve_statistics>& promise) {
        VLOG(VERBOSE) << "Starting resolve attempt [target = " << resolveStatistics.targetHost
                      << ", unsuccesful attempts = " << resolveStatistics.unsuccessfulAttempts
                      << " / successful attempts = " << resolveStatistics.successfulAttempts << "]";

        resolver.async_resolve(query, [this, &promise](const boost::system::error_code& ec, icmp::resolver::iterator iter) {
            handleResolveComplete(ec, iter, promise);
        });

        VLOG(VERBOSE) << "Setting resolve timeout";
        resolveTimer.expires_from_now(posix_time::seconds(1));
        resolveTimer.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                LOG(INFO) << "Resolve attempt timed out";
                resolver.cancel();
            }
        });
    }

    void PingV4::handleUnsuccessfulResolve(const boost::system::error_code& ec, std::promise<resolve_statistics>& promise) {
        // Resolve failed
        ++resolveStatistics.unsuccessfulAttempts;
        LOG(INFO) << "Resolve of [" << resolveStatistics.targetHost << "] failed: [" << ec.message() << "] (attempt ["
                  << resolveStatistics.unsuccessfulAttempts << " / " << maximumResolveAttempts << "]";

        if (resolveStatistics.unsuccessfulAttempts >= maximumResolveAttempts) {
            resolveStatistics.resolveDuration = clock_type::now() - resolveStatistics.resolveStart;
            std::chrono::duration<double> timeToResolveInSeconds(resolveStatistics.resolveDuration);
            LOG(ERROR) << "All resolve attempts exhausted; giving up after [" << timeToResolveInSeconds.count() << "]s";
            promise.set_value(resolveStatistics);
        } else {
            VLOG(VERBOSE) << "Scheduling next resolve attempt for [" << resolveStatistics.targetHost << "]";
            resolveTimer.expires_from_now(posix_time::seconds(1));
            resolveTimer.async_wait([this, &promise](const boost::system::error_code& ec) {
                if (!ec) {
                    attemptResolve(promise);
                } else {
                    // Hypothetically impossible but I'm paranoid.
                    LOG(ERROR) << "Should not happen: timer scheduling next resolve failed [" << ec.message() << "]";
                }
            });
        }
    }

    PingV4::ping_statistics PingV4::sendICMPEchoRequests() {
        std::promise<ping_statistics> promise;

        attemptSendICMPEchoRequest();
        startReceiveICMP(promise);
        auto pingFuture = promise.get_future();
        const auto result = pingFuture.get();
        return result;
    }

    void PingV4::attemptSendICMPEchoRequest() {
        constexpr size_t payloadSize = 28;
        constexpr std::array<uint8_t, payloadSize> body{ { 0 } };
        ICMPV4Header header;
        header.type(ICMPV4MessageType::EchoRequest);
        header.code(0);
        header.identifier(identifier);
        header.sequence(pingStatistics.echoRequestsSent);

        boost::asio::streambuf requestBuffer;
        std::ostream out(&requestBuffer);
        out << ICMPV4Header::checksum(header, body.begin(), body.end());
        out.write(reinterpret_cast<const char*>(body.data()), body.size());

        VLOG(VERBOSE) << "Sending ICMP echo request with sequence [" << pingStatistics.echoRequestsSent << "]";
        pingStatistics.transmitStart.at(pingStatistics.echoRequestsSent) = clock_type::now();
        socket.send_to(requestBuffer.data(), endpoint);

        if (++pingStatistics.echoRequestsSent < maximumPingAttempts) {
            VLOG(VERBOSE) << "Scheduling next send";
            sendTimer.expires_from_now(posix_time::seconds(1));
            sendTimer.async_wait([this](const boost::system::error_code&) { attemptSendICMPEchoRequest(); });
        }
    }

    void PingV4::startReceiveICMP(std::promise<ping_statistics>& promise) {
        receiveTimer.expires_from_now(posix_time::seconds((maximumResolveAttempts + 1) * maximumAllowedRTTSeconds));
        receiveTimer.async_wait([this, &promise](const boost::system::error_code& ec) {
            // shut it all down, we're done
            VLOG(VERBOSE) << "Receive timer expired or was cancelled [" << ec.message() << "]; cancelling socket and publishing results";
            socket.cancel();
            promise.set_value(pingStatistics);
        });

        VLOG(DEBUG) << "Scheduling first receive";
        receiveBuffer.consume(receiveBuffer.size());
        socket.async_receive(
            receiveBuffer.prepare(0x1000), [this](const boost::system::error_code& ec, size_t bytes) { handleReceiveICMP(ec, bytes); });
    }

    void PingV4::handleReceiveICMP(const boost::system::error_code& ec, size_t bytes) {

        VLOG(DEBUG) << "Received packet, EC [" << ec.message() << "]";

        if (!ec) {
            receiveBuffer.commit(bytes);
            std::istream in(&receiveBuffer);

            IPV4Header ipHeader;
            ICMPV4Header icmpHeader;

            // Read in headers. If either fails, this sets the failbit on in which causes us to fall through the if below
            in >> ipHeader >> icmpHeader;

            if (in) {
                VLOG(DEBUG) << "Received ICMP packet type [" << static_cast<uint16_t>(icmpHeader.type()) << "], code ["
                            << static_cast<uint16_t>(icmpHeader.code()) << "], source [" << ipHeader.sourceAddress() << "], destination ["
                            << ipHeader.destinationAddress() << "], TTL [" << static_cast<uint16_t>(ipHeader.timeToLive()) << "]";

                if (icmpHeader.type() == ICMPV4MessageType::EchoReply && icmpHeader.identifier() == identifier
                    && icmpHeader.sequence() < maximumPingAttempts) {

                    ++pingStatistics.echoRequestsReceived;
                    pingStatistics.roundTripTimes.at(icmpHeader.sequence())
                        = clock_type::now() - pingStatistics.transmitStart.at(icmpHeader.sequence());

                    VLOG(VERBOSE) << "Successfully received response [" << pingStatistics.echoRequestsReceived << " / "
                                  << pingStatistics.echoRequestsSent << "]";
                }
            }

            if (pingStatistics.echoRequestsReceived < maximumPingAttempts) {
                VLOG(DEBUG) << "Scheduling next receive (received [" << pingStatistics.echoRequestsReceived << "] of ["
                            << maximumPingAttempts << "])";
                receiveBuffer.consume(bytes);
                socket.async_receive(receiveBuffer.prepare(0x1000),
                    [this](const boost::system::error_code& ec, size_t bytes) { handleReceiveICMP(ec, bytes); });
            } else {
                VLOG(DEBUG) << "Received all expected responses, cancelling receive timer";
                receiveTimer.cancel();
            }
        }
    }
}
}
