/////////////////////////////////////////////////
//          ECSE 427 - Assignment 4            //
//          Author - HABIB I. AHMED            //
//              ID - 260464679                 //
//                 malloc.c                    //
//                                             //
/////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FF 0	// First fit
#define BF 1	// Best fit

#define BLOCKSIZE 16384				// struct size
#define TOP_FREE_BLOCK 131072		// top free block

// pointer helpers
#define ALIGN(x) ((x/BLOCKSIZE + 1) * BLOCKSIZE)
#define INCREASE_PTR(ptr, len) (((char*)ptr) + len)
#define DECREASE_PTR(ptr, len) (((char*)ptr) - len)

// Tag functions
#define NEW_TAG(len,free) ((len << 1) + (free == 1 ? 0b0 : 0b1))
#define GET_TAG_SIZE(len) (len >> 1)
#define GET_TAG_FREE(ptr) (ptr & 0b1)

//struct declaration for free block list
typedef struct FreeBlockList{
	struct FreeBlockList *nextBlock;
	struct FreeBlockList *previousBlock;
} FreeBlockList;

void updateContiguousBlock();                      	//updates the largest Contiguous Block when two smaller free blocks join
void updateTopFreeBlock();                   	   	//checks to see if the top free block is too large and then reduces as required by my_free()
void* createBlock(int size, int available);     	//used to create/fill a block in memory
void removeBlock(FreeBlockList *used);        		//used to remove a block from the free block list
void addBlock(FreeBlockList *new);            		//used to add a free block to the free block list

//define head and tail
FreeBlockList *head;
FreeBlockList *tail;

char *my_malloc_error;  //constant to hold errors
int* topBlock;         //holds the current top of the program break

//global variables declaration (required for my_mallinfo)
int currentPolicy = 0;
int bytesAllocated = 0;
int freeSpace = 0;
int largestContiguousBlock = 0;
int callCount = 0;

//This function returns void pointer that we can assign to any C pointer.
void *my_malloc(int size){
	if(callCount == 0){
		my_malloc_error = (char*)sbrk(1028); //set up the error reporting
	}
	
	callCount++;
	if(size < 0){
		my_malloc_error = "Error in memory allocation of requested space, my_malloc() call failed\n";
		return NULL;
	}
	
	if(currentPolicy == FF){
		FreeBlockList *temp = head;  //find the first availiable free block that can fit requested size
		while(temp != NULL){
			int* check = (int*)(temp);
			check = (int*)DECREASE_PTR(check, 4);
			unsigned available_size = GET_TAG_SIZE(check[0]);
			if(available_size - 8 >= size){
				return createBlock(size, available_size);
			}
			if(temp != NULL){
				temp = temp->nextBlock;
			}
		}
		return createBlock(size, -1); //if no block was found signal creation of a new block
	}
	
	else if(currentPolicy == BF){
		FreeBlockList *temp = head;
		unsigned max = 131073;

		while(temp != NULL){   //find the available fitting free block for requested memory
			int* check = (int*)(temp);
			check = (int*)DECREASE_PTR(check, 4);
			unsigned available_size = GET_TAG_SIZE(check[0]);
			if(available_size - 8 == size){
				return createBlock(size, available_size);
			}
			else if(available_size - 8 >= size && available_size <= max){
				max = available_size;
			}
			if(temp != NULL){
				temp = temp->nextBlock;
			}
		}
		return createBlock(size, max); 	//return the available fit or signal for new block to be created.
	}
	my_malloc_error = "Error: mallocing required space which was unavailable";  //return NULL if there is an issue
	return NULL;
}

