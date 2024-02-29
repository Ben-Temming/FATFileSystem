/*
 * Name: Ben Temming 
 * Student ID: 52094132
 */


/* filesys.h
 * 
 * describes FAT structures
 * http://www.c-jump.com/CIS24/Slides/FAT/lecture.html#F01_0020_fat
 * http://www.tavi.co.uk/phobos/fat.html
 */

#ifndef FILESYS_H
#define FILESYS_H

#include <time.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAXBLOCKS     1024
#define BLOCKSIZE     1024
#define FATENTRYCOUNT (BLOCKSIZE / sizeof(fatentry_t))
#define DIRENTRYCOUNT ((BLOCKSIZE - (2*sizeof(int)) ) / sizeof(direntry_t))
#define MAXNAME       256
#define MAXPATHLENGTH 1024

#define UNUSED        -1
#define ENDOFCHAIN     0
// #define EOF           -1 //gives redefinition warning as this is alread implemented in the standard C library


typedef unsigned char Byte ;

/* create a type fatentry_t, we set this currently to short (16-bit)
 */
typedef short fatentry_t ;


// a FAT block is a list of 16-bit entries that form a chain of disk addresses

//const int   fatentrycount = (blocksize / sizeof(fatentry_t)) ;

typedef fatentry_t fatblock_t [ FATENTRYCOUNT ] ;


/* create a type direntry_t
 */

typedef struct direntry {
   int         entrylength ;   // records length of this entry (can be used with names of variables length)
   Byte        isdir ;
   Byte        unused ;
   time_t      modtime ;
   int         filelength ;
   fatentry_t  firstblock ;
   char   name [MAXNAME] ;
} direntry_t ;

// a directory block is an array of directory entries

//const int   direntrycount = (blocksize - (2*sizeof(int)) ) / sizeof(direntry_t) ;

typedef struct dirblock {
   int isdir ;
   int nextEntry ;
   direntry_t entrylist [ DIRENTRYCOUNT ] ; // the first two integer are marker and endpos
} dirblock_t ;



// a data block holds the actual data of a filelength, it is an array of 8-bit (byte) elements

typedef Byte datablock_t [ BLOCKSIZE ] ;


// a diskblock can be either a directory block, a FAT block or actual data

typedef union block {
   datablock_t data ;
   dirblock_t  dir  ;
   fatblock_t  fat  ;
} diskblock_t ;

// finally, this is the disk: a list of diskblocks
// the disk is declared as extern, as it is shared in the program
// it has to be defined in the main program filelength

extern diskblock_t virtualDisk [ MAXBLOCKS ] ;


// when a file is opened on this disk, a file handle has to be
// created in the opening program

typedef struct filedescriptor {
   char        mode[3] ;
   fatentry_t  dirblockno; //block of the directrory 
   fatentry_t  firstblockno; // first block of file 
   fatentry_t  blockno ;           // current block no
   int         pos     ;           // byte within a block
   diskblock_t buffer  ;
} MyFILE ;



void format() ;
void writedisk ( const char * filename ) ;
void printBlock( int blockIndex); 


//C3-C1 functions
MyFILE* myfopen(const char *filename, const char *mode); 
void myfclose(MyFILE *stream); 
int myfgetc(MyFILE *stream); //return the next of the open file 
void myfputc(int b, MyFILE *stream); //writes byte to file

//B3-B1 functions 
void mymkdir(const char* path); 
char **mylistdir(const char* path); 

//A5-A4 functions 
void mychdir(const char* path);
void myremove(const char* path); 
void myremdir(const char* path); 

//A3 functions 
void copy_virtual_real(const char* virtual_path, const char* real_filename, int to_virtual);

//A2 functions 
void copy_file(const char* source_path, const char* dest_path); 
void move_file(const char* source_path, const char* dest_dir); 


//helper functions
void copyFAT(); 
int getFreeFATBlockIndex(); 
fatentry_t getFreeDirEntryBlock(fatentry_t dir_index);
void readblock(diskblock_t *block, int block_address); 
int get_num_blocks(const int start_block); 
int isEndOfFile(const int start_block, const int current_block, int current_pos, int dir_block_index);
fatentry_t create_dir(fatentry_t dir_index, char* new_dir_name); 


//test functin 
void print_current_dir_content(); 


#endif

/*
#define NUM_TYPES (sizeof types / sizeof types[0])
static* int types[] = { 
    1,
    2, 
    3, 
    4 };
*/