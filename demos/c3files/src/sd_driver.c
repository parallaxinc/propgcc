#include <stdint.h>

uint8_t sd_driver_array[] = {
 0x06, 0x00, 0x7c, 0x5c, 0x00, 0x04, 0x00, 0x00,
 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
 0xf0, 0x79, 0xbd, 0xa0, 0xbc, 0x7a, 0xbd, 0xa0,
 0x04, 0x7a, 0xfd, 0x80, 0x00, 0x0a, 0x7c, 0x86,
 0x0d, 0x00, 0x54, 0x5c, 0x9b, 0x26, 0xfd, 0x50,
 0x9f, 0x3a, 0xfd, 0x50, 0x03, 0x82, 0xbd, 0xa0,
 0x02, 0x82, 0xbd, 0x68, 0x04, 0x84, 0xbd, 0xa0,
 0x02, 0x84, 0xbd, 0x68, 0x05, 0x84, 0xbd, 0x68,
 0x03, 0x84, 0xbd, 0x68, 0x9d, 0x40, 0xfd, 0x5c,
 0x00, 0x6c, 0xfd, 0x08, 0x00, 0xec, 0xff, 0xa0,
 0xbc, 0x7e, 0x3d, 0x08, 0xbc, 0x7c, 0xbd, 0x0a,
 0x17, 0x00, 0x68, 0x5c, 0xc2, 0xec, 0xbf, 0xa0,
 0x02, 0x7c, 0x7d, 0x62, 0x1d, 0x00, 0x68, 0x5c,
 0x15, 0x00, 0x7c, 0x5c, 0xbe, 0x86, 0xbd, 0xa0,
 0x08, 0x86, 0xfd, 0x28, 0x02, 0x7c, 0xfd, 0x28,
 0x07, 0x7c, 0xfd, 0x60, 0x23, 0x7c, 0xfd, 0x80,
 0xbe, 0x00, 0x3c, 0x5c, 0x15, 0x00, 0x7c, 0x5c,
 0x15, 0x00, 0x7c, 0x5c, 0x15, 0x00, 0x7c, 0x5c,
 0x2b, 0x00, 0x7c, 0x5c, 0x3d, 0x00, 0x7c, 0x5c,
 0x55, 0x00, 0x7c, 0x5c, 0x15, 0x00, 0x7c, 0x5c,
 0x15, 0x00, 0x7c, 0x5c, 0x00, 0x70, 0xfd, 0xa0,
 0x9d, 0x40, 0xfd, 0x5c, 0xba, 0x80, 0xbd, 0xa0,
 0xac, 0x66, 0xfd, 0x5c, 0x2e, 0x80, 0xfd, 0xe4,
 0x93, 0x38, 0xfd, 0x5c, 0x40, 0x68, 0xfd, 0xa0,
 0x00, 0x6a, 0xfd, 0xa0, 0x78, 0x24, 0xfd, 0x5c,
 0x77, 0x68, 0xfd, 0xa0, 0x78, 0x24, 0xfd, 0x5c,
 0x69, 0x68, 0xfd, 0xa0, 0x78, 0x24, 0xfd, 0x5c,
 0x01, 0x8e, 0x7d, 0x86, 0x34, 0x00, 0x68, 0x5c,
 0x75, 0x8e, 0x7d, 0xec, 0xc7, 0x70, 0xbd, 0xa0,
 0x75, 0x00, 0x7c, 0x5c, 0x00, 0x70, 0xfd, 0xa0,
 0xc3, 0x88, 0xbd, 0x08, 0x04, 0x86, 0xfd, 0x80,
 0xc3, 0x8a, 0xbd, 0x0a, 0x75, 0x00, 0x68, 0x5c,
 0x04, 0x86, 0xfd, 0x80, 0xc3, 0x86, 0xbd, 0x08,
 0x93, 0x38, 0xfd, 0x5c, 0x51, 0x68, 0xfd, 0xa0,
 0xc3, 0x6a, 0xbd, 0xa0, 0x78, 0x24, 0xfd, 0x5c,
 0x88, 0x24, 0xfd, 0x5c, 0xbb, 0x72, 0xbd, 0xa0,
 0xac, 0x66, 0xfd, 0x5c, 0x4f, 0x8a, 0x7d, 0xec,
 0xc4, 0x8e, 0x3d, 0x00, 0x01, 0x88, 0xfd, 0x80,
 0x01, 0x8a, 0xfd, 0x84, 0x4a, 0x72, 0xfd, 0xe4,
 0xac, 0x66, 0xfd, 0x5c, 0xac, 0x66, 0xfd, 0x5c,
 0x01, 0x86, 0xfd, 0x80, 0x46, 0x8a, 0x7d, 0xe8,
 0x75, 0x00, 0x7c, 0x5c, 0x00, 0x70, 0xfd, 0xa0,
 0xc3, 0x88, 0xbd, 0x08, 0x04, 0x86, 0xfd, 0x80,
 0xc3, 0x8a, 0xbd, 0x0a, 0x75, 0x00, 0x68, 0x5c,
 0x04, 0x86, 0xfd, 0x80, 0xc3, 0x86, 0xbd, 0x08,
 0x93, 0x38, 0xfd, 0x5c, 0x58, 0x68, 0xfd, 0xa0,
 0xc3, 0x6a, 0xbd, 0xa0, 0x78, 0x24, 0xfd, 0x5c,
 0xfe, 0x8e, 0xfd, 0xa0, 0xa1, 0x56, 0xfd, 0x5c,
 0xbb, 0x72, 0xbd, 0xa0, 0x00, 0x8e, 0xfd, 0xa0,
 0x68, 0x8a, 0x7d, 0xec, 0xc4, 0x8e, 0xbd, 0x00,
 0x01, 0x88, 0xfd, 0x80, 0x01, 0x8a, 0xfd, 0x84,
 0xa1, 0x56, 0xfd, 0x5c, 0x63, 0x72, 0xfd, 0xe4,
 0xac, 0x66, 0xfd, 0x5c, 0xac, 0x66, 0xfd, 0x5c,
 0x88, 0x24, 0xfd, 0x5c, 0x1f, 0x8e, 0xfd, 0x60,
 0x05, 0x8e, 0x7d, 0x86, 0x01, 0x70, 0xd5, 0xa0,
 0x75, 0x00, 0x54, 0x5c, 0x00, 0x20, 0xfd, 0x50,
 0x89, 0x24, 0xfd, 0x5c, 0x01, 0x86, 0xfd, 0x80,
 0x5e, 0x8a, 0x7d, 0xe8, 0x9d, 0x40, 0xfd, 0x5c,
 0xbd, 0x70, 0x3d, 0x08, 0x15, 0x00, 0x7c, 0x5c,
 0xac, 0x66, 0xfd, 0x5c, 0xb4, 0x8e, 0xbd, 0xa0,
 0xa1, 0x56, 0xfd, 0x5c, 0xb5, 0x8e, 0xbd, 0xa0,
 0x0f, 0x8e, 0xfd, 0x28, 0xa1, 0x56, 0xfd, 0x5c,
 0xb5, 0x8e, 0xbd, 0xa0, 0x07, 0x8e, 0xfd, 0x28,
 0xa1, 0x56, 0xfd, 0x5c, 0xb5, 0x8e, 0xbd, 0xa0,
 0x01, 0x8e, 0xfd, 0x2c, 0xa1, 0x56, 0xfd, 0x5c,
 0x00, 0x8e, 0xfd, 0xa0, 0xa1, 0x56, 0xfd, 0x5c,
 0x95, 0x8e, 0xfd, 0xa0, 0xa1, 0x56, 0xfd, 0x5c,
 0xff, 0x20, 0xfd, 0x50, 0xf1, 0x6f, 0xbd, 0xa0,
 0xac, 0x66, 0xfd, 0x5c, 0xf1, 0x81, 0xbd, 0xa0,
 0xb7, 0x80, 0xbd, 0x84, 0xb6, 0x80, 0x3d, 0x85,
 0x01, 0x70, 0xcd, 0xa0, 0x75, 0x00, 0x4c, 0x5c,
 0x00, 0x8e, 0x7d, 0x86, 0x8a, 0x00, 0x68, 0x5c,
 0x00, 0x00, 0x7c, 0x5c, 0x94, 0x00, 0x7c, 0x5c,
 0x05, 0x80, 0xfd, 0xa0, 0x04, 0xe8, 0xbf, 0x64,
 0x04, 0xe8, 0xbf, 0x68, 0x05, 0xe8, 0xbf, 0x68,
 0x05, 0xe8, 0xbf, 0x64, 0x97, 0x80, 0xfd, 0xe4,
 0x9c, 0x00, 0x7c, 0x5c, 0x04, 0xe8, 0xbf, 0x64,
 0x00, 0x00, 0x7c, 0x5c, 0x9e, 0x00, 0x7c, 0x5c,
 0x04, 0xe8, 0xbf, 0x64, 0x04, 0xe8, 0xbf, 0x68,
 0x00, 0x00, 0x7c, 0x5c, 0x18, 0x8e, 0xfd, 0x2c,
 0x08, 0x8c, 0xfd, 0xa0, 0xa5, 0x00, 0x7c, 0x5c,
 0x02, 0xe8, 0xbf, 0x64, 0x01, 0x8e, 0xfd, 0x25,
 0x03, 0xe8, 0xbf, 0x70, 0x02, 0xe8, 0xbf, 0x68,
 0xa4, 0x8c, 0xfd, 0xe4, 0x02, 0xe8, 0xbf, 0x64,
 0x03, 0xe8, 0xbf, 0x68, 0x00, 0x00, 0x7c, 0x5c,
 0x00, 0x8e, 0xfd, 0xa0, 0x08, 0x8c, 0xfd, 0xa0,
 0x02, 0xe8, 0xbf, 0x68, 0xf2, 0x03, 0x3c, 0x61,
 0x01, 0x8e, 0xfd, 0x34, 0x02, 0xe8, 0xbf, 0x64,
 0xae, 0x8c, 0xfd, 0xe4, 0x00, 0x00, 0x7c, 0x5c,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
 0x00, 0x0a, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

int sd_driver_size = sizeof(sd_driver_array);