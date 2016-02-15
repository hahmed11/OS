/////////////////////////////////////////////////
//          ECSE 427 - Assignment 3            // 
//          Author- HABIB I. AHMED             // 
//              ID - 260464679                 //
//                sfs_api.c               	   //
//                                             //
/////////////////////////////////////////////////
//I would like to mention that I have spent countless hours on this assignment
//trying to figure out why I get seg faults. I hope I will not lose too many points
//because of that. I am fairly certain I did most things correctly.


#include "sfs_api.h"
#include "disk_emu.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

super_block_t sb;
dir_entry_t *root_dir;

inode_t *inode_table;
fd_table_t **fd_table;

int fileCount, dirLocation = 0;
unsigned short free_blocks[MAX_BLOCKS];

char file_data[] = "abcdefghijklmnopqrstuvwxyz";

// finds the next free bit
int search(){
  unsigned int *buffer = malloc(BLOCK_SIZE);

  if(buffer == NULL){
    printf("Error finding free bit\n");
    return -1;
  }

  read_blocks(1, FREELIST_SIZE, buffer);
  int i;
  for(i = 0; i < (BLOCK_SIZE)/sizeof(unsigned int); i++){
    int val = ffs(buffer[i]);
    if(val){
      if(val + i*8*sizeof(unsigned int) - 1 < BLOCK_SIZE)
        return val + i*8*sizeof(unsigned int) - 1;       //return correct value
    }
  }
  return -1;  //if invalid return -1 (failure)
}

//initializes the superblock
void init_superblock(){

    // init the superblock
    sb.magic = 1234;
    sb.block_size = BLOCK_SIZE;
    sb.fs_size = MAX_BLOCKS*BLOCK_SIZE;
    sb.inode_table_len = MAX_INODES;
    sb.root_dir_inode = 0;
}

/*void add_root_dir_inode(){

    //first entry in the inode table is the root
    inode_table[0].size = 45;
    inode_table[0].data_ptrs[0] = 2; //root dir is stored in the 3rd block


}	*/

/*void zero_everything(){

    bzero(&sb, sizeof(super_block_t));
    bzero(&fd_table[0], sizeof(fd_table_t)*MAX_FILES);
    bzero(&inode_table[0], sizeof(inode_t)*MAX_INODES);
    bzero(&root_dir, sizeof(dir_entry_t));
    bzero(&free_blocks[0], sizeof(unsigned int)*MAX_BLOCKS);

}	*/
//creates and initializes the inode table
void init_inode_table(){
	inode_t *buffer = malloc(ADJUST((MAX_FILES+1)*sizeof(inode_t)));

	if(buffer == NULL){
		return;
	}
	int i;
	//set up inode table
	for(i = 0; i < MAX_FILES+1; i++){
		buffer[i].exists = 0;                   // existence check
		buffer[i].link_cnt = 0;               	// link count
		buffer[i].size = 0;                     // size
		buffer[i].ref_ptr = LIMIT;      		 	// indirect pointer
		int j;
		for(j = 0; j < 12; j++){
			buffer[i].data_ptrs[j] = LIMIT;         //data/direct pointers
		}
	}
	write_blocks(INODE_TABLE, INODE_TABLE_SIZE, buffer);
	free(buffer);
	return;
}
//initializes the root directory
void init_root_dir(){
	dir_entry_t *buffer = malloc(ADJUST(MAX_FILES*sizeof(dir_entry_t)));

	if(buffer == NULL){
		return;
	}
	int i;
	for(i = 0; i < MAX_FILES; i++){
		//set up directory values
		buffer[i] = (dir_entry_t){.name = "\0",.inode_idx = LIMIT};
	}
	write_blocks(2, DIRECTORY_SIZE, buffer);
	free(buffer);
	return;
}
//method to create "free" list
void init_free_list(){
	unsigned int *buffer = malloc(BLOCK_SIZE);
	if (buffer == NULL){
		return;
	}
	int i;
	for(i = 0; i < (BLOCK_SIZE)/sizeof(unsigned int); i++){
		//sets all bits to 1
		buffer[i] = ~0;
	}
	write_blocks(1, FREELIST_SIZE, buffer);
	free(buffer);
	return;
}

