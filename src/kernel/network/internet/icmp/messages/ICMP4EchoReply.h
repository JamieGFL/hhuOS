//
// Created by hannes on 17.05.21.
//

#ifndef HHUOS_ICMP4ECHOREPLY_H
#define HHUOS_ICMP4ECHOREPLY_H

#include <kernel/network/internet/icmp/ICMP4Message.h>
#include <kernel/network/NetworkByteBlock.h>

class ICMP4EchoReply : public ICMP4Message {
private:
    typedef struct icmp4echoReply {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequenceNumber;
    } echoReplyMessage;

    echoReplyMessage echoReply;

    NetworkByteBlock *input;
    IP4Address *destinationAddress = nullptr;
    IP4Address *sourceAddress = nullptr;
public:
    //Sending constructor
    ICMP4EchoReply(uint16_t identifier, uint16_t sequenceNumber);

    //Receiving constructor
    ICMP4EchoReply(IP4Address *destinationAddress, IP4Address *sourceAddress, NetworkByteBlock *input);

    virtual ~ICMP4EchoReply();

    uint8_t copyDataTo(NetworkByteBlock *byteBlock) override;

    size_t getLengthInBytes() override;

    uint16_t getIdentifier();

    uint16_t getSequenceNumber();

    IP4Address *getSourceAddress() const;

    ICMP4MessageType getICMP4MessageType() override;

    uint8_t parseInput() override;

};


#endif //HHUOS_ICMP4ECHOREPLY_H
