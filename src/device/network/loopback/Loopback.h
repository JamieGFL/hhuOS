//
// Created by hannes on 19.04.21.
//

#ifndef HHUOS_LOOPBACK_H
#define HHUOS_LOOPBACK_H

#include <kernel/event/network/ReceiveEvent.h>
#include <kernel/service/EventBus.h>
#include <kernel/core/System.h>
#include <kernel/log/Logger.h>

#include <lib/util/SmartPointer.h>

#include <device/network/NetworkDevice.h>

/**
 *
 */
class Loopback : public NetworkDevice {

private:
    IP4Address *ip4Address = nullptr;
    IP4Netmask *ip4Netmask = nullptr;

    /**
     * A logger to provide logging information on the kernel log.
     */
    Kernel::Logger &log = Kernel::Logger::get("Loopback");

    Kernel::EventBus *eventBus = nullptr;

public:
    Loopback();

    IP4Address *getIp4Address() const;

    IP4Netmask *getIp4Netmask() const;

    /**
     * Overriding function from NetworkDevice.
     */
    void sendPacket(void *address, uint16_t length) override;

    /**
     * Overriding function from NetworkDevice.
     */
    void getMacAddress(uint8_t *buf) override;

    void trigger(Kernel::InterruptFrame &frame) override;
};

#endif //HHUOS_LOOPBACK_H