//This function deallocates the block of memory pointed by the ptr argument. The ptr should be an
//address previously allocated by the Memory Allocation Package.
void my_free(void *ptr){
	int* newFreeBlock = (int*)(ptr);  
	//Set up the pointer and retrieve size information
	newFreeBlock = (int*)DECREASE_PTR(newFreeBlock, 4);

	int size = GET_TAG_SIZE(newFreeBlock[0]);
	bytesAllocated -= size;
	freeSpace += (size + 8);

	int* bot_check = (int*)DECREASE_PTR(newFreeBlock, 4);
	int* top_check = (int*)INCREASE_PTR(newFreeBlock, (size + 8));

	int bottomBlock = GET_TAG_FREE(bot_check[0]);
	int topBlock = GET_TAG_FREE(top_check[0]);
	int bottom_size_check = GET_TAG_SIZE(bot_check[0]);
	
	//if the bottom block is the start of my_malloc(), the block below will not be freed
	if(bottom_size_check == 0){
		bottomBlock = -1;
	}
	
	//if both the bottom and top blocks are free
	if(!bottomBlock && !topBlock){ 
		
		int bot_size = GET_TAG_SIZE(bot_check[0]);
		bot_check = (int*)DECREASE_PTR(bot_check, (bot_size - 8));

		int top_size = GET_TAG_SIZE(top_check[0]);
		top_check = (int*)INCREASE_PTR (top_check, 4);

		FreeBlockList *old_bot = (FreeBlockList*)(bot_check); 
		FreeBlockList *old_top = (FreeBlockList*)(top_check);
		
		//remove the two previous bot and top blocks
		removeBlock(old_bot);
		removeBlock(old_top);

		size += (bot_size + top_size + 8);             
		bot_check = (int*)DECREASE_PTR(bot_check, 4);
		top_check = (int*)INCREASE_PTR(top_check, (top_size - 8));
		*bot_check = NEW_TAG(size, 1);
		*top_check = NEW_TAG(size, 1);

		int* start = (int*)INCREASE_PTR(bot_check, 4);
		FreeBlockList *new = (FreeBlockList*)start;
		addBlock(new);                                       
	}
	
	//if only bottom block is free	
	else if(!bottomBlock){             
	
		int bot_size = GET_TAG_SIZE(bot_check[0]);
		bot_check = (int*)DECREASE_PTR(bot_check, (bot_size - 8));

		FreeBlockList *old_bot = (FreeBlockList*)(bot_check);   
		removeBlock(old_bot);							//remove the old bottom block 

		bot_check = (int*)DECREASE_PTR(bot_check, 4);
		newFreeBlock = (int*)INCREASE_PTR(newFreeBlock, (size + 4));

		size += (bot_size + 8);                          //update the size and tag information
		*bot_check = NEW_TAG(size, 1);
		*newFreeBlock = NEW_TAG(size, 1);

		int* start = (int*)INCREASE_PTR(bot_check, 4);

		FreeBlockList *new = (FreeBlockList*)start;
		addBlock(new);                                       //add the new block to the linked list
	}
	
	//if only top block is free
	else if(!topBlock){   
		
		int top_size = GET_TAG_SIZE(top_check[0]);
		top_check = (int*)INCREASE_PTR (top_check, (4));

		FreeBlockList *old_top = (FreeBlockList*)(top_check);   //remove the old top block
		removeBlock(old_top);

		top_check = (int*)INCREASE_PTR(top_check, (top_size - 8));  //update the size and tag information
		size += (top_size + 8);
		*newFreeBlock = NEW_TAG(size, 1);
		*top_check = NEW_TAG(size, 1);

		int* start = (int*)INCREASE_PTR(newFreeBlock, 4);
		FreeBlockList *new = (FreeBlockList*)start;       //add the new block to the linked list
		addBlock(new);
	}
	
	// if no adjacent free blocks, free this block and add a new block to the list
	else{ 
		*newFreeBlock = NEW_TAG((size + 8), 1);
		newFreeBlock = (int*)INCREASE_PTR(newFreeBlock, (size + 4));
		*newFreeBlock = NEW_TAG((size+8), 1);

		int* start = (int*)DECREASE_PTR(newFreeBlock, size);
		FreeBlockList *new = (FreeBlockList*)start;
		addBlock(new);
	}
	updateTopFreeBlock(); //update the top free block and contiguous space
}

void my_mallopt(int policy){//changes the memory allocation policy as required

	if(policy == FF){
		currentPolicy = FF;
	}
	else if(policy == BF){
		currentPolicy = BF;
	}
	else{
		printf("Error: Invalid Policy\n");
		return;
	}
}

//outputs information regarding all my_malloc variables
void my_mallinfo(){
	if(currentPolicy == BF){
		printf("Current policy: Best Fit\n");		
	}
	else{
		printf("Current policy: First Fit\n");
	}
	printf("Bytes allocated: %d\n", bytesAllocated);
	printf("Amount of free space: %d\n", freeSpace);
	printf("Largest contiguous free space: %d\n", largestContiguousBlock);
	printf("Total number of my_malloc() calls: %d\n", callCount);
}

//3 helper methods for the test
//returns the amount of free space available
int freespace(){
	
	return (int)freeSpace;
}

//returns bytes allocated
int bytes(){
	
	return (int)bytesAllocated;
}

//returns the largest contiguous block
int contiguousBlock(){

	return (int)largestContiguousBlock;
}

//check to see if there is a larger contiguous block
//and then updates accordingly
void updateContiguousBlock(){
	FreeBlockList *temp = head;
	int biggestBlock = 0;
	while(temp != NULL){
		int* update = (int*)temp;
		update = (int*)DECREASE_PTR(update, 4);
		int update_size = GET_TAG_SIZE(update[0]);
		if(update_size > biggestBlock){
			biggestBlock = update_size;
		}
		temp = temp->nextBlock;
	}
	largestContiguousBlock = biggestBlock;
}

//check to see if the top block is a free block and update accordingly
void updateTopFreeBlock(){

	int* check = (int*)(topBlock);
	check = (int*)DECREASE_PTR(check, 4);
	int topBlockTag = GET_TAG_FREE(check[0]);
	if(topBlockTag == 0){
		unsigned topBlockSize = GET_TAG_SIZE(check[0]);
		
		//if top block is a free block AND is larger than 128KB
		if(topBlockSize >=  TOP_FREE_BLOCK){
			//decrease the program count and update as required
			check = (int*)DECREASE_PTR(check, (topBlockSize - 4));
			sbrk(-20000);
			topBlockSize -= 20000;

			*check = NEW_TAG(topBlockSize, 1);
			check = (int*)INCREASE_PTR(check, (topBlockSize - 4));

			*check = NEW_TAG(topBlockSize, 1);
			topBlock = (int*)DECREASE_PTR(topBlock, 20000);

			freeSpace -= 20000;
		}
	}
	updateContiguousBlock(); //update the contiguous block in case it has changed
}

