#include "HubDriver.h"
#include "../../dev/UsbDevice.h"
#include "../../include/UsbControllerInclude.h"
#include "../../include/UsbErrors.h"
#include "../../include/UsbInterface.h"
#include "../../interfaces/LoggerInterface.h"
#include "../../interfaces/ThreadInterface.h"
#include "../UsbDriver.h"

static HubDriver *internal_hub_driver = 0;

static void new_usb_driver(UsbDriver *usb_driver, char *name,
                           UsbDevice_ID *entry);

static void new_usb_driver(UsbDriver *usb_driver, char *name,
                           UsbDevice_ID *entry) {
  usb_driver->probe = &probe_hub;
  usb_driver->disconnect = &disconnect_hub;

  usb_driver->name = name;
  usb_driver->entry = entry;
  usb_driver->head.l_e = 0;
  usb_driver->l_e.l_e = 0;
}

// if we use plug & play this callback would listen to the interrupt transfer
// which checks the status of the downstream ports
void callback_hub(UsbDev *dev, uint32_t status, void *data) {}

int16_t probe_hub(UsbDev *dev, Interface *interface) {
  Endpoint **endpoints = interface->active_interface->endpoints;
  InterfaceDescriptor interface_desc =
      interface->active_interface->alternate_interface_desc;
  // check if we should support that interface !!!
  int e = interface_desc.bNumEndpoints;

  HubDev *hub_dev = internal_hub_driver->get_free_hub_dev(internal_hub_driver);
  if (hub_dev == (void *)0)
    return -1;

  MemoryService_C *mem_service =
      (MemoryService_C *)container_of(dev->mem_service, MemoryService_C, super);

  // select only 1 endpoint
  for (int i = 0; i < e; i++) {
    // check for type , direction
    if (!(endpoints[i]->endpoint_desc.bEndpointAddress & DIRECTION_IN)) {
      internal_hub_driver->free_hub_dev(internal_hub_driver, hub_dev);
      continue;
    }
    if (!(endpoints[i]->endpoint_desc.bmAttributes & TRANSFER_TYPE_INTERRUPT)) {
      internal_hub_driver->free_hub_dev(internal_hub_driver, hub_dev);
      continue;
    }
    if (!hub_dev->endpoint_addr) {
      uint8_t *key_board_buffer =
          (uint8_t *)mem_service->mapIO(mem_service, sizeof(uint8_t), 1);

      hub_dev->endpoint_addr =
          endpoints[i]->endpoint_desc.bEndpointAddress & ENDPOINT_MASK;
      hub_dev->usb_dev = dev;
      hub_dev->buffer = key_board_buffer;
      hub_dev->buffer_size = 0;
      hub_dev->priority = PRIORITY_8;
      hub_dev->interface = interface;
      hub_dev->interval = endpoints[i]->endpoint_desc.bInterval;
      return 1;
    }
  }

  return -1;
}

void disconnect_hub(UsbDev *dev, Interface *interface) {}

void new_hub_driver(HubDriver *driver, char *name, UsbDevice_ID *entry) {
  driver->read_hub_status = &read_hub_status;
  driver->read_hub_descriptor = &read_hub_descriptor;
  driver->configure_hub = &configure_hub;
  driver->set_hub_feature = &set_hub_feature;
  driver->clear_hub_feature = &clear_hub_feature;
  driver->dump_port_status = &dump_port_status;
  driver->dump_port_status_change = &dump_port_status_change;
  driver->init_hub_driver_logger = &init_hub_driver_logger;
  driver->is_device_removable = &is_device_removable;
  driver->get_free_hub_dev = &get_free_hub_dev;
  driver->match_hub_dev = &match_hub_dev;
  driver->free_hub_dev = &free_hub_dev;

  for (int i = 0; i < MAX_DEVICES_PER_USB_DRIVER; i++) {
    driver->hub_map[i] = 0;
    driver->dev[i].transfer_success = 0;
    driver->dev[i].x_powered = 0;
    driver->dev[i].x_wakeup = 0;
    driver->dev[i].usb_dev = 0;
    driver->dev[i].endpoint_addr = 0;
    driver->dev[i].buffer = 0;
    driver->dev[i].buffer_size = 0;
    driver->dev[i].priority = 0;
    driver->dev[i].interface = 0;
    driver->dev[i].interval = 0;
    driver->dev[i].callback = &callback_hub;
    driver->dev[i].driver = (UsbDriver *)driver;
  }

  internal_hub_driver = driver;

  driver->super.new_usb_driver = &new_usb_driver;
  driver->super.new_usb_driver(&driver->super, name, entry);
  driver->hub_driver_logger = driver->init_hub_driver_logger(driver);
}

