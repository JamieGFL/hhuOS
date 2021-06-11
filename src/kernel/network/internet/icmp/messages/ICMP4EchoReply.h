//
// Created by hannes on 17.05.21.
//

#ifndef HHUOS_ICMP4ECHOREPLY_H
#define HHUOS_ICMP4ECHOREPLY_H

#include <kernel/network/internet/icmp/ICMP4Message.h>
#include <kernel/network/NetworkByteBlock.h>

class ICMP4EchoReply final : public ICMP4Message {
private:
    typedef struct icmp4echoReply {
        uint8_t type = 0; //0 for echo reply, 8 for echo (RFC792)
        uint8_t code = 0;
        uint16_t checksum = 0;
        uint16_t identifier = 0;
        uint16_t sequenceNumber = 0;
    } echoReplyMessage;

    echoReplyMessage echoReply;

public:
    //Sending constructor
    ICMP4EchoReply(uint16_t identifier, uint16_t sequenceNumber);

    ICMP4EchoReply() = default;

    ~ICMP4EchoReply() = default;

    uint8_t copyTo(NetworkByteBlock *output) override;

    size_t getLengthInBytes() override;

    ICMP4MessageType getICMP4MessageType() override;

    uint8_t parseHeader(NetworkByteBlock *input) override;

    IP4ProtocolType getIP4ProtocolType() override;

    [[nodiscard]] uint16_t getIdentifier() const;

    [[nodiscard]] uint16_t getSequenceNumber() const;
};


#endif //HHUOS_ICMP4ECHOREPLY_H
