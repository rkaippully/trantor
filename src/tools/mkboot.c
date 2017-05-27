/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  Makes a Trantor boot disk by setting up boot data structures
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct partition_entry {
  uint8_t  drive_num;
  uint8_t  start_head;
  uint8_t  start_sec;
  uint8_t  start_cyl;
  uint8_t  type;
  uint8_t  end_head;
  uint8_t  end_sec;
  uint8_t  end_cyl;
  uint32_t lba_start;
  uint32_t lba_count;
} __attribute__((packed));

struct tbh_chs_entry {
  uint8_t count;
  uint8_t sector;
  uint8_t cylinder;
  uint8_t head;
} __attribute__((packed));

struct tbh_lba_entry {
  uint16_t count;
  uint32_t start;
} __attribute__((packed));

struct bpb {
  uint8_t  oem_name[8];
  uint16_t bytes_per_sec;
  uint8_t  secs_per_cluster;
  uint16_t reserved_secs;
  uint8_t  num_fats;
  uint16_t root_entries;
  uint16_t total_secs_16;
  uint8_t  media_desc;
  uint16_t secs_per_fat_16;
  uint16_t secs_per_track;
  uint16_t heads;
  uint32_t hidden_secs;
  uint32_t total_secs_32;
  union {
    // FAT12 or FAT16 structure
    struct {
      uint8_t  drive_num;
      uint8_t  reserved_1;
      uint8_t  ext_boot_sig;
      uint32_t serial;
      uint8_t  vol_label[11];
      uint8_t  fs_type[8];
    } __attribute__((packed));
    // FAT32 structure
    struct {
      uint32_t secs_per_fat_32;
      uint16_t ext_flags;
      uint16_t fs_version;
      uint32_t root_dir_cluster;
      uint16_t fs_info_sec;
      uint16_t backup_boot_sec;
      uint8_t  reserved_2[12];
      uint8_t  drive_num_32;
      uint8_t  reserved_1_32;
      uint8_t  ext_boot_sig_32;
      uint32_t serial_32;
      uint8_t  vol_label_32[11];
      uint8_t  fs_type_32[8];
    } __attribute__((packed));
  } __attribute__((packed));
} __attribute__((packed));

typedef enum {
  FAT12, FAT16, FAT32
} FATType;

struct dir_entry {
  uint8_t  filename[11];
  uint8_t  attributes;
  uint8_t  reserved_1;
  uint8_t  create_time_ms;
  uint16_t create_time;
  uint16_t create_date;
  uint16_t access_date;
  uint16_t first_cluster_high;
  uint16_t write_time;
  uint16_t write_date;
  uint16_t first_cluster_low;
  uint32_t file_size;
} __attribute__((packed));

#define min(x, y) ((x) < (y) ? (x) : (y))
#define ceil(m, n) (((m) + ((n) - 1)) / (n))

static void usage(const char* prog)
{
  fprintf(stderr, "Usage: %s <disk image> <chs image> <lba image> <mbr image>\n", prog);
  exit(1);
}

// Offset in disk image to the boot sector
static uint32_t boot_sec_offset = 0;

static void update_mbr(FILE* disk_img, FILE* mbr_img)
{
  uint8_t disk_buf[512];
  if (fread(disk_buf, 1, 512, disk_img) != 512) {
    fprintf(stderr, "Error reading disk image\n");
    exit(1);
  }
  // Is it a JMP instruction?
  if (disk_buf[0] == 0xeb || disk_buf[0] == 0xe9) {
    // This is not an MBR
    boot_sec_offset = 0;
    return;
  }

  // It is an MBR
  uint8_t mbr_buf[512];
  if (fread(mbr_buf, 1, 512, mbr_img) != 512) {
    fprintf(stderr, "Error reading mbr image\n");
    exit(1);
  }
  struct partition_entry* entry = (struct partition_entry*)(disk_buf+0x01be);
  if (entry->type != 0x06 && entry->type != 0x0c) {
    fprintf(stderr, "Invalid partition entry type\n");
    exit(1);
  }
  memcpy(mbr_buf+0x01be, entry, 4*sizeof(struct partition_entry));
  // Write it back
  fseek(disk_img, 0, SEEK_SET);
  if (fwrite(mbr_buf, 1, 512, disk_img) != 512) {
    fprintf(stderr, "Error updating mbr\n");
    exit(1);
  }
  boot_sec_offset = entry->lba_start * 512;
}

static void* get_boot_sector(FILE* disk_img)
{
  void* buf = malloc(512);
  fseek(disk_img, boot_sec_offset, SEEK_SET);
  if (fread(buf, 1, 512, disk_img) != 512) {
    fprintf(stderr, "Error reading boot sector\n");
    exit(1);
  }
  return buf;
}

// We assume that the kernel is located in the first 4 sectors of FAT
static const size_t FAT_SECS_READ = 4;