HubDev *get_free_hub_dev(HubDriver *driver) {
  for (int i = 0; i < MAX_DEVICES_PER_USB_DRIVER; i++) {
    if (driver->hub_map[i] == 0) {
      driver->hub_map[i] = 1;
      return driver->dev + i;
    }
  }
  return (void *)0;
}

HubDev *match_hub_dev(HubDriver *driver, UsbDev *dev) {
  for (int i = 0; i < MAX_DEVICES_PER_USB_DRIVER; i++) {
    if (driver->dev[i].usb_dev == dev)
      return driver->dev + i;
  }
  return (void *)0;
}

void free_hub_dev(HubDriver *driver, HubDev *hub_dev) {
  for (int i = 0; i < MAX_DEVICES_PER_USB_DRIVER; i++) {
    if (driver->dev + i == hub_dev) {
      driver->hub_map[i] = 0;
      return;
    }
  }
}

void configure_callback(UsbDev *dev, uint32_t status, void *data) {
  HubDev *hub_dev =
      internal_hub_driver->match_hub_dev(internal_hub_driver, dev);

  if (hub_dev == (void *)0)
    return;

  if (status == S_TRANSFER) {
    hub_dev->transfer_success = 1;
  } else
    hub_dev->transfer_success = 0;
}

// stacking hub's currently not supported ! -> maybe implemented in future
int configure_hub(HubDriver *driver) {
  for (int i = 0; i < MAX_DEVICES_PER_USB_DRIVER; i++) {
    if (driver->hub_map[i] == 0)
      continue;
    UsbDev *dev = driver->dev[i].usb_dev;
    Interface *itf = driver->dev[i].interface;

    HubDev *hub_dev = driver->dev + i;

    MemoryService_C *m = (MemoryService_C *)container_of(
        dev->mem_service, MemoryService_C, super);
    uint8_t *data = (uint8_t *)m->mapIO(m, PAGE_SIZE * sizeof(uint8_t), 1);

    if (driver->read_hub_descriptor(driver, hub_dev, itf, data) == -1)
      return -1;

    if (driver->read_hub_status(driver, hub_dev, itf, data, 0x02) == -1)
      return -1;
    driver->dev[i].x_powered = *data & SELF_POWERED;
    driver->dev[i].x_wakeup = *data & REMOTE_WAKEUP;

    uint8_t wait_time = driver->dev[i].hub_desc.potpgt;
    uint8_t multiplicator_wait_time_ms = 2;

    uint8_t num_ports = driver->dev[i].hub_desc.num_ports;

    driver->hub_driver_logger->info_c(driver->hub_driver_logger,
      "Detected %d downstream ports , minor %d", num_ports, i);

    dev->add_downstream(dev, num_ports);

    uint16_t port_status_field;
    uint16_t port_change_status_field;
    uint8_t device_attached_mask = 0x01;
    uint8_t start_device_num = 0x03 + ((dev->dev_num-1) * 8);

    for (uint8_t start_port = 0x01; start_port <= num_ports; start_port++) {
      if (driver->set_hub_feature(driver, hub_dev, itf, start_port,
                                  PORT_POWER) == -1)
        return -1;
      mdelay(wait_time * multiplicator_wait_time_ms);

      if (driver->clear_hub_feature(driver, hub_dev, itf, start_port,
                                    C_PORT_OVER_CURRENT) == -1)
        return -1;

#ifdef HUB_DRIVER_DUMP_PORTS

      if (driver->read_hub_status(driver, hub_dev, itf, data, 0x04) == -1)
        return -1;
      port_status_field = *((uint16_t *)data);
      port_change_status_field = *((uint16_t *)(data + 2));
      driver->dump_port_status(driver, &port_status_field);

#endif

      if (driver->clear_hub_feature(driver, hub_dev, itf, start_port,
                                    C_PORT_CONNECTION) == -1)
        return -1;

      if (driver->read_hub_status(driver, hub_dev, itf, data, 0x04) == -1)
        return -1;

      port_status_field = *((uint16_t *)data);
      port_change_status_field = *((uint16_t *)(data + 2));

#ifdef HUB_DRIVER_DUMP_PORTS
      driver->dump_port_status(driver, &port_status_field);
      driver->dump_port_status_change(driver, &port_change_status_field);
#endif

      if (port_status_field & device_attached_mask) { // device attached
        if (driver->set_hub_feature(driver, hub_dev, itf, start_port,
                                    PORT_RESET) == -1)
          return -1;
#ifdef HUB_DRIVER_DUMP_PORTS

        if (driver->read_hub_status(driver, hub_dev, itf, data, 0x04) == -1)
          return -1;
        port_status_field = *((uint16_t *)data);
        port_change_status_field = *((uint16_t *)(data + 2));
        driver->dump_port_status(driver, &port_status_field);
        driver->dump_port_status_change(driver, &port_change_status_field);

#endif
        if (driver->clear_hub_feature(driver, hub_dev, itf, start_port,
                                      C_PORT_RESET) == -1)
          return -1;

#ifdef HUB_DRIVER_DUMP_PORTS

        if (driver->read_hub_status(driver, hub_dev, itf, data, 0x04) == -1)
          return -1;
        port_status_field = *((uint16_t *)data);
        port_change_status_field = *((uint16_t *)(data + 2));
        driver->dump_port_status(driver, &port_status_field);
        driver->dump_port_status_change(driver, &port_change_status_field);

#endif

        uint8_t speed =
            ((port_status_field & 0x200) >> 9) == 1 ? LOW_SPEED : FULL_SPEED;
        uint8_t removable =
            driver->is_device_removable(driver, hub_dev, start_port);
        uint8_t level = dev->level + 1;

        UsbDev *new_dev = m->allocateKernelMemory_c(m, sizeof(UsbDev), 0);
        new_dev->new_usb_device = &new_usb_device;
        new_dev->new_usb_device(new_dev, speed, start_port, level, removable,
                                dev->rootport, start_device_num,
                                dev->mem_service, dev->controller, 1);
        if (new_dev->error_while_transfering) {
          new_dev->delete_usb_dev(new_dev);
        } else {
          dev->add_downstream_device(dev, new_dev);
          start_device_num++;
          driver->hub_driver_logger->info_c(driver->hub_driver_logger, 
          "Succesful configured Usb-Device on port : %d , "
          "Device : %s, %s",
          start_port, new_dev->manufacturer, new_dev->product);
        }
      }
    }
    m->unmap(m, (uint32_t)(uintptr_t)data);
  }

  return 1;
}

