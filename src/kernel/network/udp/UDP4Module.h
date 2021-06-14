//
// Created by hannes on 17.05.21.
//

#ifndef HHUOS_UDP4MODULE_H
#define HHUOS_UDP4MODULE_H


#include <kernel/network/NetworkEventBus.h>
#include <kernel/log/Logger.h>
#include <kernel/network/udp/sockets/UDP4SocketController.h>
#include <kernel/event/network/UDP4SendEvent.h>
#include <kernel/event/network/IP4SendEvent.h>
#include <kernel/event/network/UDP4ReceiveEvent.h>

namespace Kernel {

    class UDP4Module : public Receiver {
    private:
        NetworkEventBus *eventBus = nullptr;
        Util::HashMap<UDP4Port *, UDP4SocketController *> *sockets;

    public:
        explicit UDP4Module(Kernel::NetworkEventBus *eventBus);

        Logger &log = Logger::get("UDP4Module");

        /**
         * Inherited method from Receiver.
         * This method is meant to be overridden and
         * implemented by this class.
         */
        void onEvent(const Event &event) override;

        uint8_t registerControllerFor(UDP4Port *destinationPort, UDP4SocketController *controller);

        uint8_t unregisterControllerFor(UDP4Port *destinationPort);
    };

}

#endif //HHUOS_UDP4MODULE_H
