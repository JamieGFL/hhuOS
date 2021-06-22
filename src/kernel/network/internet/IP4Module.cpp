//
// Created by hannes on 14.05.21.
//

#include <kernel/event/network/UDP4ReceiveEvent.h>
#include <kernel/event/network/IP4SendEvent.h>
#include <kernel/event/network/IP4ReceiveEvent.h>
#include <kernel/event/network/ICMP4ReceiveEvent.h>
#include <kernel/event/network/ARPReceiveEvent.h>
#include "IP4Module.h"

namespace Kernel {
    IP4Module::IP4Module(NetworkEventBus *eventBus) {
        this->eventBus = eventBus;
        this->routingModule = new IP4RoutingModule();
        this->interfaces = new Util::ArrayList<IP4Interface *>();
        accessLock = new Spinlock();
    }

    IP4Module::~IP4Module() {
        delete accessLock;
        if(interfaces== nullptr || routingModule== nullptr){
            return;
        }
        IP4Interface *toDelete;
        for (size_t i = 0; i < interfaces->size(); i++) {
            //Deleting while iterating is always dangerous
            //-> execute get() and remove() separately!
            toDelete = interfaces->get(i);
            routingModule->removeRoutesFor(toDelete);
            interfaces->remove(i);
            i--;
            delete toDelete;
        }
        delete interfaces;
        delete routingModule;
    }

    uint8_t IP4Module::collectIP4InterfaceAttributes(Util::ArrayList<String> *strings) {
        if (strings == nullptr || interfaces == nullptr || accessLock == nullptr) {
            log.error("Given Strings list, internal interface list or accessLock was null, "
                      "not unregistering device");
            return 1;
        }
        accessLock->acquire();
        for (IP4Interface *current:*interfaces) {
            strings->add(current->asString());
        }
        accessLock->release();
    }

    uint8_t IP4Module::collectIP4RouteAttributes(Util::ArrayList<String> *strings) {
        if (strings == nullptr || routingModule == nullptr) {
            log.error("Given String list or routing module was null, not collecting route attributes");
            return 1;
        }
        return routingModule->collectIP4RouteAttributes(strings);
    }

    uint8_t IP4Module::registerDevice(EthernetDevice *ethernetDevice, IP4Address *ip4Address, IP4Netmask *ip4Netmask) {
        if (ethernetDevice == nullptr || ip4Address == nullptr || ip4Netmask == nullptr) {
            log.error("At least one given parameter was null, not registering new device");
            return 1;
        }
        if (interfaces == nullptr || routingModule == nullptr || accessLock == nullptr) {
            log.error("Internal interface list, routing module or accessLock was null, not registering new device");
            return 1;
        }
        accessLock->acquire();
        for (IP4Interface *current:*interfaces) {
            if (current->connectedTo(ethernetDevice)) {
                log.error("Ethernet device already registered, not registering it again");
                return 1;
            }
        }

        auto *newInterface = new IP4Interface(eventBus, ethernetDevice, ip4Address, ip4Netmask);
        IP4Address *netAddress = nullptr;
        ip4Netmask->extractNetPart(&netAddress, ip4Address);

        if (routingModule->addDirectRouteFor(ip4Address, ip4Netmask, newInterface)) {
            log.error("Adding route for new IP4Interface failed, rollback");
            accessLock->release();
            delete newInterface;
            return 1;
        }
        interfaces->add(newInterface);
        accessLock->release();
        return 0;
    }

    uint8_t IP4Module::unregisterDevice(EthernetDevice *ethernetDevice) {
        if (ethernetDevice == nullptr) {
            log.error("Given device was null, not unregistering device");
            return 1;
        }
        if (interfaces == nullptr || routingModule == nullptr || accessLock== nullptr) {
            log.error("Internal interface list, routing module or accessLock was null, not unregistering device");
            return 1;
        }
        accessLock->acquire();
        IP4Interface *toDelete;
        for (size_t i = 0; i < interfaces->size(); i++) {
            if (interfaces->get(i)->connectedTo(ethernetDevice)) {
                toDelete = interfaces->get(i);
                routingModule->removeRoutesFor(toDelete);
                interfaces->remove(i);
                accessLock->release();
                delete toDelete;
                return 0;
            }
        }
        accessLock->release();
        //It's not an error if there's nothing to delete
        return 0;
    }

