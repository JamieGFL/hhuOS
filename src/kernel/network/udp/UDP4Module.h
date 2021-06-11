//
// Created by hannes on 17.05.21.
//

#ifndef HHUOS_UDP4MODULE_H
#define HHUOS_UDP4MODULE_H

#include <kernel/log/Logger.h>
#include <kernel/event/Event.h>
#include <kernel/event/Receiver.h>
#include <kernel/network/NetworkEventBus.h>

namespace Kernel {

    class UDP4Module : public Receiver {
    private:
        NetworkEventBus *eventBus;

    public:
        explicit UDP4Module(Kernel::NetworkEventBus *eventBus);

        Logger &log = Logger::get("UDP4Module");

        /**
         * Inherited method from Receiver.
         * This method is meant to be overridden and
         * implemented by this class.
         */
        void onEvent(const Event &event) override;
    };

}

#endif //HHUOS_UDP4MODULE_H
