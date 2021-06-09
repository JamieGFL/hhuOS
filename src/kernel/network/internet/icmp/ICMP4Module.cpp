//
// Created by hannes on 17.05.21.
//

#include <kernel/event/network/ICMP4ReceiveEvent.h>
#include <kernel/event/network/IP4SendEvent.h>
#include <lib/libc/printf.h>
#include <kernel/network/internet/icmp/messages/ICMP4Echo.h>
#include <kernel/event/network/ICMP4SendEvent.h>
#include "ICMP4Module.h"

namespace Kernel {
    ICMP4Module::ICMP4Module(NetworkEventBus *eventBus) : eventBus(eventBus) {}

    void ICMP4Module::onEvent(const Event &event) {
        if ((event.getType() == ICMP4SendEvent::TYPE)) {
            auto *destinationAddress = ((ICMP4SendEvent &) event).getDestinationAddress();
            auto *icmp4Message = ((ICMP4SendEvent &) event).getIcmp4Message();
            if (icmp4Message == nullptr) {
                log.error("Outgoing ICMP4 message was null, ignoring");
                delete destinationAddress;
                return;
            }
            if (destinationAddress == nullptr) {
                log.error("Destination address was null, discarding message");
                switch (icmp4Message->getICMP4MessageType()) {
                    case ICMP4Message::ICMP4MessageType::ECHO_REPLY:
                        delete (ICMP4EchoReply *) icmp4Message;
                        return;
                    case ICMP4Message::ICMP4MessageType::ECHO:
                        delete (ICMP4Echo *) icmp4Message;
                        return;
                        //All implemented messages are deleted now
                        //-> we can break here
                        //NOTE: Please add new ICMP4Messages here if implemented!
                    default:
                        break;
                }
            }
            eventBus->publish(
                    new IP4SendEvent(
                            new IP4Datagram(destinationAddress, icmp4Message)
                    )
            );
            //Address has been copied inside constructor, can be deleted now
            delete destinationAddress;
        }
        if ((event.getType() == ICMP4ReceiveEvent::TYPE)) {
            auto *input = ((ICMP4ReceiveEvent &) event).getInput();
            auto *inDatagram = ((ICMP4ReceiveEvent &) event).getDatagram();

            if (inDatagram == nullptr) {
                log.error("Incoming IP4Datagram was null, discarding input");
                delete input;
                return;
            }
            if (input== nullptr) {
                log.error("Incoming input was null, discarding datagram");
                delete inDatagram;
                return;
            }

            uint8_t typeByte = 0;
            input->read(&typeByte);
            //Decrement index by one
            //-> now it points to first message byte again!
            input->decreaseIndex(1);
            switch (ICMP4Message::parseByteAsICMP4MessageType(typeByte)) {
                case ICMP4Message::ICMP4MessageType::ECHO_REPLY: {
                    auto *echoReply = new ICMP4EchoReply();

                    if (echoReply->parseHeader(input)) {
                        log.error("Parsing ICMP4EchoReply failed, discarding");
                        delete echoReply;
                        break;
                    }

                    printf("ICMP4EchoReply received! SourceAddress: %s, Identifier: %d, SequenceNumber: %d",
                           inDatagram->getSourceAddress()->asChars(),
                           echoReply->getIdentifier(),
                           echoReply->getSequenceNumber()
                    );

                    delete echoReply;
                    break;
                }
                case ICMP4Message::ICMP4MessageType::ECHO: {
                    auto *echoRequest = new ICMP4Echo();
                    if (echoRequest->parseHeader(input)) {
                        log.error("Parsing ICMP4Echo failed, discarding");
                        delete echoRequest;
                        break;
                    }

                    //create and send reply
                    eventBus->publish(
                            new IP4SendEvent(
                                    new IP4Datagram(
                                            inDatagram->getSourceAddress(),
                                            echoRequest->buildEchoReply()
                                            )
                                    )
                            );
                    delete echoRequest;
                    break;
                }
                default:
                    log.info("ICMP4MessageType of incoming ICMP4Message not supported, discarding data");
                    break;
            }

            //We are done here, cleanup memory
            delete inDatagram;
            delete input;
            return;
        }
    }
}
