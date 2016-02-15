/////////////////////////////////////////////////
//          ECSE 427 - Assignment 3            // 
//          Author- HABIB I. AHMED             // 
//              ID - 260464679                 //
//                sfs_api.h               	   //
//                                             //
/////////////////////////////////////////////////
#include "disk_emu.h"
#define DISK_FILE "habib.disk"
#define MAXFILENAMELENGTH 16
#define MAX_INODE_SIZE 4
#define BLOCK_SIZE 512
#define MAX_FILES 150
#define MAX_INODES 150
#define MAXFILENAME 60                     
					   
#define SUPERBLOCK_SIZE 1                
#define FREELIST_SIZE 1                    
#define DIRECTORY_SIZE 4            
#define INODE_TABLE 6
#define INODE_TABLE_SIZE 13
#define MAX_BLOCKS SUPERBLOCK_SIZE + FREELIST_SIZE + DIRECTORY_SIZE + INODE_TABLE_SIZE + BLOCK_SIZE
#define LIMIT 2500  
#define ADJUST(x)((x/BLOCK_SIZE + 1) * BLOCK_SIZE)

typedef struct super_block {
    unsigned int magic;
    unsigned int block_size;
    unsigned int fs_size;
    unsigned int inode_table_len;
    unsigned int root_dir_inode;
} super_block_t;


typedef struct inode { 
	unsigned int exists;
	unsigned int link_cnt;
    unsigned int size;
    unsigned int data_ptrs[12];
    unsigned int ref_ptr;
} inode_t;


typedef struct dir_entry { 
    char name[MAXFILENAME];
    unsigned int inode_idx;
} dir_entry_t;


typedef struct fd_table { 
    unsigned int inode_idx;
    unsigned int status;
    unsigned int rd_write_ptr;
    unsigned int exists;
} fd_table_t;

void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);

int search();
void init_superblock();                     
void init_inode_table();
void init_root_dir();      
void init_free_list();      
void allocate(unsigned int index);     
void setFree();   


