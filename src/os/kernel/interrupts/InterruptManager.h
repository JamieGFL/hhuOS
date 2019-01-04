#ifndef HHUOS_IODEVICEMANAGER_H
#define HHUOS_IODEVICEMANAGER_H

#include <kernel/threads/Thread.h>
#include <lib/lock/Spinlock.h>
#include <lib/util/ArrayList.h>

class InterruptHandler;

class InterruptManager : public Thread {

private:

    Util::ArrayList<InterruptHandler*> interruptHandler;

    Spinlock lock;

public:

    InterruptManager() = default;

    InterruptManager(const InterruptManager &copy) = delete;

    ~InterruptManager() override = default;

    static InterruptManager& getInstance();

    void registerInterruptHandler(InterruptHandler *device);

    void deregisterInterruptHandler(InterruptHandler *device);

    void handleBiosCallReturn();

    void run() override;

};


#endif
