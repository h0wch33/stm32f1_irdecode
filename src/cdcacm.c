/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>

#define LED_PORT GPIOC
#define LED_PIN GPIO13
#define LED_RCC RCC_GPIOC

#define CAPTURED_Q_SIZE 1024 //power of 2

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"Black Sphere Technologies",
	"CDC-ACM Demo",
	"DEMO",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];
static usbd_device *usbd_dev;
unsigned long long g_captured[CAPTURED_Q_SIZE];
unsigned int g_captured_cnt = 0;
unsigned int tickcnt;
unsigned int g_ovf = 0;
int g_qstart = 0;
int g_qend = 0;
static enum usbd_request_return_codes cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xA1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;
		// usbd_ep_write_packet(0x83, buf, 10);
		return USBD_REQ_HANDLED;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding))
			return USBD_REQ_NOTSUPP;
		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	(void)usbd_dev;

	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

//	if (len) {
	//	usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
	//}
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdcacm_control_request);
}
void led_init()
{
	rcc_periph_clock_enable(LED_RCC);
	gpio_clear(LED_PORT, LED_PIN);
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
			GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);


}

void tim1_init()
{
	tickcnt = 0;
	rcc_periph_clock_enable(RCC_TIM1);

		/* Enable TIM1 interrupt. */
		nvic_enable_irq(NVIC_TIM1_UP_IRQ);
		nvic_enable_irq(NVIC_TIM1_CC_IRQ);
		/* Reset TIM2 peripheral to defaults. */
		rcc_periph_reset_pulse(RST_TIM1);

		/* Timer global mode:
		 * - No divider
		 * - Alignment edge
		 * - Direction up
		 * (These are actually default values after reset above, so this call
		 * is strictly unnecessary, but demos the api for alternative settings)
		 */
		timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
			TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

		/*
		 * Please take note that the clock source for STM32 timers
		 * might not be the raw APB1/APB2 clocks.  In various conditions they
		 * are doubled.  See the Reference Manual for full details!
		 * In our case, TIM2 on APB1 is running at double frequency, so this
		 * sets the prescaler to have the timer run at 5kHz
		 */
		//timer_set_prescaler(TIM1, ((rcc_apb1_frequency * 2) / 5000));
		timer_set_prescaler(TIM1, (rcc_apb2_frequency / 1000000) -1 );
		/* Disable preload. */
		timer_disable_preload(TIM1);
		timer_continuous_mode(TIM1);

		/* count full range, as we'll update compare value continuously */
		timer_set_period(TIM1, 65535);

		/* Set the initual output compare value for OC1. */
		//timer_set_oc_value(TIM2, TIM_OC1, frequency_sequence[frequency_sel++]);




		//input capture
		timer_ic_set_filter(TIM1, TIM_IC1, TIM_IC_CK_INT_N_8);
		timer_ic_set_polarity(TIM1, TIM_IC1, TIM_IC_RISING);
		timer_ic_set_prescaler(TIM1, TIM_IC1, TIM_IC_PSC_OFF);
		timer_ic_set_input(TIM1, TIM_IC1, TIM_IC_IN_TI1);
		timer_ic_enable(TIM1, TIM_IC1);









		/* Counter enable. */
		timer_enable_counter(TIM1);

		/* Enable Channel 1 compare interrupt to recalculate compare values */
		timer_enable_irq(TIM1, TIM_DIER_UIE | TIM_DIER_CC1IE);
}
void TIM1_CC_IRQHandler(void)
{
	int end = g_qend;
	if (timer_get_flag(TIM1, TIM_SR_CC1IF)) {
		if(((end + 1) & (CAPTURED_Q_SIZE - 1)) == g_qstart) {
			g_ovf++;

		} else {
			if(g_captured_cnt & 1) {
				g_captured[end] = 0x8000000000000000 | ((unsigned long long)tickcnt << 16) | ((unsigned short)TIM_CCR1(TIM1));
			} else {
				g_captured[end] = ((unsigned long long)tickcnt << 16) | ((unsigned short)TIM_CCR1(TIM1));
			}

		}
		g_captured_cnt++;
		timer_clear_flag(TIM1, TIM_SR_CC1IF);
		if(g_captured_cnt & 1) {
			timer_ic_set_polarity(TIM1, TIM_IC1, TIM_IC_FALLING);
		} else {
			timer_ic_set_polarity(TIM1, TIM_IC1, TIM_IC_RISING);
		}

		end++;
		end &= (CAPTURED_Q_SIZE - 1);
		g_qend = end;

	}
}
void TIM1_UP_IRQHandler(void)//tim1_isr(void)
{
	if (timer_get_flag(TIM1, TIM_SR_UIF)) {

		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM1, TIM_SR_UIF);
		tickcnt++;
		/*
		 * Get current timer value to calculate next
		 * compare register value.
		 */
		uint16_t compare_time = timer_get_counter(TIM2);
		gpio_toggle(LED_PORT, LED_PIN);
		if(usbd_dev) {
			//usbd_ep_write_packet(usbd_dev, 0x82, "U\n", 2);
		}

#if 0
		/* Calculate and set the next compare value. */
		uint16_t frequency = frequency_sequence[frequency_sel++];
		uint16_t new_time = compare_time + frequency;

		timer_set_oc_value(TIM2, TIM_OC1, new_time);
		if (frequency_sel == ARRAY_LEN(frequency_sequence)) {
			frequency_sel = 0;
		}
#endif
		/* Toggle LED to indicate compare event. */
		//gpio_toggle(LED1_PORT, LED1_PIN);
	}
}
volatile unsigned long long last = 0;
int main(void)
{
	volatile int i;

	int start;
	char buff[60];

	usbd_dev = NULL;
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);

	//pa8 ir input pin from TL1838
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
			GPIO_CNF_INPUT_PULL_UPDOWN, GPIO8);
	led_init();
	tim1_init();
		//while(1);

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
		gpio_clear(GPIOA, GPIO12);
		for(i = 0; i < 5000000; i++);
		gpio_set(GPIOA, GPIO12);

//	rcc_periph_clock_enable(RCC_GPIOC);

//	gpio_set(GPIOC, GPIO11);
//	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
	//	      GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

	//for (i = 0; i < 0x800000; i++)
		//__asm__("nop");
	//gpio_clear(GPIOC, GPIO11);
	i = 0;
	while (1) {
		usbd_poll(usbd_dev);
		if(g_qstart != g_qend) {
			start = g_qstart;
			if(last == 0) {
				last = g_captured[start];
			}

			sprintf(buff, "%d %d %08x %u\r\n", g_ovf, ((((long long)g_captured[start]) < 0) ? 1 : 0), (unsigned int)g_captured[start], (unsigned int)(g_captured[start] - last));
			last = g_captured[start];
			start++;
			start &= (CAPTURED_Q_SIZE - 1);
			g_qstart = start;
			usbd_ep_write_packet(usbd_dev, 0x82, buff, strlen(buff));
		}
//		if((i++ & 0x3fff) == 0) {
		//	usbd_ep_write_packet(usbd_dev, 0x82, "Hello\r\n", 7);
	//	}
	}

}
