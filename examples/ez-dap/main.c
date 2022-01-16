/*
Copyright (C) 2022 Ein Terakawa <applause@elfmimi.jp>

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

// This example is a minimally functional CMSIS-DAP ported to CY7C68013A.
//
// By means of Microsoft OS 2.0 Descriptors, MS-OS-2.0 for short,
// WinUSB driver will automatically get attached to the device.

#include <fx2lib.h>
#include <fx2delay.h>
#include <fx2usb.h>
#include <usbcdc.h>
#include "DAP.h"
#include "IO_Config.h"

usb_desc_device_c usb_device = {
  .bLength              = sizeof(struct usb_desc_device),
  .bDescriptorType      = USB_DESC_DEVICE,
  .bcdUSB               = 0x0210,
  .bDeviceClass         = USB_DEV_CLASS_PER_INTERFACE,
  .bDeviceSubClass      = USB_DEV_SUBCLASS_PER_INTERFACE,
  .bDeviceProtocol      = USB_DEV_PROTOCOL_PER_INTERFACE,
  .bMaxPacketSize0      = 64,
  .idVendor             = 0x04b4, // 0x0d28,
  .idProduct            = 0x8613, // 0x0204,
  .bcdDevice            = 0x0000,
  .iManufacturer        = 1,
  .iProduct             = 2,
  .iSerialNumber        = 3,
  .bNumConfigurations   = 1,
};

usb_desc_interface_c usb_iface_vendor = {
  .bLength              = sizeof(struct usb_desc_interface),
  .bDescriptorType      = USB_DESC_INTERFACE,
  .bInterfaceNumber     = 0,
  .bAlternateSetting    = 0,
  .bNumEndpoints        = 2,
  .bInterfaceClass      = USB_IFACE_CLASS_VENDOR,
  .bInterfaceSubClass   = 0,
  .bInterfaceProtocol   = 0,
  .iInterface           = 4,
};

usb_desc_endpoint_c usb_endpoint_ep2_out = {
  .bLength              = sizeof(struct usb_desc_endpoint),
  .bDescriptorType      = USB_DESC_ENDPOINT,
  .bEndpointAddress     = 2,
  .bmAttributes         = USB_XFER_BULK,
  .wMaxPacketSize       = 512,
  .bInterval            = 0,
};

usb_desc_endpoint_c usb_endpoint_ep6_in = {
  .bLength              = sizeof(struct usb_desc_endpoint),
  .bDescriptorType      = USB_DESC_ENDPOINT,
  .bEndpointAddress     = 6|USB_DIR_IN,
  .bmAttributes         = USB_XFER_BULK,
  .wMaxPacketSize       = 512,
  .bInterval            = 0,
};

usb_configuration_c usb_config = {
  {
    .bLength              = sizeof(struct usb_desc_configuration),
    .bDescriptorType      = USB_DESC_CONFIGURATION,
    .bNumInterfaces       = 1,
    .bConfigurationValue  = 1,
    .iConfiguration       = 0,
    .bmAttributes         = USB_ATTR_RESERVED_1,
    .bMaxPower            = 50,
  },
  {
    { .interface = &usb_iface_vendor },
    { .endpoint  = &usb_endpoint_ep2_out },
    { .endpoint  = &usb_endpoint_ep6_in },
    { 0 }
  }
};

usb_configuration_set_c usb_configs[] = {
  &usb_config,
};

usb_ascii_string_c usb_strings[] = {
  [0] = "libfx2",
  [1] = "FX2 series CMSIS-DAP example",
  [2] = "0011223344556677",
  [3] = "CMSIS-DAP v2",
};

usb_descriptor_set_c usb_descriptor_set = {
  .device           = &usb_device,
  .config_count     = ARRAYSIZE(usb_configs),
  .configs          = usb_configs,
  .string_count     = ARRAYSIZE(usb_strings),
  .strings          = usb_strings,
};

extern __xdata uint8_t bos_desc[];
extern __xdata uint8_t msos20_desc[];
extern __xdata uint16_t BOS_DESC_SIZE;
extern __xdata uint16_t MSOS20_DESC_SIZE;
// #define VENDOR_MS_OS_20_REQUEST 0xCC
extern __xdata uint8_t VENDOR_MS_OS_20_REQUEST;

void handle_usb_get_descriptor(enum usb_descriptor type, uint8_t index) {
  // At this point following conditions are satisfied.
  // req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_STANDARD|USB_DIR_IN) &&
  // req->bRequest == USB_REQ_GET_DESCRIPTOR
  if(type == 15 /* USB_DESC_BOS */) {
    // SETUP_EP0_IN_DATA(bos_desc, sizeof(bos_desc));
    // uint16_t length = 33; // sizeof(bos_desc);
    uint16_t length = BOS_DESC_SIZE;
    xmemcpy(EP0BUF, bos_desc, length);
    SETUP_EP0_BUF(length);
    return;
  }

  usb_serve_descriptor(&usb_descriptor_set, type, index);
}

