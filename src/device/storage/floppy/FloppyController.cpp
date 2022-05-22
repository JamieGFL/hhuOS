/*
 * Copyright (C) 2018-2022 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "kernel/interrupt/InterruptDispatcher.h"
#include "FloppyController.h"
#include "FloppyDevice.h"
#include "device/time/Cmos.h"
#include "kernel/service/MemoryService.h"
#include "kernel/system/System.h"
#include "device/interrupt/Pic.h"
#include "device/cpu/Cpu.h"
#include "kernel/service/StorageService.h"

namespace Device::Storage {

Kernel::Logger FloppyController::log = Kernel::Logger::get("Floppy");

bool FloppyController::isAvailable() {
    Cpu::disableInterrupts();
    Cmos::disableNmi();
    auto cmosValue = Cmos::read(0x10);
    Cmos::enableNmi();
    Cpu::enableInterrupts();

    return cmosValue != 0x00;
}

FloppyController::FloppyController() :
        dmaMemory(Kernel::System::getService<Kernel::MemoryService>().allocateLowerMemory(512)),
        timeService(Kernel::System::getService<Kernel::TimeService>()),
        statusRegisterA(IO_BASE_ADDRESS + 0), statusRegisterB(IO_BASE_ADDRESS + 1), digitalOutputRegister(IO_BASE_ADDRESS + 2),
        tapeDriveRegister(IO_BASE_ADDRESS + 3), mainStatusRegister(IO_BASE_ADDRESS + 4), dataRateSelectRegister(IO_BASE_ADDRESS + 4),
        fifoRegister(IO_BASE_ADDRESS + 5), digitalInputRegister(IO_BASE_ADDRESS + 7), configControlRegister(IO_BASE_ADDRESS + 7) {}

FloppyController::~FloppyController() {
    Kernel::System::getService<Kernel::MemoryService>().freeLowerMemory(dmaMemory);
}

bool FloppyController::isBusy() {
    return (mainStatusRegister.readByte() & 0x10u) == 0x10;
}

bool FloppyController::isFifoBufferReady() {
    return (mainStatusRegister.readByte() & 0x80u) == 0x80;
}

void FloppyController::initializeAvailableDrives() {
    if (!isAvailable()) {
        log.info("No floppy drives available");
    }

    Cpu::disableInterrupts();
    Cmos::disableNmi();
    uint8_t driveInfo = Cmos::read(0x10);
    Cmos::enableNmi();
    Cpu::enableInterrupts();

    plugin();

    auto primaryDriveType = static_cast<DriveType>(driveInfo >> 4u);
    auto secondaryDriveType = static_cast<DriveType>(driveInfo & static_cast<uint8_t>(0xfu));

    if (primaryDriveType != DriveType::DRIVE_TYPE_NONE && primaryDriveType != DriveType::DRIVE_TYPE_UNKNOWN_1 && primaryDriveType != DriveType::DRIVE_TYPE_UNKNOWN_2) {
        log.info("Found primary floppy drive");

        auto *device = new FloppyDevice(*this, 0, primaryDriveType);
        auto success = resetDrive(*device);

        if (success) {
            Kernel::System::getService<Kernel::StorageService>().registerDevice(device, DEVICE_CLASS);
        } else {
            log.error("Unable to initialize primary floppy drive");
            delete device;
        }
    }

    if (secondaryDriveType != DriveType::DRIVE_TYPE_NONE && secondaryDriveType != DriveType::DRIVE_TYPE_UNKNOWN_1 && secondaryDriveType != DriveType::DRIVE_TYPE_UNKNOWN_2) {
        log.info("Found secondary floppy drive");

        auto *device = new FloppyDevice(*this, 1, secondaryDriveType);
        auto success = resetDrive(*device);

        if (success) {
            Kernel::System::getService<Kernel::StorageService>().registerDevice(device, DEVICE_CLASS);
        } else {
            log.error("Unable to initialize secondary floppy drive");
            delete device;
        }
    }
}

void FloppyController::writeFifoByte(uint8_t command) {
    uint32_t timeout = 0;
    while (timeout < TIMEOUT) {
        if (isFifoBufferReady()) {
            fifoRegister.writeByte(command);
            return;
        }

        timeService.wait(Util::Time::Timestamp(0, 10000000));
        timeout += 10;
    }

    log.error("Timeout while issuing write command");
}

uint8_t FloppyController::readFifoByte() {
    uint32_t timeout = 0;
    while (timeout < TIMEOUT) {
        if (isFifoBufferReady()) {
            return fifoRegister.readByte();
        }

        timeService.wait(Util::Time::Timestamp(0, 10000000));
        timeout += 10;
    }

    log.error("Timeout while reading data from FIFO-buffer");
    return 0;
}

FloppyController::SenseInterruptState FloppyController::senseInterrupt() {
    writeFifoByte(SENSE_INTERRUPT);
    return {
        readFifoByte(),
        readFifoByte()
    };
}

FloppyController::CommandStatus FloppyController::readCommandStatus() {
    return {
        readFifoByte(),
        readFifoByte(),
        readFifoByte(),
        readFifoByte(),
        readFifoByte(),
        readFifoByte(),
        readFifoByte()
    };
}

void FloppyController::setMotorState(FloppyDevice &device, FloppyController::MotorState desiredState) {
    if (desiredState == WAIT) {
        return;
    } else if (desiredState == ON) {
        if (device.getMotorState() == ON) {
            return;
        } else if (device.getMotorState() == WAIT) {
            device.setMotorState(ON);
            return;
        }

        digitalOutputRegister.writeByte(device.getDriveNumber() | static_cast<uint8_t>(0x1c)); // Turn on the floppy motor
        device.setMotorState(ON);
    } else if (desiredState == OFF) {
        if (device.getMotorState() == OFF || device.getMotorState() == WAIT) {
            return;
        }

        device.setMotorState(WAIT);
    }
}

void FloppyController::killMotor(FloppyDevice &device) {
    digitalOutputRegister.writeByte(device.getDriveNumber() | static_cast<uint8_t>(0x0c));
    device.setMotorState(OFF);
}

bool FloppyController::checkMedia(FloppyDevice &device) {
    setMotorState(device, ON);
    // Check disk change indicator flag
    if (digitalInputRegister.readByte() & 0x80) {
        // If the bit is set, a disk change has occurred and we need to check, if a disk is present
        // First we let the drive seek a cylinder other than 0 and afterwards we let it seek back to 0
        seek(device, 37, 0);
        seek(device, 0, 0);

        // If the bit is still set, there is no disk present in the drive
        // Otherwise, a disk swap has occurred
        return (digitalInputRegister.readByte() & 0x80) == 0;
    } else {
        // If bit is not set, a disk is present
        return true;
    }
}

bool FloppyController::resetDrive(FloppyDevice &device) {
    log.info("Resetting drive %u", device.getDriveNumber());
    receivedInterrupt = false;

    digitalOutputRegister.writeByte(0x00); // Disable controller;
    digitalOutputRegister.writeByte(device.getDriveNumber() | static_cast<uint8_t>(0x0c)); // Enable controller;

    uint32_t timeout = 0;
    while (!receivedInterrupt) {
        timeService.wait(Util::Time::Timestamp(0, 10000000));
        timeout += 10;

        if (timeout > TIMEOUT) {
            log.error("Timeout while resetting drive");
            return false;
        }
    }

    senseInterrupt();

    switch (device.getDriveType()) {
        case DRIVE_TYPE_360KB_5_25 :
            configControlRegister.writeByte(0x02);
            break;
        case DRIVE_TYPE_1200KB_5_25 :
            configControlRegister.writeByte(0x00);
            break;
        case DRIVE_TYPE_720KB_3_5 :
            configControlRegister.writeByte(0x01);
            break;
        case DRIVE_TYPE_1440KB_3_5 :
            configControlRegister.writeByte(0x00);
            break;
        case DRIVE_TYPE_2880KB_3_5 :
            configControlRegister.writeByte(0x00);
            break;
        default :
            configControlRegister.writeByte(0x00);
            break;
    }

    writeFifoByte(SPECIFY);
    writeFifoByte(0xdf);
    writeFifoByte(0x02);

    bool success = calibrateDrive(device);
    if (!success) {
        log.error("Failed to reset drive %u", device.getDriveNumber());
    }

    return success;
}

bool FloppyController::calibrateDrive(FloppyDevice &device) {
    log.info("Calibrating drive %u", device.getDriveNumber());
    setMotorState(device, ON);

    for (uint8_t i = 0; i < RETRY_COUNT; i++) {
        receivedInterrupt = false;

        writeFifoByte(RECALIBRATE);
        writeFifoByte(device.getDriveNumber());

        uint32_t timeout = 0;
        while (!receivedInterrupt && timeout < TIMEOUT) {
            timeService.wait(Util::Time::Timestamp(0, 10000000));
            timeout += 10;
        }

        if (!receivedInterrupt) {
            continue;
        }

        SenseInterruptState interruptState = senseInterrupt();
        if ((interruptState.statusRegister0 & 0xc0u) == 0xc0) {
            continue;
        }

        if (interruptState.currentCylinder == 0) {
            setMotorState(device, OFF);
            return true;
        }
    }

    log.error("Failed to calibrate drive %u", device.getDriveNumber());
    setMotorState(device, OFF);
    return false;
}

bool FloppyController::seek(FloppyDevice &device, uint8_t cylinder, uint8_t head) {
    setMotorState(device, ON);
    for (uint8_t i = 0; i < RETRY_COUNT; i++) {
        receivedInterrupt = false;

        writeFifoByte(SEEK);
        writeFifoByte(device.getDriveNumber() | static_cast<uint8_t>(head << 2u));
        writeFifoByte(cylinder);
        uint32_t timeout = 0;

        while (!receivedInterrupt && timeout < TIMEOUT) {
            timeService.wait(Util::Time::Timestamp(0, 10000000));
            timeout += 10;
        }

        if (!receivedInterrupt) {
            continue;
        }

        SenseInterruptState interruptState = senseInterrupt();
        if ((interruptState.statusRegister0 & 0xc0u) == 0xc0) {
            continue;
        }

        if (interruptState.currentCylinder == cylinder) {
            setMotorState(device, OFF);
            return true;
        }
    }

    log.error("Failed to seek on drive %u: Did not find cylinder %u", device.getDriveNumber(), cylinder);
    setMotorState(device, OFF);
    return false;
}

void FloppyController::plugin() {
    Kernel::InterruptDispatcher::getInstance().assign(Kernel::InterruptDispatcher::FLOPPY, *this);
    Pic::getInstance().allow(Pic::Interrupt::FLOPPY);
}

void FloppyController::trigger(Kernel::InterruptFrame &frame) {
    receivedInterrupt = true;
}

void FloppyController::prepareDma(FloppyDevice &device, Isa::TransferMode transferMode) {
    auto physicalAddress = reinterpret_cast<uint32_t>(Kernel::System::getService<Kernel::MemoryService>().getPhysicalAddress(dmaMemory))
            + (reinterpret_cast<uint32_t>(dmaMemory) % Util::Memory::PAGESIZE);

    Isa::selectChannel(2);
    Isa::setAddress(2, physicalAddress);
    Isa::setCount(2, static_cast<uint16_t>(device.getSectorSize() - 1));
    Isa::setMode(2, transferMode, false, false, Isa::SINGLE_TRANSFER);
    Isa::deselectChannel(2);
}

bool FloppyController::readSector(FloppyDevice &device, uint8_t *buff, uint8_t cylinder, uint8_t head, uint8_t sector) {
    if (!seek(device, cylinder, head)) {
        return false;
    }

    setMotorState(device, ON);
    for (uint8_t i = 0; i < RETRY_COUNT; i++) {
        receivedInterrupt = false;
        prepareDma(device, Isa::WRITE);

        writeFifoByte(READ_DATA | MULTITRACK | MFM);
        writeFifoByte(device.getDriveNumber() | (head << 2u));
        writeFifoByte(cylinder);
        writeFifoByte(device.getDriveNumber() | (head << 2u));
        writeFifoByte(sector);
        writeFifoByte(2);
        writeFifoByte(device.getSectorsPerTrack());
        writeFifoByte(device.getGapLength());
        writeFifoByte(0xff);

        uint32_t timeout = 0;

        while (!receivedInterrupt && timeout < TIMEOUT) {
            timeService.wait(Util::Time::Timestamp(0, 10000000));
            timeout += 10;
        }

        if (!receivedInterrupt) {
            if (!handleReadWriteError(device, cylinder, head)) {
                log.error("Timeout while reading a sector on drive %u", device.getDriveNumber());
                return false;
            }

            continue;
        }

        CommandStatus status = readCommandStatus();
        if ((status.statusRegister0 & 0xc0u) != 0) {
            if (!handleReadWriteError(device, cylinder, head)) {
                log.error("Failed to read a sector on drive %u", device.getDriveNumber());
                return false;
            }

            continue;
        }

        auto sourceAddress = Util::Memory::Address<uint32_t>(dmaMemory);
        auto targetAddress = Util::Memory::Address<uint32_t>(buff);
        targetAddress.copyRange(sourceAddress, device.getSectorSize());

        setMotorState(device, OFF);
        return true;
    }

    setMotorState(device, OFF);
    log.error("Failed to read a sector on drive %u", device.getDriveNumber());
    return false;
}

bool FloppyController::writeSector(FloppyDevice &device, const uint8_t *buff, uint8_t cylinder, uint8_t head, uint8_t sector) {
    auto sourceAddress = Util::Memory::Address<uint32_t>(buff);
    auto targetAddress = Util::Memory::Address<uint32_t>(dmaMemory);
    targetAddress.copyRange(sourceAddress, device.getSectorSize());

    if (!seek(device, cylinder, head)) {
        return false;
    }

    setMotorState(device, ON);

    for (uint8_t i = 0; i < RETRY_COUNT; i++) {
        receivedInterrupt = false;

        prepareDma(device, Isa::READ);

        writeFifoByte(WRITE_DATA | MULTITRACK | MFM);
        writeFifoByte(device.getDriveNumber() | (head << 2u));
        writeFifoByte(cylinder);
        writeFifoByte(device.getDriveNumber() | (head << 2u));
        writeFifoByte(sector);
        writeFifoByte(2);
        writeFifoByte(device.getSectorsPerTrack());
        writeFifoByte(device.getGapLength());
        writeFifoByte(0xff);

        uint32_t timeout = 0;

        while (!receivedInterrupt && timeout < TIMEOUT) {
            timeService.wait(Util::Time::Timestamp(0, 10000000));
            timeout += 10;
        }

        if (!receivedInterrupt) {
            if (!handleReadWriteError(device, cylinder, head)) {
                log.error("Timeout while writing a sector on drive %u", device.getDriveNumber());
                return false;
            }
            continue;
        }

        CommandStatus status = readCommandStatus();
        if ((status.statusRegister0 & 0xc0u) != 0) {
            if (!handleReadWriteError(device, cylinder, head)) {
                log.error("Failed to write a sector on drive %u", device.getDriveNumber());
                return false;
            }
            continue;
        }

        setMotorState(device, OFF);
        return true;
    }

    setMotorState(device, OFF);
    log.error("Failed to write a sector on drive %u", device.getDriveNumber());
    return false;
}

bool FloppyController::handleReadWriteError(FloppyDevice &device, uint8_t cylinder, uint8_t head) {
    if (checkMedia(device)) {
        calibrateDrive(device);

        if (!seek(device, 0, 0)) {
            return false;
        }

        return seek(device, cylinder, head);
    }

    return false;
}

}