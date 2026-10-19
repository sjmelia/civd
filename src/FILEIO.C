/* Image and file loading, including resources in archives. */
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DRIVER.H"
#include "F0DATA.H"
#include "PICTURE.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "FILEIO.H"

struct archive_entry {
    char name[12];
    int reserved[4];
    long offset;
};
struct sprite_header {
    unsigned char left, right, planes, flags, height, reserved, extra, spare;
};
extern int (*pic_reader)();
extern struct sprite_header sprite_header;
extern char far *font_address();

void load_palette();            /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

int archive_find(char *, struct archive_entry *);

int resource_open(char *name, int mode)
{
    struct archive_entry entry;
    int file;
    if (archive_find(name, &entry)) {
        if (file_seek(archive_handle, entry.offset))
            file_error(name);
        return archive_handle;
    }
    if (_dos_open(name, mode, &file))
        file_error(name);
    return file;
}

int file_create(char *name)
{
    int file;
    if (_dos_creat(name, 0, &file))
        file_error(name);
    return file;
}

void resource_close(int file)
{
    if (file != archive_handle && _dos_close(file))
        file_error(0);
}

int resource_read(char *name, char *buffer)
{
    int count, handle;
    handle = resource_open(name, 0);
    if (_dos_read(handle, buffer, 65535U, &count))
        file_error(0);
    resource_close(handle);
    return count;
}

int file_read_far(char *name, char far *buffer)
{
    int count, handle;
    handle = resource_open(name, 0);
    if (_dos_read(handle, buffer, 65535U, &count))
        file_error(0);
    resource_close(handle);
    return count;
}