void handle_usb_setup(__xdata struct usb_req_setup *req) {
  if(req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_IN) &&
     req->bRequest == VENDOR_MS_OS_20_REQUEST &&
     req->wIndex == 7 /* MS_OS_20_DESCRIPTOR_INDEX */) {
    // SETUP_EP0_IN_DATA(msos20_desc, sizeof(msos20_desc));
    // uint16_t length = 30; // sizeof(msos20_desc);
    uint16_t length = MSOS20_DESC_SIZE;
    xmemcpy(EP0BUF, msos20_desc, length);
    SETUP_EP0_BUF(length);
    return;
  }

  STALL_EP0();
}


volatile bool pending_ep6_in;

void isr_IBN() __interrupt {
  pending_ep6_in = true;
  CLEAR_USB_IRQ();
  NAKIRQ = _IBN;
  IBNIRQ = _IBNI_EP6;
}

static uint8_t led_connected = 0;
static int led_timer = 0;
// Register an interrupt handler for TIMER0 overflow
void isr_TF0() __interrupt(_INT_TF0) {
  if (led_timer != 0) {
    led_timer -= 1;
    if (led_timer == 0) {
      PIN_LED1 = led_connected & 1;
      PIN_LED0 = 0;
    }
  }
}

void led_blink() {
  PIN_LED0 = 1;
  PIN_LED1 = (led_connected ^ 1) & 1;
  led_timer = 8;
}

int main() {
  // Run core at 48 MHz fCLK.
  CPUCS = _CLKSPD1;

  // IO-pins to GPIO mode
  IFCONFIG = 0x80;

  // Configure TIMER0
  TMOD = _M0_0; // use 16-bit counter mode for TIMER0
  ET0 = 1;      // generate an interrupt
  TR0 = 1;      // run

  // Use newest chip features.
  REVCTL = _ENH_PKT|_DYN_OUT;

  // NAK all transfers.
  SYNCDELAY;
  FIFORESET = _NAKALL;

  // EP1IN is configured as INTERRUPT IN.
  EP1INCFG = _VALID|_TYPE1|_TYPE0;
  // EP1OUT is not used.
  EP1OUTCFG &= ~_VALID;

  // EP2 is configured as 512-byte double buffed BULK OUT.
  EP2CFG  =  _VALID|_TYPE1|_BUF1;
  EP2CS   = 0;
  // EP6 is configured as 512-byte double buffed BULK IN.
  EP6CFG  =  _VALID|_DIR|_TYPE1|_BUF1;
  EP6CS   = 0;
  // EP4/8 are not used.
  EP4CFG &= ~_VALID;
  EP8CFG &= ~_VALID;

  // Enable IN-BULK-NAK interrupt for EP6.
  IBNIE = _IBNI_EP6;
  NAKIE = _IBN;

  // Reset and prime EP2, and reset EP6.
  SYNCDELAY;
  FIFORESET = _NAKALL|2;
  SYNCDELAY;
  OUTPKTEND = _SKIP|2;
  SYNCDELAY;
  OUTPKTEND = _SKIP|2;
  SYNCDELAY;
  FIFORESET = _NAKALL|6;
  SYNCDELAY;
  FIFORESET = 0;

  PIN_LED0_OE |= PIN_LED0_MASK; // Set LED0 pin as output
  PIN_LED1_OE |= PIN_LED1_MASK; // Set LED1 pin as output
  PIN_LED0 = 0; // Red LED (v1.1 and v1.2 only) 
  PIN_LED1 = 0; // Blue LED
  
  {
    DAP_Data.transfer.retry_count = 255;
    DAP_Data.transfer.idle_cycles = 2;
    DAP_Data.transfer.match_retry = 255;
    DAP_Data.swd_conf.turnaround = 1;
    DAP_Data.swd_conf.data_phase = 1;
  }

  // Re-enumerate, to make sure our descriptors are picked up correctly.
  usb_init(/*disconnect=*/true);
  // usb_init() will enable all interrupts. ( EA = 1 )
  {
    uint16_t length = 0;
    while(1) {
      if(length == 0 && !(EP2CS & _EMPTY)) {
        length = (EP2BCH << 8) | EP2BCL;
        // read from EP2FIFOBUF , write to scratch
        length = dap_execute_command(0, length) & 0xFF;
        EP2BCL = 0;
      }

      if(length != 0 && pending_ep6_in) {
        xmemcpy(EP6FIFOBUF, scratch, length);
        EP6BCH = length >> 8;
        SYNCDELAY;
        EP6BCL = length;

        length = 0;
        pending_ep6_in = false;
      }
    }
  }
}