// if bit is set -> non removable
uint8_t is_device_removable(HubDriver *driver, HubDev *hub_dev,
                            uint8_t downstream_port) {
  uint8_t *removable_x = hub_dev->hub_desc.x;
  uint8_t mask = 0x01;

  if (downstream_port < 8) {
    return ((removable_x[0] >> downstream_port) & mask) ^ mask;
  }
  uint8_t byte_num = downstream_port / sizeof(uint8_t);
  uint8_t shift = downstream_port % sizeof(uint8_t);
  return (removable_x[byte_num] >> shift) ^ mask;
}

void dump_port_status(HubDriver *driver, uint16_t *port_status_field) {
  uint8_t mask = 0x01;
  uint8_t port_indicator = (*port_status_field >> PORT_INDICATOR_STATUS) & mask;
  uint8_t port_test = (*port_status_field >> PORT_TEST_STATUS) & mask;
  uint8_t port_high_speed =
      (*port_status_field >> PORT_DEVICE_ATTACHED_HIGH_SPEED_STATUS) & mask;
  uint8_t port_low_speed =
      (*port_status_field >> PORT_DEVICE_ATTACHED_LOW_SPEED_STATUS) & mask;
  uint8_t port_power = (*port_status_field >> PORT_POWER_STATUS) & mask;
  uint8_t port_reset = (*port_status_field >> PORT_RESET_STATUS) & mask;
  uint8_t port_over_current =
      (*port_status_field >> PORT_OVER_CURRENT_STATUS) & mask;
  uint8_t port_suspend = (*port_status_field >> PORT_SUSPEND_STATUS) & mask;
  uint8_t port_enable = (*port_status_field >> PORT_ENABLE_STATUS) & mask;
  uint8_t port_connect = *port_status_field & mask;

  driver->hub_driver_logger->debug_c(
      driver->hub_driver_logger,
      "PORT_STATUS :\n\n\tport indicator : %u\n\tport test : %u\n\tport high "
      "speed attached : %u\n"
      "\tport low speed attached : %u\n\tport power : %u\n\tport reset : "
      "%u\n\tport over current : %u\n"
      "\tport suspend : %u\n\tport enable : %u\n\tport connect : %u\n",
      port_indicator, port_test, port_high_speed, port_low_speed, port_power,
      port_reset, port_over_current, port_suspend, port_enable, port_connect);
}