//creates a block if the program fails to allocate sufficient memory to an existing free block
// or its adjacent free blocks
void* createBlock(int size, int available){
	int block_size = ALIGN(size);
	
	// checks to see if within limits
	if(available == -1 || available == 131073){
		
		//if the requested size is sufficient to create a new free block
		if(block_size > (size + sizeof(FreeBlockList) +  8)){
			block_size = ALIGN(size);
		}
		//if not, create a second block regardless
		else{
			block_size += BLOCKSIZE;
		}

		int* start = (int*)sbrk(block_size + 8);   //increase the program break
		int* freeBlockEnd = (int*)sbrk(0);
		bytesAllocated += size;
		topBlock = freeBlockEnd;
		*start = NEW_TAG(size, 0);
		start = (int*)INCREASE_PTR(start, 4);
		
		int* end = (int*)INCREASE_PTR(start, size);
		*end = NEW_TAG(size, 0);

		int* freeBlockStart = (int*)INCREASE_PTR(end, 4);
		*freeBlockStart = NEW_TAG((block_size - size), 1);
		freeBlockStart = (int*)INCREASE_PTR(freeBlockStart, 4);

		FreeBlockList *new = (FreeBlockList*)freeBlockStart;  
		addBlock(new);				//add a new free block

		freeSpace += (block_size - size);
		freeBlockEnd = (int*)INCREASE_PTR(freeBlockStart, (block_size-(size + 8)));
		*freeBlockEnd = NEW_TAG((block_size - size), 1);

		updateContiguousBlock();		 //updates contiguous block if necessary
		return (void*)start; 			 //return the address
	}
	//if a block has been specified search for it in free list
	else{ 
		FreeBlockList *temp = head;
		while(temp != NULL){
			int* find = (int*)(temp);
			find = (int*)DECREASE_PTR(find, 4);
			unsigned available_size = GET_TAG_SIZE(find[0]);
			if(available_size == available)
				break;
			else
			temp = temp->nextBlock;
		}
		
		int* check = (int*)(temp);    //once found recheck the size of said block
		check = (int*)DECREASE_PTR(check, 4);
		unsigned available_size = GET_TAG_SIZE(check[0]);

		//either fill the entire free space or split it up
		if(available_size - 8 >= size && available_size > (size + sizeof(FreeBlockList) + 8)){
			int new_size = (available_size - (size + 8));
			int* start = (int*)check;
			int* free_end = (int*)INCREASE_PTR(check, (available_size));

			*start = NEW_TAG(size, 0);
			start = (int*)INCREASE_PTR(start, 4);
			int* end = (int*)INCREASE_PTR(start, size);
			*end = NEW_TAG(size, 0);

			int* free_start = (int*)INCREASE_PTR(end, 4);
			*free_start = NEW_TAG(new_size, 1);
			free_start = (int*)INCREASE_PTR(free_start, 4);

			removeBlock(temp);                           //remove old block and add new one

			FreeBlockList *new = (FreeBlockList*)free_start;
			addBlock(new);

			free_end = (int*)DECREASE_PTR(free_end, 4);
			*free_end = NEW_TAG(new_size, 1);
			bytesAllocated += size;
			freeSpace -= (size+8);
			updateContiguousBlock();
			return (void*)start;      //return the address to be written too
		} 
		
		else if(available_size - 8 >= size){ //or return entire free block space and do not split it
			int* start = (int*)check;

			*start = NEW_TAG((available_size-8), 0);
			start = (int*)INCREASE_PTR(start, 4);
			int* end = (int*)INCREASE_PTR(start, (available_size-8));
			*end = NEW_TAG((available_size-8), 0);

			removeBlock(temp);         //remove the free block and return the appropriate address

			bytesAllocated += (available_size-8);
			freeSpace -= available_size;

			printf(" %d extra bytes were allocated to complete malloc() request\n", available_size - (size + 8));
			updateContiguousBlock();
			return (void*)start;
		}
		else{
			return NULL;
		}
	}
}

void removeBlock(FreeBlockList *used){//removes a block from the block list
	// changes next and previous block pointers
	if(used->previousBlock != NULL){
		used->previousBlock->nextBlock = used->nextBlock;
	}
	if(used->nextBlock != NULL){
		used->nextBlock->previousBlock = used->previousBlock;
	}
	// moves head and tail blocks if block being removed is either one of them
	if(used == head){
		head = used->nextBlock;
	}
	if(used == tail){
		tail = used->previousBlock;
	}
	// sets everything to null
	used->nextBlock = NULL;
	used->previousBlock = NULL;
	used = NULL;
}

void addBlock(FreeBlockList *new){//adds a block to the head of the block list
	
	if(head != NULL){
		head->previousBlock = new;
		new->nextBlock = head;
		head = new;
		new->previousBlock = NULL;
	}
	
	else{
		head = new;
		tail = new;
	}
}
