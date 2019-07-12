## *This is an archive of [this](https://github.com/nikolasjchaconas/CS270) repo*

# FFS

> Linux compliant filesystem written with libfuse

FFS is a rudimentary filesystem written using libfuse, which allows the filesystem to trap and handle any system calls which require disk I/O. It has its own `mkfs` function which is used to format a raw disk, allowing it to act as a secondary disk for any supported Linux distro.

## Important Notes

- The project is using a Makefile which auto-detects new .c/.h files so there is no need to edit it. This does mean, however, if you make a .c file real quick to test something, it will be included in compilation
- Take a look at global.h to get a feel for the naming semantics for variables, functions, etc.
- The output of the memory disk is decently large- keep this in mind
- Put anything you don't want being committed into the "temp" folder and try to cleanup executables/garbage before merging
- Take a look at online references for stdint.h- the size of values is very important and thus the OS uses strict sizings of uint(8, 16, 32, 64)_t for most things
- If you try to recompile and get bizarre errors, try "make clean" followed by "make" to recompile/link everything. Also reading the Makefile and understanding how it's compiling everything helps
- .txt and .o files are also ignored, edit the .gitignore file if you need to commit something with one of these extensions

## File System Size References

### Disk Version

- 4096 byte blocks
- 4 byte block numbers
- Inodes contain 8 direct blocks, 3 1st indirect blocks, and 5 2nd indirect blocks
- Blocks = 8 + 3 * (4096 / 4) + 5 * ((4096 / 4)^2) = 5,245,960
- Bytes = Blocks * 4096 = 21,487,452,160 B (approximately 20.011749 GiB)

### Memory Version

- 1024 byte blocks
- 4 byte block numbers
- Inodes contain 8 direct blocks, 3 1st indirect blocks, and 1 2nd indirect blocks
- Blocks = 8 + 3 * (1024 / 4) + 1 * ((1024 / 4)^2) = 66,312
- Bytes = Blocks * 1024 = 67,903,488 B (approximately 64.7578125 MiB)

## Disk Organization

The disk is organized into 5 sections: the super block, the free inode list, the free data block list, the inode list, and the data block list, in that order. While there are no blocks that aren't allocated to one of these 5 groups, some blocks may be only partially filled with data to make offsetting easier. The size of each section is compted in the following order/proportion due to the interdependencies of the various sections:
- Super Block: 1 block
- Inode Blocks: floor(0.1 * BLOCK_COUNT)
- Free Inode Blocks: ceiling(INODE_COUNT / (8 * BLOCK_SIZE))
- Free Data Blocks: ceiling((BLOCK_COUNT - (Super Block + Free Inode Blocks + Inode Blocks)) / (8 * BLOCK_SIZE))
- Data Blocks: BLOCK_COUNT - (Super Block + Inode Blocks + Free Inode Blocks + Free Data Blocks)

Each section will be contiguous within itself, but may have some empty space at the end of it's very last block. It's important to note that in the above equations, INODE_COUNT refers to the number of Inodes, not the number of Inode blocks. This is an intermediary calculation performed between finding the number of Inode blocks and Free Inode Blocks.

## Inode Nomenclature

- Each inode has a f_type field which specifies what type of object it points to
    - 0 is a file
    - 1 is a directory

## Stretch Optimizations

- Buffer cache
- TLB
- Reading in larger chunks from free list
- Don't read inodes when allocating
- Dynamically size key-value paris for directory data blocks with garbage collection helper
- Elements of ZFS functionality
- 