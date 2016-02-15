/////////////////////////////////////////////////
//          ECSE 427 - Assignment 4            //
//          Author - HABIB I. AHMED            //
//              ID - 260464679                 //
//                 test.c                      //
//                                             //
/////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCKSIZE 16384		//block size is 16KB by default
#define VAL_A 2048			//using a block of size 2KB
#define VAL_B 8192			//using a block of size 8KB
#define VAL_C 15000			//using a block of size 15000 bytes

extern char *my_malloc_error;

void *my_malloc(int size);
void my_free(void *ptr);
void my_mallopt(int policy);
void my_mallinfo();
int freespace();
int bytes();
int contiguousBlock();

int main(int argc, char* argv[])
{
	printf("No policy allocated\n\n\n");
	my_mallinfo();
	printf("/-------------------------------------------------------------------------------/\n");
	
	////// CHANGE THE VALUE OF my_mallopt() TO OBTAIN VALUES FOR FIRST FIT
	// my_mallopt(1) sets the policy to Best Fit and my_mallopt(0) changes the policy to First fit
	my_mallopt(1);
	
	// test for an invalid case to see if it passes through
	int* failCase = my_malloc(-10);
	printf("%s\r\n", my_malloc_error);
	// running test one for block with size 2048 (VAL_A)
	int* test_one = my_malloc(VAL_A);
	int i, j=0;
	for(i = 0; i < VAL_A/sizeof(int); i++){
		test_one[i] = i + j;
		j++;
	}
	printf("/-------------------------------------------------------------------------------/\n");	
	printf("Running test case 1...\n");	
	my_mallinfo();
	printf("Current address: %p\r\n", test_one);
	
	// checks for bytes allocation
	if(bytes() == VAL_A){
		printf("Bytes allocation successful.\r\n");		
	}
	else{
		printf("Bytes allocation failed.\r\n");		
	}

	// checks for freeing of specified amount of bytes
	if(freespace() == (BLOCKSIZE - VAL_A)){
		printf("Bytes freed successfully.\r\n");		
	}
	else{
		printf("Bytes were not freed successfully.\r\n");		
	}

	// checks for largest contiguous block available
	if(contiguousBlock() == (BLOCKSIZE - VAL_A)){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");		
	}

	printf("/-------------------------------------------------------------------------------/\n");
	// running test two for block with size 8192 (VAL_B) + 2048 (VAL_A)
	// since my_malloc is run again, VAL_B must be taken into consideration as well as VAL_A
	int* test_two = my_malloc(VAL_B);
	printf("Running test case 2...\n");	
	printf("Current address: %p\r\n", test_two);

	if(bytes() == VAL_A + VAL_B){
		printf("Bytes allocation successful.\r\n");		
	}
	else{
		printf("Bytes allocation failed.\r\n");		
	}

	if(freespace() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Bytes freed successfully.\r\n");		
	}
	else{
		printf("Bytes were not freed successfully.\r\n");		
	}

	if(contiguousBlock() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");		
	}

	my_mallinfo();

	printf("/-------------------------------------------------------------------------------/\n");
	j = 0;
	for(i = 0; i < VAL_B/sizeof(int); i++){
		test_two[i] = i + j;
		j++;
	}
	my_free(test_one);			// frees up memory after running the first test

	// since memory has been freed up, only VAL_B values will be shown and taken into account
	// this is the third test case
	printf("Running test case 3...\n");	
	if(bytes() == VAL_B){
		printf("Bytes allocation successful.\r\n");		
	}
	else{
		printf("Bytes allocation failed.\r\n");	
	}

	if(freespace() == (BLOCKSIZE - VAL_B)){
		printf("Bytes freed successfully.\r\n");		
	}
	else{
		printf("Bytes were not freed successfully.\r\n");		
	}

	if(contiguousBlock() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");		
	}



	my_mallinfo();
	printf("/-------------------------------------------------------------------------------/\n");
	// my_malloc is run again using VAL_A on top of VAL_B this time
	// returns the same value as in the 2nd test case
	// this is the fourth test case
	printf("Running test case 4...\n");	
	int* test_three = my_malloc(VAL_A);
	j = 0;
	for(i = 0; i < VAL_A/sizeof(int); i++){
		test_three[i] = i + j;
		j++;
	}
	printf("Current address: %p\r\n", test_three);
	
	if(bytes() == VAL_A + VAL_B){
		printf("Bytes allocation successful.\r\n");
	}
	else{
		printf("Bytes allocation failed.\r\n");			
	}

	if(freespace() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Bytes freed successfully.\r\n");			
	}
	else{
		printf("Bytes were not freed successfully.\r\n");			
	}

	if(contiguousBlock() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Largest contiguous block is valid.\r\n");			
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");		
	}
	my_mallinfo();
	printf("/-------------------------------------------------------------------------------/\n");
	my_free(test_three);		//frees up memory after running third test. allows us to recheck for results of VAL_B
	
	// this is the fifth test case
	// The purpose of this test case is to show that address has changed
	//  i.e, even though the values being used are the same, blocks are still being deallocated
	printf("Running test case 5...\n");	
	if(bytes() == VAL_B){
		printf("Bytes allocation successful.\r\n");		
	}
	else{
		printf("Bytes allocation failed.\r\n");				
	}

	if(freespace() == (BLOCKSIZE - VAL_B)){
		printf("Bytes freed successfully.\r\n");			
	}
	else{
		printf("Bytes were not freed successfully.\r\n");				
	}

	if(contiguousBlock() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Largest contiguous block is valid.\r\n");			
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");			
	}
	
	my_mallinfo();
	printf("/-------------------------------------------------------------------------------/\n");
	// using VAL_C + VAL_B to test the memory allocater
	// this is the sixth test case
	printf("Running test case 6...\n");	
	int* test_four = my_malloc(VAL_C);
	j = 0;
	for(i = 0; i < VAL_C/sizeof(int); i++){
		test_four[i] = i + j;
		j++;
	}
	printf("Current address: %p\r\n", test_four);

	my_mallinfo();
	
	if(bytes() == VAL_B+VAL_C){
		printf("Bytes allocation successful.\r\n");			
	}
	else{
		printf("Bytes allocation failed.\r\n");
	}

	if(freespace() == (2*(BLOCKSIZE) - (VAL_B+VAL_C))){
		printf("Bytes freed successfully.\r\n");
	}
	else{
		printf("Bytes were not freed successfully.\r\n");
	}

	if(contiguousBlock() == (BLOCKSIZE - (VAL_A+VAL_B+8))){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");			
	}
	printf("/-------------------------------------------------------------------------------/\n");
	my_free(test_two);			//frees up results of test two

	// freeing up test_two allows to check for results of VAL_C only
	// this is the 7th test case
	printf("Running test case 7...\n");	
	if(bytes() == VAL_C){
		printf("Bytes allocation successful.\r\n");					
	}
	else{
		printf("Bytes allocation failed.\r\n");		
	}

	if(freespace() == (2*(BLOCKSIZE)+8 - VAL_C)){
		printf("Bytes freed successfully.\r\n");		
	}
	else{
		printf("Bytes were not freed successfully.\r\n");		
	}
	
	if(contiguousBlock() == (BLOCKSIZE+8)){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");			
	}
	my_mallinfo();
	printf("/-------------------------------------------------------------------------------/\n");
	my_free(test_four);			//frees up test four data
	
	//all four test data have been freed
	//checks to see if the data is consistent
	// can be considered as the 8th test case
	if(bytes() == 0){
		printf("Bytes allocation successful.\r\n");				
	}
	else{
		printf("Bytes allocation failed.\r\n");			
	}

	if(freespace() == (2*(BLOCKSIZE) + 16)){
		printf("Bytes freed successfully.\r\n");		
	}
	else{
		printf("Bytes were not freed successfully.\r\n");			
	}

	if(contiguousBlock() == (2*(BLOCKSIZE) + 16)){
		printf("Largest contiguous block is valid.\r\n");		
	}
	else{
		printf("Largest contiguous block is invalid.\r\n");		
	}

	my_mallinfo();		
	printf("/-------------------------------------------------------------------------------/\n");
	printf("All test cases have been run\n");	
	return 0;
}