static void* get_fat(FILE* disk_img, struct bpb* bpb)
{
  size_t sz = FAT_SECS_READ * bpb->bytes_per_sec;
  void* fat = malloc(sz);
  fseek(disk_img, boot_sec_offset + bpb->reserved_secs * bpb->bytes_per_sec, SEEK_SET);
  if (fread(fat, sz, 1, disk_img) != 1) {
    fprintf(stderr, "Error reading FAT\n");
    exit(1);
  }
  return fat;
}

static struct dir_entry* find_kernel_dir_entry(FILE* disk_img, struct bpb* bpb)
{
  uint32_t fat_secs = bpb->secs_per_fat_16 ? bpb->secs_per_fat_16 : bpb->secs_per_fat_32;
  uint32_t dir_offset = boot_sec_offset + (bpb->reserved_secs + bpb->num_fats * fat_secs) * bpb->bytes_per_sec;
  fseek(disk_img, dir_offset, SEEK_SET);

  size_t sz = sizeof(struct dir_entry);
  struct dir_entry* entry = malloc(sz);
  if (fread(entry, sz, 1, disk_img) != 1) {
    fprintf(stderr, "Error reading directory entry\n");
    exit(1);
  }
  if (memcmp(entry->filename, "TRANTOR SYS", 11)) {
    fprintf(stderr, "Kernel directory entry not found\n");
    exit(1);
  }
  entry->attributes = 0x27; // Read-only, hidden, system, archive

  // Write it back
  fseek(disk_img, dir_offset, SEEK_SET);
  if (fwrite(entry, sz, 1, disk_img) != 1) {
    fprintf(stderr, "Error updating directory entry\n");
    exit(1);
  }

  return entry;
}

static FATType get_fat_type(struct bpb* bpb)
{
  uint32_t root_dir_secs = ceil(bpb->root_entries * 32, bpb->bytes_per_sec);
  uint32_t secs_per_fat = bpb->secs_per_fat_16 ? bpb->secs_per_fat_16 : bpb->secs_per_fat_32;
  uint32_t total_secs = bpb->total_secs_16 ? bpb->total_secs_16 : bpb->total_secs_32;
  uint32_t total_data_secs = total_secs - (bpb->reserved_secs + (bpb->num_fats * secs_per_fat) + root_dir_secs);
  uint32_t data_clusters = total_data_secs / bpb->secs_per_cluster;
  if (data_clusters < 4085)
    return FAT12;
  else if (data_clusters < 65525)
    return FAT16;
  else
    return FAT32;
}

static void* find_tbh_entry(void* boot_sec, FATType type)
{
  uint16_t version_and_format = type == FAT12 ? 0x0000 : 0x0100;
  uint16_t *ptr = boot_sec+0x40, *end = boot_sec+0x200;
  while (ptr < end) {
    if (*ptr == 0x5254 && *(ptr+1) == 0x5254 && *(ptr+2) == 0x5254 && // "TRTRTR"
        *(ptr+3) == version_and_format)
      return ptr+4;
    ptr += 2;
  }

  fprintf(stderr, "Could not find Trantor boot header\n");
  exit(1);
}

static int is_end_of_chain(uint32_t idx, FATType type)
{
  switch(type) {
  case FAT12:
    return idx < 2 || idx > 0xfef;
  case FAT16:
    return idx < 2 || idx > 0xffef;
  case FAT32:
    return idx < 2 || idx > 0x0fffffef;
  default:
    fprintf(stderr, "Illegal FAT type\n");
    exit(1);
  }
}

static uint32_t get_fat_entry(void* fat, FATType type, uint32_t idx)
{
  uint32_t* lv;
  uint16_t* sv;
  switch(type) {
  case FAT12:
    lv = fat + idx*3/2;
    if (idx/2)
      return *lv & 0xfff;
    else
      return (*lv >> 4) & 0xfff;
  case FAT16:
    sv = fat;
    return sv[idx];
  case FAT32:
    lv = fat;
    return lv[idx] & 0x0fffffff;
  default:
    fprintf(stderr, "Illegal FAT type\n");
    exit(1);
  }
}

static struct tbh_lba_entry* next_lba_run(struct bpb* bpb, void* fat, FATType type, uint32_t idx)
{
  static uint32_t lba_run_idx = 0;
  static struct tbh_lba_entry entry;

  if (idx != 0)
    lba_run_idx = idx;
  else
    idx = lba_run_idx;

  uint32_t cnt = 0;
  uint32_t p = lba_run_idx;
  while(1) {
    if (is_end_of_chain(p, type))
      break;

    cnt++;
    p = get_fat_entry(fat, type, p);
    if (p == lba_run_idx + 1)
      lba_run_idx = p;
    else {
      lba_run_idx = p;
      break;
    }
  }

  if (cnt == 0)
    return NULL;

  uint32_t fat_secs = bpb->secs_per_fat_16 ? bpb->secs_per_fat_16 : bpb->secs_per_fat_32;
  uint32_t root_dir_secs = ceil(bpb->root_entries * 32, bpb->bytes_per_sec);
  entry.start = boot_sec_offset/bpb->bytes_per_sec + bpb->reserved_secs + bpb->num_fats*fat_secs +
    root_dir_secs + (idx - 2)*bpb->secs_per_cluster;
  entry.count = cnt * bpb->secs_per_cluster;
  return &entry;
}

