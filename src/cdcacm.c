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

#define CAPTURED_Q_SIZE 2048 //power of 2

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
//volatile unsigned long long last = 0;
unsigned int tickcnt;
unsigned int g_ovf = 0;
int g_qstart = 0;
int g_qend = 0;
char g_first_time;
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
	g_captured[0] = 0;
	g_qstart = 1;
	g_qend = 1;
	g_first_time = 1;

	rcc_periph_clock_enable(RCC_TIM1);

	nvic_enable_irq(NVIC_TIM1_UP_IRQ);
	nvic_enable_irq(NVIC_TIM1_CC_IRQ);
	rcc_periph_reset_pulse(RST_TIM1);

	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	timer_set_prescaler(TIM1, (rcc_apb2_frequency / 1000000) -1 );
	timer_disable_preload(TIM1);
	timer_continuous_mode(TIM1);

	timer_set_period(TIM1, 65535);

	//input capture
	timer_ic_set_filter(TIM1, TIM_IC1, TIM_IC_CK_INT_N_2);
	timer_ic_set_polarity(TIM1, TIM_IC1, TIM_IC_RISING);
	timer_ic_set_prescaler(TIM1, TIM_IC1, TIM_IC_PSC_OFF);
	timer_ic_set_input(TIM1, TIM_IC1, TIM_IC_IN_TI1);
	timer_set_ti1_ch123_xor(TIM1);
	timer_ic_enable(TIM1, TIM_IC1);

	timer_ic_set_filter(TIM1, TIM_IC3, TIM_IC_CK_INT_N_2);
	timer_ic_set_polarity(TIM1, TIM_IC3, TIM_IC_FALLING);//RISING);
	timer_ic_set_prescaler(TIM1, TIM_IC3, TIM_IC_PSC_OFF);
	timer_ic_set_input(TIM1, TIM_IC3, TIM_IC_IN_TI1);
	timer_ic_enable(TIM1, TIM_IC3);

	timer_enable_counter(TIM1);

	timer_enable_irq(TIM1, TIM_DIER_UIE | TIM_DIER_CC1IE | TIM_DIER_CC3IE);
}
void TIM1_CC_IRQHandler(void)
{
	int end = g_qend;
	if (timer_get_flag(TIM1, TIM_SR_CC3OF)) {
		g_ovf++;
		timer_clear_flag(TIM1, TIM_SR_CC3OF);
	}
	if (timer_get_flag(TIM1, TIM_SR_CC1OF)) {
		g_ovf++;
		timer_clear_flag(TIM1, TIM_SR_CC1OF);
	}

	//falling edge
	if (timer_get_flag(TIM1, TIM_SR_CC3IF)) {
		if(((end + 1) & (CAPTURED_Q_SIZE - 1)) == g_qstart
		 || ((end + 2) & (CAPTURED_Q_SIZE - 1)) == g_qstart
		) {
			g_ovf++;
		} else {

			g_captured[end] = 0x8000000000000000 | ((unsigned long long)tickcnt << 16) | ((unsigned short)TIM_CCR3(TIM1));
		}
		end++;
		end &= (CAPTURED_Q_SIZE - 1);
		g_qend = end;

	}
	//rising edge
	if (timer_get_flag(TIM1, TIM_SR_CC1IF)) {
		if(((end + 1) & (CAPTURED_Q_SIZE - 1)) == g_qstart
		 || ((end + 2) & (CAPTURED_Q_SIZE - 1)) == g_qstart
		) {
			g_ovf++;
		} else {
			g_captured[end] = ((unsigned long long)tickcnt << 16) | ((unsigned short)TIM_CCR1(TIM1));
		}
		end++;
		end &= (CAPTURED_Q_SIZE - 1);
		g_qend = end;

	}


}
void TIM1_UP_IRQHandler(void)
{
	if (timer_get_flag(TIM1, TIM_SR_UIF)) {

		timer_clear_flag(TIM1, TIM_SR_UIF);
		tickcnt++;

//		uint16_t compare_time = timer_get_counter(TIM2);
		gpio_toggle(LED_PORT, LED_PIN);

	}
}

void tim2_out_init()
{
	//pb10 tim2 ch3 pwm output for testing
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO10);

	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ, AFIO_MAPR_TIM2_REMAP_FULL_REMAP);
	rcc_periph_clock_enable(RCC_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1,
	               TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, (rcc_apb2_frequency / 2000000) -1 );
	//timer_set_prescaler(TIM2, 100);
	timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM2);
	timer_enable_oc_output(TIM2, TIM_OC3);
	timer_enable_break_main_output(TIM2);
	timer_set_oc_value(TIM2, TIM_OC3, 5);
	timer_set_period(TIM2,200);// 4999);//9999);
	timer_enable_counter(TIM2);


}
int main(void)
{
	volatile int i;

	int start,cnt;
	char buff[100];

	usbd_dev = NULL;
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);

	tim2_out_init(); //can be commented out if not needed


	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
			GPIO_CNF_INPUT_PULL_UPDOWN, GPIO8);

	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				GPIO_CNF_INPUT_PULL_UPDOWN, GPIO9);


	//pa10 ir input pin from TL1838
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				GPIO_CNF_INPUT_PULL_UPDOWN, GPIO10);

	gpio_set(GPIOA, GPIO8 | GPIO9 | GPIO10); //pull up


	led_init();
	tim1_init();


	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
		gpio_clear(GPIOA, GPIO12);
		for(i = 0; i < 2100000; i++);//about 500ms for 2100000
		gpio_set(GPIOA, GPIO12);

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

	i = 0;
	while (1) {
		usbd_poll(usbd_dev);
		cnt = 0;
		if(g_qstart != g_qend) {
			//memset(buff, ' ', sizeof(buff));
			start = g_qstart;

			if(g_first_time) {
				sprintf(buff, "\r\n=====\r\n0 00 1 00000000 %u\r\n%d %02x %d %08x ",
					((unsigned int)g_captured[start]) - ((unsigned int)g_captured[(start - 1) & (CAPTURED_Q_SIZE - 1)]),
					g_ovf,
					start & 0xff,
					((((long long)g_captured[start]) > 0) ? 1 : 0),
					(unsigned int)g_captured[start]

				);

				g_first_time = 0;
			} else {
				sprintf(buff, "%u\r\n%d %02x %d %08x ",
					((unsigned int)g_captured[start]) - ((unsigned int)g_captured[(start - 1) & (CAPTURED_Q_SIZE - 1)]),
					g_ovf,
					start & 0xff,
					((((long long)g_captured[start]) > 0) ? 1 : 0),
					(unsigned int)g_captured[start]

				);
			}
			//start++;
			//start &= (CAPTURED_Q_SIZE - 1);
			//if(cnt < 2 && start != g_qend) {
				//cnt++;
				//continue;
			//}

			if(usbd_ep_write_packet(usbd_dev, 0x82, buff, strlen(buff)) != 0) {
				start++;
				start &= (CAPTURED_Q_SIZE - 1);
				g_qstart = start;
			}

		}
	}

}
