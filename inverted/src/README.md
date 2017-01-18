P3: Better Inverted Index Using MapReduce

Group Members

Hilary Johnson
Utsav Roy
Class

CS430 - Introduction to Parallel Computing
BUILDING AND RUNNING:

to build the program: $ make

to run the program copy the jar file produce after building the program to the hadoop setup and follow the following command: $ hadoop jar inverted-index.jar InvertedIndex input output

to clean the executable: $make clean

Goal

Given a starting file "InvertedIndex.java", which uses mapreduce to report each word along with each file that contained that word.

Step 1: Instead of reporting each file that it occurs in, list the count that it occurs in each file. Step 2: Improve the output to print the files that contain the word the most are listed first.
Step 3: Improve the timing of the program.