//used to allocate to files in free list
void allocate(unsigned int index){
	int byte = index / (8*sizeof(unsigned int));  //find byte to change
	int bit = index % (8*sizeof(unsigned int));   //find bit to change
	unsigned int *buffer = malloc(BLOCK_SIZE);

	if(buffer == NULL){
		printf("Error assigning allocated bit\n");
		return;
	}
	if(index >= BLOCK_SIZE){
		printf("Error assigning allocated bit\n");
		return;
	}
	read_blocks(1, FREELIST_SIZE, buffer);
	buffer[byte] &= ~(1 << bit);    //set bit to 0 (allocated)
	write_blocks(1, FREELIST_SIZE, buffer);
	free(buffer);
}

//method to set bits to free (opposite of allocate())
void setFree(unsigned int index){
	if(index > MAX_BLOCKS){
		printf("Allocation error\n");
		return;
	}
	int byte = index / (8*sizeof(unsigned int));
	int bit = index % (8*sizeof(unsigned int));
	unsigned int *buffer = malloc(BLOCK_SIZE);

	if(buffer == NULL){
		printf("Error assigning free bit\n");
		return;
	}
	read_blocks(1, FREELIST_SIZE, buffer);
	buffer[byte] |= 1 << bit;
	write_blocks(1, FREELIST_SIZE, buffer);
	free(buffer);
}

void mksfs(int fresh){
	//Implement mksfs here
	if (fresh == 1){
		//deletes fs if it already exists
		if(access(DISK_FILE, F_OK) != -1){
			unlink(DISK_FILE);			
		}
		//fs creation
		printf("Initalizing sfs\n");
		init_fresh_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
        //zero_everything();
		// writes superblock to the first block
        printf("Writing superblocks\n");
        init_superblock();
		write_blocks(0, 1, &sb);
		// write the inode table to the 2nd block
        printf("Writing inode table\n");
		init_inode_table();
        //add_root_dir_inode();
        //add_dummy_file_inode();
		//write_blocks(1, 1, &inode_table);
		// creates free list
		init_free_list();
        // write root directory data to the 3rd block
        printf("Writing root dir\n");		
		init_root_dir();

		//allocate memory for directory inode
		inode_t *inode = malloc(ADJUST((MAX_FILES+1)*sizeof(inode_t)));
		read_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode);
		if(inode == NULL){
			return;
		}
		//set first inode to point to directory
		inode[0].size = DIRECTORY_SIZE*BLOCK_SIZE;
		inode[0].link_cnt = DIRECTORY_SIZE;
		inode[0].exists = 1;
		
		//check to see if we need to use ref_ptr
		//if(DIRECTORY_SIZE > 12){
			//inode[0].ref_ptr = search();
			//setAlloc(inode[0].ref_ptr);
			//unsigned int *buffer = malloc(BLOCK_SIZE);
			//write_blocks(19 + inode[0].ref_ptr, 1, buffer);
			//free(buffer);
		
		//assign the pointers the location of directory files
		int i;
		for(i = 0; i < DIRECTORY_SIZE; i++){
			if(i > 11){
				unsigned int *buffer = malloc(BLOCK_SIZE);
				read_blocks(19+ inode[0].ref_ptr, 1, buffer);
				buffer[i - 12] = 2 + i;
				write_blocks(19+ inode[0].ref_ptr, 1, buffer);
				free(buffer);
			} 
			else{
				inode[0].data_ptrs[i] = 2 + i;
			}
		}
		//update inode and free memory
		write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode);
		free(inode);		
	}
	
	else if (fresh == 0){
		if(init_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS) != 0){
			printf("Error initializing disk\n");
			return;
		}
	}
	//allocate main memory for filesystem data structures
	int *superblock = malloc(BLOCK_SIZE*SUPERBLOCK_SIZE);

	if(superblock == NULL){
		printf("Error allocating memory for superblock\n");
		return;
	}
	read_blocks(0, SUPERBLOCK_SIZE, superblock);
	//allocate main memory for directory
	root_dir = malloc(ADJUST(sizeof(dir_entry_t)*MAX_FILES));
	
	if(root_dir == NULL){
		printf("Error allocating memory for root directory\n");
		return;
	}
	read_blocks(2, DIRECTORY_SIZE, root_dir);
	//allocate memory for inode table
	inode_table = malloc(ADJUST(sizeof(inode_t)*(MAX_FILES+1)));
	
	if(inode_table == NULL){
		printf("Error allocating memory for inode table");
		return;
	}

	read_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode_table);
	fileCount = 0;
	fd_table = NULL;
	return;		
}
int sfs_getnextfilename(char *fname) {
	//printf("current index value is %d\n", dirLoc);
	// if end of directory return 0
	if(dirLocation == MAX_FILES){
		printf("Maximum number of files reached\n");
		return 0;
	}
	// copy file name and increase dirLocation value
	strncpy(fname, root_dir[dirLocation].name, MAXFILENAME); 
	dirLocation++;
	return 1;
}


