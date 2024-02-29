/*
 * Name: Ben Temming 
 * Student ID: 52094132
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


void CGS_D3_D1_test(){
  printf("CGS D3-D1\n"); 
  
  printf("Format Virtual Disk\n"); 
  format(); 

  //Printing the first 4 block to check if formatting worked
  for (int i=0; i < 4; i++){
    printf("block %d\n", i);
    printBlock(i); 
  }
  printf("\n");
  
  //writing virtual disk to file
  writedisk("virtualdiskD3_D1");
  printf("\n");
}

void CGS_C3_C1_test(){
  printf("CGS C3-C1\n"); 
  // //crate a file 
  MyFILE *file = myfopen("testfile.txt", "w");

  if (file != NULL){
    //create array of characters to load into file 
    char char_arr[4*BLOCKSIZE]; 
    //fill it with characters 
    char alphabet_str[] = "ABCDEFGHIJKLMNOPQRSTUVWXWZ"; //0 to 25 index 
    for (int i=0; i < 4*BLOCKSIZE; i++){
      char_arr[i] = alphabet_str[i%26]; 
    }
    //write char_arr to file 
    for (int i=0; i<4*BLOCKSIZE; i++){
      myfputc(char_arr[i], file); 
    }
    //close the file 
    myfclose(file); 
  }

  //write complete virtual disk to file 
  writedisk("virtualdiskC3_C1"); 


  //open file in read mode 
  MyFILE *file2 = myfopen("testfile.txt", "r");
  //open output file 
  FILE *outputFile = fopen("testfileC3_C1_copy.txt", "w"); 

  if (file2 != NULL && outputFile != NULL){
    //read file contents and print them to screen 
    printf("\nFile contents: \n");

    int byte, counter = 0;
    while ((byte = myfgetc(file2)) != EOF) {
      //write byte to screen
      printf("%c", byte);
      counter++;
      //write byte to output file  
      fputc(byte, outputFile);
    }

    printf("\n\nNumber of bytes read: %d\n\n", counter); 
    
    //close the file 
    myfclose(file2); 
    //close the output file 
    fclose(outputFile);
  }

  printf("\n");
}


void CGS_B3_B1_test(){
  printf("CGS B3-B1\n"); 
  //create new directory 
  mymkdir("/myfirstdir/myseconddir/mythirddir"); 
  //print contents in directory 
  char **directoryEntries = mylistdir("/myfirstdir/myseconddir");
  if (directoryEntries != NULL){
    printf("Directory entries of /myfirstdir/myseconddir:\n");
    //print each file name in terminal 
    for (int i = 0; directoryEntries[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, directoryEntries[i]);
      //free each entry
      free(directoryEntries[i]);
    }
    //free list of entries 
    free(directoryEntries);
    printf("\n");
  }
  //writing virtual disk to file
  writedisk("virtualdiskB3_B1_a");


  //crate a file “/myfirstdir/myseconddir/testfile.txt” in the virtual disk 
  MyFILE *file3 = myfopen("/myfirstdir/myseconddir/testfile.txt", "w");
  if (file3 != NULL){
    //add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('1', file3); 
    }
    //close the file 
    myfclose(file3);
  } 

  //print contents in directory 
  char **directoryEntries2 = mylistdir("/myfirstdir/myseconddir");
  if (directoryEntries2 != NULL){
    printf("\nDirectory entries of /myfirstdir/myseconddir:\n");
    //print each file name in terminal 
    for (int i = 0; directoryEntries2[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, directoryEntries2[i]);
      //free each entry
      free(directoryEntries2[i]);
    }
    //free list of entries 
    free(directoryEntries2);
    printf("\n");
  }
  //writing virtual disk to file
  writedisk("virtualdiskB3_B1_b");
  printf("\n"); 
}


void CGS_A5_A4_test(){
  //CGS A5-A4
  printf("CGS A5-A4\n"); 
  //create new directory 
  mymkdir("/firstdir/seconddir"); 
  MyFILE *file = myfopen("/firstdir/seconddir/testfile1.txt", "w");
  if (file != NULL){
    // add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('1', file); 
    }
    //close the file 
    myfclose(file);
  }

  // print contents in directory 
  char **dir_list = mylistdir("/firstdir/seconddir");
  if (dir_list != NULL){
    printf("\nDirectory entries of /firstdir/seconddir:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //change to directory  
  mychdir("/firstdir/seconddir"); 

  // print contents in directory 
  dir_list = mylistdir(".");
  if (dir_list != NULL){
    printf("\nDirectory entries of .:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //create file with relative path 
  file = myfopen("testfile2.txt", "w");
  if (file != NULL){
    // add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('2', file); 
    }
    //close the file 
    myfclose(file);
  }

  //create new directory with relative path
  mymkdir("thirddir");

  //create file 
  file = myfopen("thirddir/testfile3.txt", "w");
  if (file != NULL){
    // add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('3', file); 
    }
    //close the file 
    myfclose(file);
  }

  //writing virtual disk to file
  writedisk("virtualdiskA5_A1_a"); 


  //print directory 
  dir_list = mylistdir("/firstdir/seconddir");
  if (dir_list != NULL){
    printf("\nDirectory entries of /firstdir/seconddir:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //remove testfile1.txt
  myremove("testfile1.txt"); 

  //remove testfile2.txt
  myremove("testfile2.txt");

  //writing virtual disk to file
  writedisk("virtualdiskA5_A1_b"); 
  

  //print directory after removing files 
  dir_list = mylistdir("/firstdir/seconddir");
  if (dir_list != NULL){
    printf("\nDirectory entries of /firstdir/seconddir after myremove:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }
 
  //change directory 
  mychdir("thirddir"); 

  //remove testfile3.txt
  myremove("testfile3.txt");

  //writing virtual disk to file
  writedisk("virtualdiskA5_A1_c"); 

  //change directory using /firstdir/seconddir or ..
  mychdir(".."); 

  //remove thirddir
  myremdir("thirddir"); 

  //change directory /firstdir
  mychdir("/firstdir");

  //remove seconddir
  myremdir("seconddir"); 

  //change directory / or ..
  mychdir(".."); 

  //remove thirddir
  myremdir("firstdir");

  //writing virtual disk to file
  writedisk("virtualdiskA5_A1_d");

  printf("\n");
}


void CGS_A3_test(){
  //CGS A3
  printf("CGS A3\n"); 
  //copy file from real to virtual disk test_input_file_A3
  copy_virtual_real("/A3_dir/test_input_file_A3.txt", "test_input_file_A3.txt", TRUE);
  //writing virtual disk to file
  writedisk("virtualdiskA3");
  //copy file from virtual to real disk 
  copy_virtual_real("/A3_dir/test_input_file_A3.txt", "test_output_file_A3.txt", FALSE);
}


void CGS_A2_test(){
  printf("\nCGS A2\n"); 

  //crate a new directory 
  mymkdir("A2_dir");

  //create file 
  MyFILE *file = myfopen("testfile_A2.txt", "w");
  if (file != NULL){
    // add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('A', file); 
    }
    //close the file 
    myfclose(file);
  }
  
  //print the current directory content
  char **dir_list = mylistdir("/.");
  if (dir_list != NULL){
    printf("\nDirectory entries of /.:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  } 

  //copy file (source path, destination path) 
  copy_file("testfile_A2.txt", "testfile_A2_copy.txt"); 

  //print the current direcotry content 
  dir_list = mylistdir("/."); 
  if (dir_list != NULL){
    printf("\nDirectory entries of /.:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //writing virtual disk to file
  writedisk("virtualdiskA2_a");

  //print contents in A2_dir before moving file 
  dir_list = mylistdir("/A2_dir");
  if (dir_list != NULL){
    printf("\nDirectory entries of /A2_dir (before moving file):\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //moving file (source path, destination dir)
  move_file("testfile_A2_copy.txt", "A2_dir");

  // print contents in A2_dir after moving file 
  dir_list = mylistdir("/A2_dir");
  if (dir_list != NULL){
    printf("\nDirectory entries of /A2_dir (after moving file):\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //make new file in root dir to overwrite old file name on disk
  //create file 
  MyFILE *file2 = myfopen("testfile_A2_overwrite.txt", "w");
  if (file2 != NULL){
    // add something to file for testing 
    for (int i=0; i<BLOCKSIZE; i++){
      myfputc('T', file2); 
    }
    //close the file 
    myfclose(file2);
  }

  // print contents in directory 
  dir_list = mylistdir("/.");
  if (dir_list != NULL){
    printf("\nDirectory entries of /.:\n");
    //print each file name in terminal 
    for (int i = 0; dir_list[i] != NULL; i++) {
      //print directory entry 
      printf("Entry %d: %s\n", i, dir_list[i]);
      //free each entry
      free(dir_list[i]);
    }
    //free list of entries 
    free(dir_list);
    printf("\n");
  }

  //writing virtual disk to file
  writedisk("virtualdiskA2_b");
}



int main(){
  printf("shell> start\n");

  // CGS D3-D1 testing 
  CGS_D3_D1_test();

  //CGS C3-C1 testing
  CGS_C3_C1_test();

  // //B3-B1 testing 
  CGS_B3_B1_test(); 

  CGS_A5_A4_test(); 
  /*
  Note: The myremdir() and myremove() functions do no change the contents 
  of the virtual disk when a file or directory is removed, but rather it marks the block 
  unused in the FAT table. The data will stay on the disk until another directory or 
  file is created and overwrites the block on the viritual disk. 
  */

  //CGS A3 testing 
  CGS_A3_test();

  // //CGS A2 testing
  CGS_A2_test();  
  
  printf ( "\nshell> end\n");
  return 0;
}