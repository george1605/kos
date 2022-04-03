#pragma once
#include "../bits.h"
struct hci_dev {
   char  name[8];
   unsigned long  flags;
   uint16_t		source;
   uint16_t		vendor;
   uint16_t		product;
   uint16_t		version;

   int (*open)(struct hci_dev *hdev);
   int (*close)(struct hci_dev *hdev);
   int (*flush)(struct hci_dev *hdev);
   int (*setup)(struct hci_dev *hdev);
   int (*shutdown)(struct hci_dev *hdev);
};

#define hci_dev_set_flag(hdev, nr)     set_bit((nr), (hdev)->flags)
#define hci_dev_clear_flag(hdev, nr)   clear_bit((nr), (hdev)->flags)

#define hci_dev_clear_volatile_flags(hdev)			\
	do {							\
		hci_dev_clear_flag(hdev, HCI_LE_SCAN);		\
		hci_dev_clear_flag(hdev, HCI_LE_ADV);		\
		hci_dev_clear_flag(hdev, HCI_LL_RPA_RESOLUTION);\
		hci_dev_clear_flag(hdev, HCI_PERIODIC_INQ);	\
		hci_dev_clear_flag(hdev, HCI_QUALITY_REPORT);	\
	} while(0)

#define hci_clrflags(hci) hci_dev_clear_volatile_flags(hci)
#define HCI_MAX_ACL_SIZE	1024
#define HCI_MAX_SCO_SIZE	255
#define HCI_MAX_ISO_SIZE	251
#define HCI_MAX_EVENT_SIZE	260

#define HCI_RTSOFF 0x18
#define HCI_CAPLENGTH 0x00
#define HCI_DBOFF 0x14
#define HCI_RVERSION 0x02
#define HCI_HCSPARAMS1 0x04
#define HCI_HCSPARAMS2 0x08
#define HCI_HCSPARAMS2 0x0C
#define HCI_HCCPARAMS1 0x10
#define HCI_HCCPARAMS2 0x1C
#define HCI_USBSTS 0x04
#define HCI_CONFIG 0x38

void hciopen(struct hci_dev* u){
   u->open(u);
}