    void IP4Module::onEvent(const Event &event) {
        if ((event.getType() == IP4SendEvent::TYPE)) {
            IP4Datagram *datagram = ((IP4SendEvent &) event).getDatagram();
            if (datagram == nullptr) {
                log.error("Outgoing datagram was null, ignoring");
                return;
            }
            if (routingModule == nullptr) {
                log.error("Internal routing module was null, not sending anything");
                //delete on NULL objects simply does nothing!
                delete datagram;
                return;
            }
            if (datagram->getLengthInBytes() == 0) {
                log.error("Outgoing datagram was empty, discarding it");
                delete datagram;
                return;
            }
            if (routingModule->sendViaBestRoute(datagram)) {
                log.error("Sending failed, see syslog for more details");
                delete datagram;
            }
            //Datagram will be deleted in EthernetModule after sending
            //-> no delete here!
            return;
        }
        if ((event.getType() == IP4ReceiveEvent::TYPE)) {
            auto *ip4Header = ((IP4ReceiveEvent &) event).getHeader();
            auto *input = ((IP4ReceiveEvent &) event).getInput();

            if (ip4Header == nullptr) {
                log.error("Incoming IP4Datagram was null, discarding input");
                delete input;
                return;
            }
            if (input == nullptr) {
                log.error("Incoming input was null, discarding datagram");
                delete ip4Header;
                return;
            }
            if (!ip4Header->headerValid()) {
                log.error("Incoming IP4Header corrupted, discarding datagram");
                delete ip4Header;
                delete input;
                return;
            }
            switch (ip4Header->getIP4ProtocolType()) {
                case IP4DataPart::IP4ProtocolType::ICMP4: {
                    if (input->bytesRemaining() == 0) {
                        log.error("Incoming ICMP4Message was empty, discarding");
                        delete ip4Header;
                        delete input;
                        return;
                    }
                    //We don't care about all the possible ICMP4 messages here
                    //-> send full input to ICMP4Module for parsing and processing
                    eventBus->publish(new ICMP4ReceiveEvent(ip4Header, input));

                    //We need input AND ip4Header in next module
                    //-> don't delete anything here!
                    return;
                }
                case IP4DataPart::IP4ProtocolType::UDP: {
                    auto *udp4Header = new UDP4Header();
                    if (udp4Header->parse(input)) {
                        log.error("Could not assemble UDP header, discarding data");
                        //udp4Header is not part of ip4Datagram here
                        //-> we need to delete it separately!
                        delete udp4Header;
                        delete ip4Header;
                        delete input;
                        return;
                    }
                    eventBus->publish(new UDP4ReceiveEvent(ip4Header, udp4Header, input));
                    //We need input AND both headers in next module
                    //-> don't delete anything here!
                    return;
                }
                default:
                    log.error("IP4ProtocolType of incoming IP4Datagram not supported, discarding");
                    delete ip4Header;
                    delete input;
                    return;
            }
        }
        if ((event.getType() == ARPReceiveEvent::TYPE)) {
            //Each IP4 interface has its own ARP module (RFC1180 page 12 middle)
            //-> we need to find the destination interface first when processing ARP messages!
            auto *arpMessage = ((ARPReceiveEvent &) event).getARPMessage();
            EthernetDataPart::EtherType arpProtocolType =
                    EthernetDataPart::parseIntAsEtherType(
                            arpMessage->getProtocolType()
                    );
            if (arpProtocolType != EthernetDataPart::EtherType::IP4) {
                log.error("Incoming ARP message was not for IPv4, discarding");
                delete arpMessage;
                return;
            }
            if (interfaces == nullptr || accessLock== nullptr) {
                log.error("Internal interface list or accessLock not initialized, discarding ARP message");
                delete arpMessage;
                return;
            }

            auto *destinationAddress =
                    new EthernetAddress(arpMessage->getTargetHardwareAddress());
            accessLock->acquire();
            for (IP4Interface *current:*interfaces) {
                if (current->connectedTo(destinationAddress) && current->notify(arpMessage)) {
                    log.error("Processing ARP message failed, see syslog for more details");
                }
            }
            accessLock->release();
            delete destinationAddress;
            delete arpMessage;
            return;
        }
    }
}
