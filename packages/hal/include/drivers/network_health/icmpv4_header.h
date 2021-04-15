#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <type_traits>
namespace hal {
namespace details {

    /// See https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
    enum struct ICMPV4MessageType : uint8_t {
        /// https://en.wikipedia.org/wiki/Ping_(networking_utility)#ICMP_packet
        EchoReply = 0,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Destination_unreachable
        DestinationUnreachable = 3,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Source_quench
        SourceQuench = 4,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Redirect
        Redirect = 5,
        /// https://en.wikipedia.org/wiki/Ping_(networking_utility)#ICMP_packet
        EchoRequest = 8,
        /// https://en.wikipedia.org/wiki/ICMP_Router_Discovery_Protocol
        RouterAdvertisement = 9,
        /// https://en.wikipedia.org/wiki/ICMP_Router_Discovery_Protocol
        RouterSolicitation = 10,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Time_exceeded
        TimeExceeded = 11,
        ParameterProblem = 12,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Timestamp
        TimestampRequest = 13,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Timestamp_request
        TimestampReply = 14,

        InfoRequest = 15,
        InfoReply = 16,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Address_mask_request
        AddressRequest = 17,
        /// https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Address_mask_reply
        AddressReply = 18
    };

    /// See https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
    ///
    /// This is a customized version of the header; in addition to the basic ICMP header fields, it adds
    /// sequence and identifier which inspect bytes 4-8 of the header as if we were looking at either an
    /// echo request or echo reply.
    ///
    class ICMPV4Header {
    public:
        constexpr ICMPV4Header()
            : rep{ { 0 } } {}

        constexpr ICMPV4MessageType type() const { return static_cast<ICMPV4MessageType>(rep.at(0)); }

        inline void type(ICMPV4MessageType t) { rep.at(0) = static_cast<std::underlying_type<ICMPV4MessageType>::type>(t); }

        constexpr uint8_t code() const { return rep.at(1); }

        inline void code(uint8_t c) { rep.at(1) = c; }

        constexpr uint16_t checksum() const { return decode16(2, 3); }

        inline void checksum(uint16_t c) { encode16(2, 3, c); }

        constexpr uint16_t identifier() const { return decode16(4, 5); }

        inline void identifier(uint16_t i) { encode16(4, 5, i); }

        constexpr uint16_t sequence() const { return decode16(6, 7); }

        inline void sequence(uint16_t s) { encode16(6, 7, s); }

        inline friend std::istream& operator>>(std::istream& in, ICMPV4Header& header) {
            return in.read(reinterpret_cast<char*>(header.rep.data()), ICMPV4Header::headerSize);
        }

        inline friend std::ostream& operator<<(std::ostream& out, const ICMPV4Header& header) {
            return out.write(reinterpret_cast<const char*>(header.rep.data()), ICMPV4Header::headerSize);
        }

        /// Lifted from http://www.boost.org/doc/libs/1_64_0/doc/html/boost_asio/example/cpp03/icmp/icmp_header.hpp
        ///
        /// \tparam ITERATOR
        /// \param header IMCPV4 header
        /// \param begin payload begin
        /// \param end payload end
        /// \return header with checksum set
        template <typename ITERATOR> static ICMPV4Header& checksum(ICMPV4Header& header, ITERATOR begin, ITERATOR end) {
            using traits = std::iterator_traits<ITERATOR>;
            static_assert(std::is_same<typename traits::value_type, uint8_t>::type::value, "Only handle containers of uint8_t for now");

            // Compute header portion of checksum. Note: when computing the checksum, treat the checksum field of the header
            // as 0, which is why we skip header.decode(2, 3) in the sum.
            uint32_t sum = header.decode16(0, 1) + header.decode16(4, 5) + header.decode16(6, 7);
            ITERATOR body_iterator = begin;

            while (body_iterator != end) {
                sum += (*body_iterator++) << 8;
                if (body_iterator != end) {
                    sum += (*body_iterator++);
                }
            }

            sum = (sum >> 16) + (sum + 0xffff);
            sum += sum >> 16;
            header.checksum(static_cast<uint16_t>(~sum));
            return header;
        }

    private:
        static constexpr size_t headerSize = 8;
        std::array<uint8_t, headerSize> rep;

        constexpr uint16_t decode16(size_t a, size_t b) const { return (rep.at(a) << 8) | rep.at(b); }

        inline void encode16(size_t a, size_t b, uint16_t v) {
            rep.at(a) = (v >> 8) & 0xff;
            rep.at(b) = v & 0xff;
        }
    };
}
}
