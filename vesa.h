#pragma once
#include "lib.c"
#include "port.h"
#define VESA_NO_BLANKING 0
#define VESA_VSYNC_SUSPEND 1
#define VESA_HSYNC_SUSPEND 2
#define VESA_POWERDOWN 3
#define VESA_VALID0(X,Y) (X < 1024 && Y < 768)
#define VESA_VALID1(X,Y) (X < 1920 && Y < 1080)
#define VESA_VALID2(X, Y) (X < 2560 && Y < 1440)
size_t vbecolor = 0xffffff;

typedef uint32_t *uiptr;
typedef union uint24 {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb;
    unsigned int integer : 24 __attribute__((packed));;
}uint24_t;

typedef struct
{
    // Mandatory info for all VBE revisions
    uint16_t mode_attributes;
    uint8_t window_a_attributes;
    uint8_t window_b_attributes;
    uint16_t window_granularity;
    uint16_t window_size;
    uint16_t window_a_segment;
    uint16_t window_b_segment;
    uint32_t window_function_pointer;
    uint16_t bytes_per_scanline;

    // Mandatory info for VBE 1.2 and above
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t x_charsize;
    uint8_t y_charsize;
    uint8_t number_of_planes;
    uint8_t bits_per_pixel;
    uint8_t number_of_banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t number_of_image_pages;
    uint8_t reserved1;

    // Direct color fields (required for direct/6 and YUV/7 memory models)
    uint8_t red_mask_size;
    uint8_t red_field_position;
    uint8_t green_mask_size;
    uint8_t green_field_position;
    uint8_t blue_mask_size;
    uint8_t blue_field_position;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_position;
    uint8_t direct_color_mode_info;

    // Mandatory info for VBE 2.0 and above
    uint32_t physical_base_pointer; // Physical address for flat memory frame buffer
    uint32_t reserved2;
    uint16_t reserved3;

    // Mandatory info for VBE 3.0 and above
    uint16_t linear_bytes_per_scanline;
    uint8_t bank_number_of_image_pages;
    uint8_t linear_number_of_image_pages;
    uint8_t linear_red_mask_size;
    uint8_t linear_red_field_position;
    uint8_t linear_green_mask_size;
    uint8_t linear_green_field_position;
    uint8_t linear_blue_mask_size;
    uint8_t linear_blue_field_position;
    uint8_t linear_reserved_mask_size;
    uint8_t linear_reserved_field_position;
    uint32_t max_pixel_clock;

    uint8_t reserved4[190]; 

} __attribute__((packed)) vbe_mode_info;

vbe_mode_info* gfxmode = (vbe_mode_info*)0x9000;

void vesa_memcpy24_32(uint24_t* dest, size_t* src, size_t count) {
    uint24_t t;
    uint32_t i;
    for(i = 0; i < count; i++) {
        t.integer = src[i];
        dest[i] = t;
    }
}

void vesa_memset_rgb(uint8_t* dest, size_t rgb, size_t count) {
    if(count % 3 != 0)
        count = count + 3 - (count % 3);
    uint8_t r = rgb & 0x00ff0000;
    uint8_t g = rgb & 0x0000ff00;
    uint8_t b = rgb & 0x000000ff;
    for(int i = 0; i < count; i++) {
        *dest++ = r;
        *dest++ = g;
        *dest++ = b;
    }
}

size_t vbe_regread(size_t index) {
    outportl(0x01CE, index);
    return inportl(0x01CF);
}

size_t* vbe_getfb() {
    return (size_t*)vbe_regread(13);
}

void vbe_putpixel(int x, int y, size_t color){
    size_t* ptr = gfxmode->physical_base_pointer;
    ptr[y * gfxmode->x_resolution + x] = color;
}

void vbe_putrect(int x1,int y1,int x2,int y2){
    size_t *ptr = gfxmode->physical_base_pointer;
    int x,y;
    for(x = x1;x < x2;x++)
      for(y = y1;y < y2;y++)
        ptr[y * gfxmode->x_resolution + x] = vbecolor;
}
