//
// Created by hannes on 02.06.21.
//

#ifndef HHUOS_GENERICICMP4MESSAGE_H
#define HHUOS_GENERICICMP4MESSAGE_H


#include <kernel/network/internet/icmp/ICMP4Message.h>
#include <kernel/network/internet/addressing/IP4Address.h>
#include "ICMP4Echo.h"

class GenericICMP4Message : public ICMP4Message {
private:
    IP4Address *destinationAddress = nullptr;
    IP4Address *sourceAddress = nullptr;
    NetworkByteBlock *input = nullptr;
    ICMP4MessageType messageType = ICMP4MessageType::INVALID;
public:
    GenericICMP4Message(IP4Address *destinationAddress, IP4Address *sourceAddress, NetworkByteBlock *input);

    GenericICMP4Message(IP4Address *destinationAddress, IP4Address *sourceAddress, ICMP4Message *message);

    uint8_t parseInput() override;

    ICMP4MessageType getICMP4MessageType() override;

    virtual ~GenericICMP4Message();

    IP4ProtocolType getIP4ProtocolType() override;

    uint8_t copyDataTo(NetworkByteBlock *byteBlock) override;

    size_t getLengthInBytes() override;

    ICMP4Echo *buildICMP4EchoWithInput();

    ICMP4EchoReply *buildICMP4EchoReplyWithInput();
};


#endif //HHUOS_GENERICICMP4MESSAGE_H