int sfs_getfilesize(const char* path) {

	//Implement sfs_getfilesize here
    int i;
	printf("Path = %s\n", path);
	for(i = 0; i < MAX_FILES; i++){
		printf("root_dir[%d].name = %s\n", i, root_dir[i].name);
		//if(strncmp(root_dir[i].name, path, MAXFILENAME + 1) == 0)
		if(strcmp(root_dir[i].name, path) == 0){
			unsigned int size = inode_table[root_dir[i].inode_idx].size;
			return size;
        }
    }	
    return -1;
}

int sfs_fopen(char *name) {

	//Implement sfs_fopen here	

    // filename length check
	if(strlen(name) > MAXFILENAMELENGTH){
        printf("File Name too long\n");
        return -1;
    }
	
	if(root_dir == NULL){
		printf("File system not found");
		return -1;
	}
	
    int i, j, k;
    for(i = 0;i < MAX_FILES;i++){
        // checks if name matches
        if(strncmp(root_dir[i].name, name, MAXFILENAME + 1) == 0 ){
            //int currentFileIndex = root_dir[i].inode_idx - 1;
            // checks for valid inode
            if(root_dir[i].inode_idx == LIMIT){
                printf("iNode error\n");
                return -1;
            }
			int entry = -1;
            for(j = 0; j < fileCount; j++){
				// returns j if file is already open
                if(fd_table[j] && root_dir[i].inode_idx == fd_table[j]->inode_idx && root_dir[i].inode_idx != LIMIT){
					return j;
				}
			}
			for(j = 0; j < fileCount; j++){
				// entry in fd table
				if(fd_table[j] == NULL){
					fd_table[j] = malloc(sizeof(fd_table_t));
					entry = j;
					break;
				}
            }
			
			if(entry == -1){
				if(fd_table == NULL){
					fd_table = malloc(sizeof(fd_table_t*));
				}
				else{
					fd_table = realloc(fd_table, (1+fileCount)*(sizeof(fd_table_t*)));
					fd_table[fileCount] = (fd_table_t *) malloc(sizeof(fd_table_t));
					entry = fileCount;
					fileCount++;
				}
			}
			
			// new entry info
			fd_table_t *newEntry = fd_table[entry];
			if(newEntry == NULL){
				printf("File could not be opened\n");
				return -1;
			}
			newEntry->rd_write_ptr = inode_table[root_dir[i].inode_idx].size;
			newEntry->inode_idx = root_dir[i].inode_idx;
			return entry;
		}
	}

    // create file if it does not exist
	for(i = 0; i < MAX_FILES; i++){
		if(strncmp(root_dir[i].name, "\0", 1) == 0 && root_dir[i].inode_idx == LIMIT){
			int entry = -1;
			// entry in fd table
			for(k = 0; k < fileCount; k++){
				if(fd_table[k] == NULL){
					fd_table[k] = malloc(sizeof(fd_table_t));
					entry = k;
					break;
				}
			}

			if(entry == -1){
				if (fd_table == NULL){
					fd_table = malloc(sizeof(fd_table_t*));				
				}
			else{
				fd_table = realloc(fd_table, (1+fileCount)*(sizeof(fd_table_t*)));				
			}

			fd_table[fileCount] = (fd_table_t *) malloc(sizeof(fd_table_t));
			entry = fileCount;
			fileCount++;
			}

			// sets inode
			fd_table_t *newEntry = fd_table[entry];

			if(newEntry == NULL){
				printf("Error: cannot create new file\n");
				return -1;
			}
			int temp_inode = -1;
			int x;
			// looks for free inode
			for(x = 1; x < MAX_FILES+1; x++){
				if(inode_table[x].exists == 0){
				  inode_table[x].exists = 1;
				  temp_inode = x;
				  break;
				}
			}
		if(temp_inode == -1){
			printf("Inode error\n");
			return -1;
		}

		// looks for free location
		int freeLocation = search();

		if(freeLocation == -1){
			return -1;			  
		}

		allocate(freeLocation);
		// updates fd table entry
		newEntry->rd_write_ptr = 0;
		newEntry->inode_idx = temp_inode;

		// updates root directory entry
		strncpy(root_dir[i].name, name, MAXFILENAME+1);
		root_dir[i].inode_idx = temp_inode;
		write_blocks(2, DIRECTORY_SIZE, root_dir);

		//update inode
		inode_table[temp_inode].size = 0;
		inode_table[temp_inode].link_cnt = 1;
		inode_table[temp_inode].exists = 1;
		inode_table[temp_inode].data_ptrs[0] = freeLocation;
		write_blocks(INODE_TABLE,INODE_TABLE_SIZE,inode_table);
		return entry;
		}
	}
	return -1;  //return -1 on failure
}

