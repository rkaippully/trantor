/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully
*/

.macro def_bpb
  /*
    Boot Parameter Block definition
  */

oem_name:
  .ascii "TRANTOR "

bpb:

bpb_bytes_per_sec:
  .short 0              // Bytes per sector
  .byte  0              // Sectors per cluster
  .short 0              // Count of reserved sectors
  .byte  0              // Number of FATs
  .short 0              // Number of root dir entries
  .short 0              // Total logical sectors (16-bit)
  .byte  0              // Media descriptor
  .short 0              // Logical sectors per FAT (16-bit)
  .short 0              // Sectors per track
  .short 0              // Number of heads
  .long  0              // Number of hidden sectors
  .long  0              // Total logical sectors (32-bit)

ebpb:

  .long  0              // Logical sectors per FAT (32-bit)
  .short 0              // Extended flags
  .short 0              // Version number
  .long  0              // Root directory cluster number
  .short 0              // FSInfo sector number
  .short 0              // Sector number of backup boot record
  .fill  12, 1, 0       // Reserved
  .byte  0              // Physical drive number
  .byte  0              // Reserved
  .byte  0              // Extended boot signature
  .long  0              // Volume ID
  .ascii "NO NAME    "  // Volume label
  .ascii "FAT     "     // File system type
.endm

.macro relocate
	/*
	  Relocate boot sector/MBR to RELOC_ADDR and jump there
	*/

	// Set up stack
	cli
	cld
	xorw  %ax, %ax
	movw  %ax, %ss
	movw  $0x7c00, %sp

	// We are loaded at some address - most likely 0x7c00 but not all BIOSes do
	// that. Relocate ourselved to 0x500.
	pushw %cs
	popw  %ds
	call  1f
1:
	popw  %si
	subw  $(1b - start), %si    /* ds:si now points to the load address */
	movw  %ax, %es
	movw  $RELOC_ADDR, %di      /* es:di = 0x500 (relocation address) */
	movw  $512, %cx
	rep movsb %ds:(%si), %es:(%di)
	ljmp  $0, $1f
1:
	movw  %ax, %ds
.endm

.macro def_print_msg
  /*
	  Routine to print an error message to console and halt
	*/
print_msg:
	movb  $0x0e, %ah
	movw  $0x0007, %bx
1:
	lodsb %ds:(%si)
	andb  %al, %al
	jz    1f
	int   $0x10
	jmp   1b
1:
	hlt
	jmp   1b
.endm
