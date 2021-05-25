//
// Created by hannes on 14.05.21.
//

#ifndef HHUOS_IP4MODULE_H
#define HHUOS_IP4MODULE_H

#include <kernel/event/network/IP4SendEvent.h>
#include <kernel/core/System.h>
#include <kernel/service/EventBus.h>
#include <kernel/event/network/ARPReceiveEvent.h>
#include <kernel/event/network/IP4ReceiveEvent.h>
#include <kernel/event/network/ICMP4ReceiveEvent.h>
#include <kernel/network/internet/icmp/messages/ICMP4DestinationUnreachable.h>
#include <kernel/event/network/UDPReceiveEvent.h>
#include "IP4Module.h"
#include "IP4Interface.h"
#include <kernel/event/Receiver.h>
#include <kernel/log/Logger.h>
#include <kernel/network/internet/routing/IP4RoutingModule.h>
#include <kernel/network/internet/arp/ARPModule.h>
#include "IP4Interface.h"

namespace Kernel {

    class IP4Module final : public Receiver {
    private:
        Kernel::EventBus *eventBus;
        IP4RoutingModule *routingModule;
        Util::ArrayList<IP4Interface *> *interfaces;

    public:

        IP4Module(Kernel::EventBus *eventBus);

        ~IP4Module() override;

        /**
         * A logger to provide information on the kernel log.
         */
        Logger &log = Logger::get("IP4Module");


/**
     * Inherited method from Receiver.
     * This method is meant to be overridden and
     * implemented by this class.
     */
        void onEvent(const Event &event) override;

        void registerDevice(EthernetDevice *device);

        void unregisterDevice(EthernetDevice *device);
    };

}


#endif //HHUOS_IP4MODULE_H