int sfs_fclose(int fileID){

	//Implement sfs_fclose here	

    // already closed
	if(fileID > fileCount || fd_table[fileID] == NULL){
		printf("File not open\n");
		return -1;
	}
	free(fd_table[fileID]);
	fd_table[fileID] = NULL;
	return 0;
}

int sfs_fread(int fileID, char *buf, int length){

	//Implement sfs_fread here	

    // error check
    if(fd_table[fileID] == NULL || length < 0 || fileID > fileCount || buf == NULL){
        printf("Error reading file\n");
        return -1;
    }
	
    fd_table_t *read = fd_table[fileID];
    inode_t *temp_inode = &(inode_table[read->inode_idx]);


    if(read->inode_idx == LIMIT){
        printf("Inode error\n");
        return -1;
    }
    
    if(read->rd_write_ptr + length > temp_inode->size){
        length = temp_inode->size - read->rd_write_ptr;
    }

    char *buffer = malloc(BLOCK_SIZE);

    int readLength = length, offset = 0;
    int block = (read->rd_write_ptr)/BLOCK_SIZE;
    int bytes = (read->rd_write_ptr)%BLOCK_SIZE;
    int endOfBlock = (temp_inode->size)/BLOCK_SIZE;

    unsigned int readLocation;

    if(block > 139){
        printf("ERROR: Cannot read. File exceeds maximum size\n");
        return -1;
    }
    else if(block > 11){
        unsigned int *tempBuf = malloc(BLOCK_SIZE);
        read_blocks(19 + temp_inode->ref_ptr, 1, tempBuf);
        readLocation = tempBuf[block - 12];
        free(tempBuf);
    }
    else{
        readLocation = temp_inode->data_ptrs[block];
    }

    while(length > 0){
        read_blocks((19 + readLocation), 1, buffer);
		int bytesRead;

        if(BLOCK_SIZE - bytes < length){
            bytesRead = BLOCK_SIZE - bytes;
        }
        else{
            bytesRead = length;
        }

        memcpy(&buf[offset], &buffer[bytes], bytesRead);
        length -= (bytesRead);
        offset += (bytesRead);
        bytes = 0;

        if(length > 0){
            block++;
            if(endOfBlock < block){
                return -1;
            }
            if(block > 139){
                printf("ERROR: Cannot read. File exceeds maximum size\n");
                return -1;
            }
            else if(block > 11){
                unsigned int *nextBuffer = malloc(BLOCK_SIZE);
                read_blocks(19 + temp_inode->ref_ptr, 1, nextBuffer);
                readLocation = nextBuffer[block - 12];
                free(nextBuffer);
            }
            else{
                readLocation = temp_inode->data_ptrs[block];
            }
        }
    }

    free(buffer);
    read->rd_write_ptr += readLength;
    
    return readLength;
}

