/*
The application uses the libusb 1.0 API from libusb.org.
Compile with: gcc razer.c -lusb-1.0 -I /usr/include/libusb-1.0/ -Wall -o razer_$ARCH;sudo chown root:root razer_$ARCH;sudo chmod +s razer_$ARCH
*/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>

// Values for bmRequestType in the Setup transaction's Data packet.

static const int CONTROL_REQUEST_TYPE_IN = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
static const int CONTROL_REQUEST_TYPE_OUT = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;

// From the HID spec:

static const int HID_GET_REPORT = 0x01;
static const int HID_SET_REPORT = 0x09;
static const int HID_REPORT_TYPE_INPUT = 0x01;
static const int HID_REPORT_TYPE_OUTPUT = 0x02;
static const int HID_REPORT_TYPE_FEATURE = 0x03;

// With firmware support, transfers can be > the endpoint's max packet size.

static const int MAX_CONTROL_IN_TRANSFER_SIZE = 2;
static const int MAX_CONTROL_OUT_TRANSFER_SIZE = 2;

static const int INTERFACE_NUMBER = 1;
static const int TIMEOUT_MS = 5000;

int write_chars(libusb_device_handle *devh){
	unsigned char data_out[90]= {	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
				0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x06, 0x00};
	// Send data to the device.
	/*int i;
	for (i=0;i < sizeof(data_out); i++){
		printf("#%d\n",data_out[i]);
	}
	return 1;*/
	//int status = usb_bulk_write (devh, 3, data_out, sizeof (data_out), 500);
	//printf ("bulk writing: %s\n", status ? "failed" : "passed");
	int bytes_sent = libusb_control_transfer(
			devh,
			CONTROL_REQUEST_TYPE_OUT,
			HID_SET_REPORT,
			//(HID_REPORT_TYPE_FEATURE<<8)|0x00,
			0x0300,
			INTERFACE_NUMBER,
			data_out,
			sizeof(data_out),
			TIMEOUT_MS);
	printf("Bytes sent: %d\n",bytes_sent);
	return 1;
}
int main(void){
	// Change these as needed to match idVendor and idProduct in your device's device descriptor.

	static const int VENDOR_ID = 0x1532;
	static const int PRODUCT_ID = 0x010e;

	struct libusb_device_handle *devh = NULL;
	int device_ready = 0;
	int result;

	result = libusb_init(NULL);
	if (result >= 0){
		devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);

		if (devh != NULL){
			// The HID has been detected.
			// Detach the hidusb driver from the HID to enable using libusb.
			libusb_detach_kernel_driver(devh, INTERFACE_NUMBER);
			{
				result = libusb_claim_interface(devh, INTERFACE_NUMBER);
				if (result >= 0){
					device_ready = 1;
				}else{
					fprintf(stderr, "libusb_claim_interface error %d\n", result);
				}
			}
		}else{
			fprintf(stderr, "Unable to find the device.\n");
		}
	}else{
		fprintf(stderr, "Unable to initialize libusb.\n");
	}

	if (device_ready){
		// Send and receive data.
		write_chars(devh);
		// Finished using the device.
		libusb_release_interface(devh, INTERFACE_NUMBER);
	}
	libusb_close(devh);
	libusb_exit(NULL);
	return 0;
}
