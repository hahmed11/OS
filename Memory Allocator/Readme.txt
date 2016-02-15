Contents:
1. malloc.c: implementation of malloc
2. test.c: test file to test the malloc implementation
3. Makefile: makefile to compile and run the test
4. Output - 1.jpeg, Output - 2.jpeg: Screenshots of test results
5. Output.txt: test results output in a text file 


Testing steps:

1. Enter "make clean" in the terminal
2. Enter "make" in the terminal
3. Enter "./malloc" in the terminal

By default the policy is set to best fit. In order to change to first fit, you can change "my_mallopt(1);" in line 37 of the "test.c" file to:    "my_mallopt(0);"

NOTE:
->Test cases 3, 5 and 7 only involve deallocating memory or freeing up blocks. As a result, my_malloc() is not called in these cases and the output represents this.
->The number of my_malloc() call jumps from 0 to 2 because my_malloc() was called once in line 40 of "test.c". This was an invalid case that I used to check if it passes through and is therefore, intentional. So, my_malloc() counter increases even though it doesn't pass through.
->Explanations for the test cases can be found within the "test.c" file ( in comments)