int resource_write(char *name, char far *buffer, unsigned int count)
{
    int file;
    file = file_create(name);
    if (_dos_write(file, buffer, count, &count))
        file_error(0);
    resource_close(file);
    return count;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
/* mode: where a picture's palette goes. 0 keeps it unapplied in msgtext,
 * 1 applies it, any other value is the buffer to copy it to. */
void LOAD_PIC(int page, int x, int y, char *name, char *mode)
{
    int row, handle;
    handle = resource_open(name, 0);
    set_file_reader(handle);
    picture_target(mode);
    if (page < 0)
        picture_height = 0;
    for (row = 0; row < picture_height; row++) {
        decode_scanline(unpack_raster);
        draw_scanline(unpack_raster, page, x, y + row, picture_width);
    }
    resource_close(handle);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void load_picture(int page, char *name)
{
    LOAD_PIC(page, 0, 0, name, (char *) 1);
}

void load_picture_mode(int page, char *name, char *mode)
{
    LOAD_PIC(page, 0, 0, name, mode);
}

void load_screen(int page, char *name)
{
    LOAD_PIC(page, 0, 0, name, 0);
}

void load_back(char *name)
{
    LOAD_PIC(-1, 0, 0, name, (char *) 1);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_palette(name, mode)
char *name;
char *mode;
{
    LOAD_PIC(-1, 0, 0, name, mode);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int load_block(unsigned int segment, unsigned int offset, char *name)
{
    int file;
    file = resource_open(name, 0);
    set_file_reader(file);
    begin_picture();
    copy_picture(segment, offset, picture_bytes);
    resource_close(file);
    return picture_bytes;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int load_tile(int tile, int index, char *name)
{
    unsigned int width, num;
    int bytes;
    char far *address;
    int handle, count;
    if (_dos_open(name, 0, &handle))
        return image_error;
    if (_dos_read(handle, &num, 2, &count))
        return image_error;
    image_error = 2;
    if (index > num)
        return 2;
    if (file_seek(handle, (long) (index * 2)))
        return image_error;
    if (_dos_read(handle, &num, 2, &count))
        return image_error;
    num -= 8;
    if (file_seek(handle, num))
        return image_error;
    if (_dos_read(handle, &sprite_header, 8, &count))
        return image_error;
    width = sprite_header.right - sprite_header.left + 1;
    bytes = width * sprite_header.planes * (sprite_header.height + sprite_header.extra) + 8;
    address = font_address(tile) - 8;
    if (!sprite_header.flags) {
        address -= width;
        num -= width;
        bytes += width;
    }
    if (file_seek(handle, num))
        return image_error;
    if (_dos_read(handle, (char far *) address, bytes, &count))
        return image_error;
    if (_dos_close(handle))
        return image_error;
    return 0;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_cursor(char *name)
{
    int row, handle;
    handle = resource_open(name, 0);
    set_file_reader(handle);
    picture_target(0);
    sprite_begin(picture_width, picture_height);
    for (row = 0; row < picture_height; row++) {
        decode_scanline(unpack_raster);
        sprite_encode_row(unpack_raster);
    }
    resource_close(handle);
    sprite_end();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void LOAD_SPRITES(char *name, int page)
{
    load_picture(page, name);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_rows(char *name, int page)
{
    int row, handle;
    handle = resource_open(name, 0);
    set_file_reader(handle);
    picture_target(0);
    for (row = 0; row < picture_height; row++) {
        decode_scanline(unpack_raster);
        draw_scanline(unpack_raster, page, 0, row, picture_width);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_memory_rows(int segment, int page)
{
    int row;
    source_offset = 0;
    source_segment = segment;
    file_read = image_buffer;
    pic_reader = memory_reader;
    picture_target(0);
    for (row = 0; row < picture_height; row++) {
        decode_scanline(unpack_raster);
        draw_scanline(unpack_raster, page, 0, row, picture_width);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int load_memory_block(int offset, int unused, int destination, int segment)
{
    int dummy;
    source_offset = offset;
    source_segment = segment;
    file_read = image_buffer;
    pic_reader = memory_reader;
    begin_picture();
    copy_picture(destination, segment, picture_bytes);
    return picture_bytes;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void set_file_reader(int file)
{
    image_file = file;
    file_read = image_buffer;
    pic_reader = read_next_block;
}

int memory_reader(void)
{
    char far *buffer;
    buffer = diskbuffer;
    movedata(source_segment, source_offset, FP_SEG(buffer), FP_OFF(buffer), 512);
    file_read = diskbuffer;
    source_offset += 512;
    return 512;
}

int read_first_block(int file)
{
    image_file = file;
    return read_next_block();
}

int read_next_block(void)
{
    int count;
    _dos_read(image_file, diskbuffer, 512, &count);
    file_read = diskbuffer;
    return count;
}

void file_error(char *name)
{
    file_video_mode(3);
    perror(name);
    exit(99);
}

void file_video_mode(int mode)
{
    int saved;
    union REGS regs;
    saved = image_error;
    regs.h.ah = 0;
    regs.h.al = mode;
    int86(16, &regs, &regs);
    image_error = saved;
}

int archive_open(char *name)
{
    int file;
    if (_dos_open(name, 0x8000, &file))
        return 0;
    archive_handle = file;
    archive_index = 0;
    return 1;
}

void archive_close(void)
{
    if (archive_handle != -1) {
        if (_dos_close(archive_handle))
            file_error(0);
        archive_handle = -1;
    }
}

int archive_find(char *name, struct archive_entry *entry)
{
    int number, count;
    if (archive_handle == -1)
        return 0;
    if (file_seek(archive_handle, 0L))
        file_error(0);
    if (_dos_read(archive_handle, &number, 2, &count))
        file_error(0);
    if (name == 0) {
        archive_index++;
        if (number < archive_index)
            return 0;
        number = archive_index;
        while (number--) {
            if (_dos_read(archive_handle, entry, 24, &count))
                file_error(0);
        }
        return 1;
    }
    while (number--) {
        if (_dos_read(archive_handle, entry, 24, &count))
            file_error(0);
        if (!strnicmp(entry->name, name, 12))
            return 1;
    }
    return 0;
}

int file_seek(int file, long offset)
{
    union REGS regs;
    regs.x.ax = 0x4200;
    regs.x.bx = file;
    regs.x.cx = (unsigned long) offset >> 16;
    regs.x.dx = offset;
    intdos(&regs, &regs);
    if (!regs.x.cflag)
        return 0;
    return regs.x.ax;
}