static struct tbh_chs_entry* next_chs_run(struct bpb* bpb, struct tbh_lba_entry* lba_run)
{
  static struct tbh_lba_entry lba = {0, 0};
  static struct tbh_chs_entry entry;

  if (lba_run != NULL)
    lba = *lba_run;

  uint8_t s = (lba.start % bpb->secs_per_track) + 1;
  uint8_t h = (lba.start / bpb->secs_per_track) % bpb->heads;
  uint16_t c = lba.start / (bpb->heads * bpb->secs_per_track);
  if (lba.count == 0)
    return NULL;
  else {
    entry.count = min(bpb->secs_per_track - s + 1, lba.count);
    entry.sector = s | ((c >> 2) & 0xc0);
    entry.cylinder = c & 0xff;
    entry.head = h;

    lba.count -= entry.count;
    lba.start += entry.count;

    return &entry;
  }
}

static void update_tbh_chs(void* boot_sec, struct bpb* bpb,
                           void* fat, FATType type, struct dir_entry* entry)
{
  struct tbh_chs_entry* ptr = find_tbh_entry(boot_sec, type);
  uint32_t idx = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
  struct tbh_lba_entry* lba_run = next_lba_run(bpb, fat, type, idx);
  while (lba_run != NULL) {
    struct tbh_chs_entry* chs_run = next_chs_run(bpb, lba_run);
    while (chs_run != NULL) {
      *ptr++ = *chs_run;
      chs_run = next_chs_run(bpb, NULL);
    }
    lba_run = next_lba_run(bpb, fat, type, 0);
  }
  ptr->count = 0;
}

static void update_tbh_lba(void* boot_sec, struct bpb* bpb,
                           void* fat, FATType type, struct dir_entry* entry)
{
  struct tbh_lba_entry* ptr = find_tbh_entry(boot_sec, type);
  uint32_t idx = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
  struct tbh_lba_entry* lba_run = next_lba_run(bpb, fat, type, idx);
  while (lba_run != NULL) {
    *ptr++ = *lba_run;
    lba_run = next_lba_run(bpb, fat, type, 0);
  }
  ptr->count = 0;
}

static void* update_bpb_and_tbh(FILE* img_file, struct bpb* bpb, void* fat, FATType type, struct dir_entry* entry,
                                void (*update)(void*, struct bpb*, void*, FATType, struct dir_entry*))
{
  uint8_t* new_boot_sec = malloc(512);
  if (fread(new_boot_sec, 1, 512, img_file) != 512) {
    fprintf(stderr, "Error reading boot sector image\n");
    exit(1);
  }
  struct bpb* new_bpb = (struct bpb*)(new_boot_sec+3);
  memcpy(new_bpb, bpb, sizeof(struct bpb));
  memcpy(new_bpb->oem_name, "TRANTOR ", 8);
  if (type == FAT32)
    memcpy(new_bpb->vol_label_32, "TrantorBoot", 11);
  else
    memcpy(new_bpb->vol_label, "TrantorBoot", 11);
  update(new_boot_sec, new_bpb, fat, type, entry);

  return new_boot_sec;
}

static void update_boot_sector(FILE* disk_img, FILE* chs_img, FILE* lba_img,
                               void* boot_sec, struct bpb* bpb,
                               void* fat, struct dir_entry* entry)
{
  FATType type = get_fat_type(bpb);
  void* new_boot_sec;
  switch(type) {
  case FAT12:
    new_boot_sec = update_bpb_and_tbh(chs_img, bpb, fat, type, entry, update_tbh_chs);
    break;
  case FAT16:
  case FAT32:
    new_boot_sec = update_bpb_and_tbh(lba_img, bpb, fat, type, entry, update_tbh_lba);
    break;
  }

  // Write it back
  fseek(disk_img, boot_sec_offset, SEEK_SET);
  if (fwrite(new_boot_sec, 1, 512, disk_img) != 512) {
    fprintf(stderr, "Error updating boot sector\n");
    exit(1);
  }
}

int main(int argc, char** argv)
{
  if (argc != 5)
    usage(argv[0]);

  FILE* disk_img = fopen(argv[1], "r+b");
  FILE* chs_img = fopen(argv[2], "rb");
  FILE* lba_img = fopen(argv[3], "rb");
  FILE* mbr_img = fopen(argv[4], "rb");

  update_mbr(disk_img, mbr_img);

  void* boot_sec = get_boot_sector(disk_img);
  struct bpb* bpb = boot_sec+3;
  void* fat = get_fat(disk_img, bpb);

  struct dir_entry* entry = find_kernel_dir_entry(disk_img, bpb);
  update_boot_sector(disk_img, chs_img, lba_img, boot_sec, bpb, fat, entry);

  return 0;
}
