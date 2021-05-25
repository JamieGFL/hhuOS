//
// Created by hannes on 15.05.21.
//

#include "IP4Route.h"

IP4Route::IP4Route(IP4Address *netAddress, IP4Netmask *netMask, IP4Address *nextHop,
                           IP4Interface *outInterface) {
    this->netAddress = netAddress;
    this->netMask=netMask;
    this->nextHopAddress=nextHop;
    this->outInterface=outInterface;
}

IP4Route::IP4Route(IP4Address *netAddress, IP4Netmask *netMask, IP4Interface *outInterface) {
    this->netAddress = netAddress;
    this->netMask=netMask;
    this->nextHopAddress= nullptr;
    this->outInterface=outInterface;
}

IP4Address *IP4Route::getNetAddress() const {
    return netAddress;
}

IP4Netmask *IP4Route::getNetMask() const {
    return netMask;
}

IP4Address *IP4Route::getNextHopAddress() const {
    return nextHopAddress;
}

IP4Interface *IP4Route::getOutInterface() const {
    return outInterface;
}