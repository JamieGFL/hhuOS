//
// Created by hannes on 15.05.21.
//

#include "EthernetFrame.h"

EthernetFrame::EthernetFrame(EthernetAddress *destinationAddress, EthernetDataPart *ethernetDataPart) {
    this->destinationAddress = destinationAddress;
    this->etherType = ethernetDataPart->getEtherType();
    this->ethernetDataPart = ethernetDataPart;
}

void *EthernetFrame::getDataAsByteBlock() {
    return nullptr;
}

uint16_t EthernetFrame::getLength() {
    return 0;
}

EthernetFrame::EthernetFrame(void *packet, uint16_t length) {
//TODO:Implement parsing from incoming data
}

EtherType EthernetFrame::getEtherType() const {
    return etherType;
}

EthernetDataPart *EthernetFrame::getDataPart() const {
    return ethernetDataPart;
}

void EthernetFrame::setSourceAddress(EthernetAddress *ethernetAddress) {
    this->sourceAddress=ethernetAddress;
}