void dump_port_status_change(HubDriver *driver,
                             uint16_t *port_status_change_field) {
  uint8_t mask = 0x01;
  uint8_t reset_change =
      (*port_status_change_field >> PORT_RESET_CHANGE_STATUS) & mask;
  uint8_t over_current_change =
      (*port_status_change_field >> PORT_OVER_CURRENT_CHANGE_STATUS) & mask;
  uint8_t suspend_change =
      (*port_status_change_field >> PORT_SUSPEND_CHANGE_STATUS) & mask;
  uint8_t enable_change =
      (*port_status_change_field >> PORT_ENABLE_CHANGE_STATUS) & mask;
  uint8_t connect_change = *port_status_change_field & mask;

  driver->hub_driver_logger->debug_c(
      driver->hub_driver_logger,
      "PORT CHANGE STATUS :\n\n\treset change : %u\n\tover current change : "
      "%u\n\tsuspend change : %u\n"
      "\tenable change : %u\n\tconnect change : %u\n",
      reset_change, over_current_change, suspend_change, enable_change,
      connect_change);
}

Logger_C *init_hub_driver_logger(HubDriver *driver) {
  Logger_C *l = (Logger_C *)interface_allocateMemory(sizeof(Logger_C), 0);
  l->new_logger = &new_logger;
  l->new_logger(l, USB_DRIVER_LOGGER_TYPE, LOGGER_LEVEL_DEBUG);

  return l;
}

int read_hub_status(HubDriver *driver, HubDev *hub_dev, Interface *itf,
                    uint8_t *data, unsigned int len) {
  if (hub_dev->usb_dev->get_req_status(hub_dev->usb_dev, itf, data, len,
                                       &configure_callback) == -1)
    return -1;

  if (!hub_dev->transfer_success)
    return -1;
  return 1;
}

int read_hub_descriptor(HubDriver *driver, HubDev *hub_dev, Interface *itf,
                        uint8_t *data) {
  if (hub_dev->usb_dev->get_descriptor(hub_dev->usb_dev, itf, data, 2,
                                       &configure_callback) == -1)
    return -1;
  // instead of polling in the control block, we will just poll in here ->
  // needed to change again the whole control transfer via flag
  if (!hub_dev->transfer_success)
    return -1;

  HubDescriptor *hub_desc = (HubDescriptor *)data;
  uint8_t hub_desc_len = hub_desc->len;
  if (hub_dev->usb_dev->get_descriptor(hub_dev->usb_dev, itf, data,
                                       hub_desc_len, &configure_callback) == -1)
    return -1;
  if (!hub_dev->transfer_success)
    return -1;
  hub_desc = (HubDescriptor *)data;

  hub_dev->hub_desc = *hub_desc;

  return 1;
}

int set_hub_feature(HubDriver *driver, HubDev *hub_dev, Interface *itf,
                    uint16_t port, uint16_t feature) {
  if (hub_dev->usb_dev->set_feature(hub_dev->usb_dev, itf, feature, port,
                                    &configure_callback) == -1)
    return -1;

  if (!hub_dev->transfer_success)
    return -1;

  return 1;
}

int clear_hub_feature(HubDriver *driver, HubDev *hub_dev, Interface *itf,
                      uint16_t port, uint16_t feature) {
  if (hub_dev->usb_dev->clear_feature(hub_dev->usb_dev, itf, feature, port,
                                      &configure_callback) == -1)
    return -1;

  if (!hub_dev->transfer_success)
    return -1;

  return 1;
}