int sfs_fwrite(int fileID, const char *buf, int length){

	//Implement sfs_fwrite here
	
    // error check
    if(fd_table[fileID] == NULL || length < 0 || fileID > fileCount || buf == NULL){
        printf("Error reading file\n");
        return -1;
    }
	
	fd_table_t *write = fd_table[fileID];
	inode_t *temp_inode = &(inode_table[write->inode_idx]);
	
	// check fd table entry
    if(write->inode_idx == LIMIT){
		printf("Inode error");
		return -1;
	}
	  
    char *buffer = malloc(BLOCK_SIZE);

    int writeLength = length, offset = 0;
    int block = (write->rd_write_ptr)/BLOCK_SIZE;
    int bytes = (write->rd_write_ptr)%BLOCK_SIZE;
    int endOfBlock = (temp_inode->size)/BLOCK_SIZE;
    int bytesToWrite;

    unsigned int writeLocation;	  
	
	// max file size error check
    if(block > 139){
        printf("ERROR: Cannot write. File exceeds maximum size\n");
        return -1;
    }
	
	// retrieves block location using ref_ptr
    else if(block > 11){
        unsigned int *tempBuf = malloc(BLOCK_SIZE);
        read_blocks(19 + temp_inode->ref_ptr, 1, tempBuf);
        writeLocation = tempBuf[block - 12];
        free(tempBuf);
    }	
	// uses data pointers if ref_ptr search fails
    else{
        writeLocation = temp_inode->data_ptrs[block];
    }	

	if(writeLocation != -1){
		while(length > 0){ 
			read_blocks((19 + writeLocation), 1, buffer);
			if(BLOCK_SIZE - bytes < length){
				bytesToWrite = BLOCK_SIZE - bytes;
			}
			else{
				bytesToWrite = length;			
			}
			memcpy(&buffer[bytes], &buf[offset], bytesToWrite);
			write_blocks(19 + writeLocation, 1, buffer);
	
			length -= (bytesToWrite);
			offset += (bytesToWrite);
			bytes = 0;
			block++;
		
			if(length > 0){
				if(block > 139){
					printf("ERROR: Cannot write. File exceeds maximum size\n");
					return -1;
				}
				else if(endOfBlock < block){ 
					if(block == 12 && temp_inode->ref_ptr == LIMIT){
						int searchPtr = search();
						allocate(searchPtr);
						temp_inode->ref_ptr = searchPtr;
					}
					int nextLocation = search();
					allocate(nextLocation);
					if(nextLocation == -1){
						return -1;
					}
					writeLocation = nextLocation;
					if(block > 11){
						unsigned int *nextBuffer = malloc(BLOCK_SIZE);
						read_blocks(19+ temp_inode->ref_ptr, 1, nextBuffer);
						nextBuffer[block - 12] = writeLocation;
						write_blocks(19+ temp_inode->ref_ptr, 1, nextBuffer);
						free(nextBuffer);
					}
					else{
						temp_inode->data_ptrs[block] = writeLocation;					
					}
				
					temp_inode->link_cnt++; // updates link count. Not sure if necessary
				}
				else{
					if(block > 11){
						unsigned int *nextBuffer = malloc(BLOCK_SIZE);
						read_blocks(19+ temp_inode->ref_ptr, 1, nextBuffer);
						writeLocation = nextBuffer[block - 12];
						free(nextBuffer);
					}
					else{
						writeLocation = temp_inode->data_ptrs[block];					
					}
				}
			}
		}
	}
	// update filesize in inode table
	if(write->rd_write_ptr + writeLength > temp_inode->size){
		temp_inode->size = write->rd_write_ptr + writeLength;
	}
	// update read/write pointer
	write->rd_write_ptr += writeLength;
	// update inode table
	write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode_table);
	free(buffer);
	// returns length of written data
	return writeLength;
}

int sfs_fseek(int fileID, int loc){
        
    if(fileID >= fileCount || fd_table[fileID] == NULL || inode_table[fd_table[fileID]->inode_idx].size < loc){
        printf("Seek Error\n");
		return -1;
    }
	
	fd_table[fileID]->rd_write_ptr = loc;
	return 0;
}

int sfs_remove(char *file) {
    
    int i, j, k;

    // looks for 'file'
    for(i = 0;i < MAX_FILES;i++){
		if(strncmp(root_dir[i].name, file, MAXFILENAME) == 0 && root_dir[i].inode_idx != LIMIT){
			// updates root directory
			dir_entry_t *remove = &(root_dir[i]);
			int temp_inode = remove->inode_idx;
			strcpy(remove->name, "\0");
			inode_t *inodeRemove = &(inode_table[temp_inode]);
			remove->inode_idx = LIMIT;
			if(inodeRemove->link_cnt > 12){
				unsigned int *tempBuf = malloc(BLOCK_SIZE);
				read_blocks(19+ inodeRemove->ref_ptr, 1, tempBuf);
				//updates inode link_cnt
				for(j = 0; j < inodeRemove->link_cnt - 12; j++){
					setFree(tempBuf[j]);
				}
				free(tempBuf);
				inodeRemove->link_cnt = inodeRemove->link_cnt - 12;
				setFree(inodeRemove->ref_ptr);
				inodeRemove->ref_ptr = LIMIT;
			}
			for(k = 0; k < 12; k++){
				setFree(inodeRemove->data_ptrs[k]);
				inodeRemove->data_ptrs[k] = LIMIT;
			}
			// updates inode data
			inodeRemove->exists = 0;
			inodeRemove->link_cnt = 0;
			inodeRemove->size = 0;
			write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode_table);
			return 0;
		}
	}
	printf("File not found\n");
	return -1;
}