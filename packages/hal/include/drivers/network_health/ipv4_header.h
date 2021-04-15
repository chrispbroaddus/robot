#pragma once

#include <boost/asio/ip/address_v4.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace hal {
namespace details {

    /// Represent an IPv4 packet header.
    ///
    /// Based on
    /// http://www.boost.org/doc/libs/1_63_0/doc/html/boost_asio/example/cpp03/icmp/ipv4_header.hpp
    class IPV4Header {
    public:
        constexpr IPV4Header()
            : rep{ { 0 } } {}

        ///
        /// Should be 4 for IPv4
        ///
        constexpr uint8_t version() const { return (rep[0] >> 4) & 0xf; }

        /// Get the header length in bytes
        constexpr uint8_t internetHeaderLength() const { return (rep[0] & 0xf) * 4; }

        /// https://en.wikipedia.org/wiki/Differentiated_services
        constexpr uint8_t differentiatedServiceCodePoint() const { return (rep[1] >> 2) & 0x3f; }

        /// https://en.wikipedia.org/wiki/Explicit_Congestion_Notification
        constexpr uint8_t explicitCongestionNotification() const { return (rep[1]) & 0x3; }

        /// Total packet length in bytes, including header and payload
        constexpr uint16_t packetLength() const { return decode16(2, 3); }

        /// Identification
        constexpr uint16_t identification() const { return decode16(4, 5); }

        /// Don't fragment flag
        constexpr bool dontFragment() const { return rep.at(6) & 0x40; }

        /// More fragments flag
        constexpr bool moreFragments() const { return rep.at(6) & 0x20; }

        /// Fragment offset as measured in 8 byte blocks.
        constexpr uint16_t fragmentOffset() const { return decode16(6, 7) & 0x1fff; }

        // Time-to-live (hops)
        constexpr uint8_t timeToLive() const { return rep.at(8); }

        /// Protocol
        constexpr uint8_t protocol() const { return rep.at(9); }

        /// Source address
        inline boost::asio::ip::address_v4 sourceAddress() const {
            return boost::asio::ip::address_v4(
                boost::asio::ip::address_v4::bytes_type{ { rep.at(12), rep.at(13), rep.at(14), rep.at(15) } });
        }

        /// Destination address
        inline boost::asio::ip::address_v4 destinationAddress() const {
            return boost::asio::ip::address_v4(
                boost::asio::ip::address_v4::bytes_type{ { rep.at(16), rep.at(17), rep.at(18), rep.at(19) } });
        }

        /// @note
        /// Assumes binary I/O on the provided stream
        ///
        /// @note
        /// If we see junk in the stream, calls setstate(std::ios::failbit).
        ///
        /// @param in Input stream
        /// @param header Header
        /// @return input stream
        inline friend std::istream& operator>>(std::istream& in, IPV4Header& header) {
            in.read(reinterpret_cast<char*>(header.rep.data()), minimumPacketHeaderSize);

            if (header.version() != expectedIpVersion) {
                in.setstate(std::ios::failbit);
            } else {
                const unsigned bytesRemaining = header.internetHeaderLength() - minimumPacketHeaderSize;

                // Note: exploits overflow behavior of computing bytesRemaining using
                // unsigned quantities above
                if (bytesRemaining > (maximumPacketHeaderSize - minimumPacketHeaderSize)) {
                    in.setstate(std::ios::failbit);
                } else {
                    in.read(reinterpret_cast<char*>(header.rep.data()) + minimumPacketHeaderSize, bytesRemaining);
                }
            }

            return in;
        }

        template <typename ITERATOR> static inline IPV4Header fromIterator(ITERATOR begin, ITERATOR end) {
            using traits = std::iterator_traits<ITERATOR>;
            static_assert(std::is_same<typename traits::value_type, uint8_t>::type::value, "Only handle containers of uint8_t for now");

            const auto distance = std::distance(begin, end);

            if (distance < static_cast<signed>(minimumPacketHeaderSize)) {
                throw std::invalid_argument("Iterator range is too small (must be at least 20)");
            }

            if (distance > static_cast<signed>(maximumPacketHeaderSize)) {
                throw std::invalid_argument("Iterator range is too large (must be no more than 60)");
            }

            IPV4Header result;
            std::copy(begin, end, result.rep.begin());

            return result;
        }

    private:
        static constexpr uint8_t expectedIpVersion = 4;
        static constexpr size_t minimumPacketHeaderSize = 20;
        static constexpr size_t maximumPacketHeaderSize = 60;

        constexpr uint16_t decode16(size_t idx1, size_t idx2) const { return (rep.at(idx1) << 8) | rep.at(idx2); }

        std::array<uint8_t, maximumPacketHeaderSize> rep;
    };
}
}
