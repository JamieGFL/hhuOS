//
// Created by hannes on 17.05.21.
//

#ifndef HHUOS_UDP4DATAGRAM_H
#define HHUOS_UDP4DATAGRAM_H

//TODO: Verify values with RFC Sources
#define UDP4DATAPART_MAX_LENGTH 1500
#define UDP4HEADER_MAX_LENGTH 0xffff

#include <kernel/network/internet/IP4DataPart.h>
#include <kernel/network/udp/sockets/UDP4Port.h>

class UDP4Datagram final : public IP4DataPart {
private:
    typedef struct udp4Header {
        uint16_t sourcePort = 0;
        uint16_t destinationPort = 0;
        uint16_t length = 0;
        uint16_t checksum = 0;
    } header_t;

    header_t header;
    NetworkByteBlock *dataBytes = nullptr;

    UDP4Port *sourcePort = nullptr;
    UDP4Port *destinationPort = nullptr;

public:
    UDP4Datagram(UDP4Port *sourcePort, UDP4Port *destinationPort, uint8_t *outgoingBytes, size_t length);

    UDP4Datagram() = default;

    ~UDP4Datagram();

    uint8_t copyTo(NetworkByteBlock *output) override;

    size_t getLengthInBytes() override;

    IP4ProtocolType getIP4ProtocolType() override;

    uint8_t parseHeader(NetworkByteBlock *input) override;

    [[nodiscard]] UDP4Port *getDestinationPort() const;
};


#endif //HHUOS_UDP4DATAGRAM_H
