/*
 * Name: Ben Temming 
 * Student ID: 52094132
 */


/* filesys.c
 * 
 * provides interface to virtual disk
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"



diskblock_t  virtualDisk [MAXBLOCKS]= {0} ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;

/* writedisk : writes virtual disk out to physical disk
 * 
 * in: file name of stored virtual disk
 */

void writedisk ( const char * filename )
{
   printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;
   
}

void readdisk ( const char * filename )
{
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}


/* the basic interface to the virtual disk
 * this moves memory around
 */

void writeblock ( diskblock_t * block, int block_address )
{
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}

//function to read a block from virtual disk
void readblock(diskblock_t *block, int block_address){
  //copy the data from the virtual disk to the block 
  memmove(block->data, virtualDisk[block_address].data, BLOCKSIZE); 
}


void print_current_dir_content(){
  //Function that prints the content (including deleted content) in the current 
  //directory. Used for testing
  
  //define start directory  
  fatentry_t startDirIndex = currentDirIndex;
  diskblock_t start_dir_block;
  //read the start directory block from the virtual disk 
  readblock(&start_dir_block, startDirIndex); 
  direntry_t *startDir = start_dir_block.dir.entrylist; 
  
  //go through current dir and print contents 
  printf("Current dir entries: \n");
  //go through dir block by block 
  int curr_block_index = startDirIndex; 
  while (curr_block_index != ENDOFCHAIN){
    //for every entry in the entrylist
    for (int i = 0; i < start_dir_block.dir.nextEntry; i++) { 
      if (startDir[i].unused == FALSE){
        //if direcotry entry has not been deleted 
        printf("Current dir entry: %s  filelength: %d\n", startDir[i].name, startDir[i].filelength);
      }else{
        //if direcotry entry has been deleted but now overwritten
        printf("Deleted: %s  filelength: %d\n", startDir[i].name, startDir[i].filelength);
      }
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&start_dir_block, curr_block_index);
      startDir = start_dir_block.dir.entrylist; 
    }
  }
  printf("\n");
}


void move_file(const char* source_path, const char* dest_dir_path){
  //moves a file located at the source_path into a differnt directory
  //specified by the dest_dir_path

  printf("Moving file %s to directory: %s\n", source_path, dest_dir_path); 

  //go to directory of the file file
  MyFILE *file = myfopen(source_path, "r"); 
  //get the directory block of the file 
  fatentry_t file_dir_index = file->dirblockno; 
  //close the file 
  myfclose(file); 

  //get the files name from the path 
  char *file_next_path_string; 
  char *file_rest = strdup(source_path); //create a mutalbe copy of the path 
  char *file_current_path_string; 
  //Save the original pointer so that the memory can be freed 
  char *file_original_rest = file_rest;
  //get the first string in the path
  file_next_path_string = strtok_r(file_rest, "/", &file_rest); 
  //go through path until last element, this is the files name
  while (file_next_path_string != NULL){
    //save the current string
    file_current_path_string = file_next_path_string;
    //move to next string in path 
    file_next_path_string = strtok_r(file_rest, "/", &file_rest); 
  }
  // printf("File name: %s\n", file_current_path_string); 

  //define file directory  
  diskblock_t file_dir_block;
  //read the file directory block from the virtual disk 
  readblock(&file_dir_block, file_dir_index);  
  direntry_t *file_Dir = file_dir_block.dir.entrylist; 

  //check if the file exists in the entries in the current directory 
  //need to go through all blocks that belong to the directory
  int curr_block_index = file_dir_index; 
  int found_file = FALSE; 
  int file_entrylist_index; 
  while (curr_block_index != ENDOFCHAIN){
    //for every element in the directory entry list
    for (int i=0; i < file_dir_block.dir.nextEntry; i++){
      if (strcmp(file_Dir[i].name, file_current_path_string) == 0){
        // printf("Files: %s\n", file_Dir[i].name); 
        found_file = TRUE; 
        file_entrylist_index = i; 
      }
    }
    if (found_file == FALSE){
      //move to next block 
      curr_block_index = FAT[curr_block_index]; 
      if (curr_block_index != ENDOFCHAIN){
        //load the next block 
        readblock(&file_dir_block, curr_block_index); 
        file_Dir = file_dir_block.dir.entrylist; 
        //move file_dir_index to the next dir block 
        file_dir_index = curr_block_index;
      } 
    }else{
      break; 
    }
  }

  //test 
  // printf("File is: %s\n", file_Dir[file_entrylist_index].name); 

  //go to destination directory 
  fatentry_t dest_DirIndex = currentDirIndex;
  diskblock_t dest_dir_block;
  //read the dest directory block from the virtual disk 
  readblock(&dest_dir_block, dest_DirIndex);  
  direntry_t *dest_dir = dest_dir_block.dir.entrylist;

  //check if absolute path 
  if (dest_dir_path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk 
    readblock(&dest_dir_block, rootDirIndex); 
    //set the startDirIndex to rootDirIndex7
    dest_DirIndex = rootDirIndex; 
    //set the startDir to root dir entrylist 
    dest_dir = dest_dir_block.dir.entrylist; 
  }

  //variables for spliting the string 
  char *dir_name; 
  char *dest_rest = strdup(dest_dir_path); // Create a mutable copy of the path 
  //Save the original pointer so that the memory can be freed 
  char *dest_original_rest = dest_rest;
  //go through every directory in the path
  while ((dir_name = strtok_r(dest_rest, "/", &dest_rest))){
    //go throuhg the all blocks of the directory
    int found_dir = FALSE; 
    int curr_block_index = dest_DirIndex;
    while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
      //for entry in directory entry list 
      for (int i=0; i < dest_dir_block.dir.nextEntry; i++){
        //check if name matches, entry is a directory and entry is not deleted
        if (strcmp(dest_dir[i].name, dir_name) == 0 && dest_dir[i].isdir == TRUE && dest_dir[i].unused == FALSE){
          //found dir, update variables 
          dest_DirIndex = dest_dir[i].firstblock; 
          //read the directory block from virtual disk 
          readblock(&dest_dir_block, dest_DirIndex); 
          dest_dir = dest_dir_block.dir.entrylist; 
          found_dir = TRUE; 
          break; 
        }
      }
      if (found_dir == FALSE){
        //move to next block 
        curr_block_index = FAT[curr_block_index]; 
        if (curr_block_index != ENDOFCHAIN){
          //load the next block 
          readblock(&dest_dir_block, curr_block_index); 
          dest_dir = dest_dir_block.dir.entrylist; 
        }
      }
    }
    //if directory does not exist 
    if (found_dir == FALSE){
      printf("Could note move file as %s direcotry does not exist\n", dest_dir_path); 
      //free the memory allocated by strdup() 
      free(dest_original_rest); 
      free(file_original_rest);
      return; 
    }
  }

  //get the next block that has a free directory element
  dest_DirIndex = getFreeDirEntryBlock(dest_DirIndex); 
  //read the directory block that has a free directory element 
  readblock(&dest_dir_block, dest_DirIndex); 
  dest_dir = dest_dir_block.dir.entrylist; 

  //get the next free directory entry, 
  int free_entry_index = dest_dir_block.dir.nextEntry; 
  for (int i = 0; i < dest_dir_block.dir.nextEntry; i++){
    //if there is an unused element in the directory
    if (dest_dir_block.dir.entrylist[i].unused == TRUE){
      free_entry_index = i; 
      break; 
    }
  }
  //if there was no available element, add to the end 
  if (free_entry_index == dest_dir_block.dir.nextEntry){
    //move the next entry counter
    dest_dir_block.dir.nextEntry++;
  }
  //reset the memory of the name char array 
  memset(dest_dir[free_entry_index].name, '\0', MAXNAME); 
  //copy the file name to directory entry 
  strcpy(dest_dir[free_entry_index].name, file_Dir[file_entrylist_index].name); 
  //set unused to FALSE 
  dest_dir[free_entry_index].unused = FALSE; 
  //set isdir to FALSE 
  dest_dir[free_entry_index].isdir = FALSE; 
  //set isdir to FALSE 
  dest_dir[free_entry_index].isdir = FALSE; 
  //set the file length 
  dest_dir[free_entry_index].filelength = file_Dir[file_entrylist_index].filelength; 
  //set the first block entry in the directory 
  dest_dir[free_entry_index].firstblock = file_Dir[file_entrylist_index].firstblock; 
  //write the updated directory block to virtual disk 
  writeblock(&dest_dir_block, dest_DirIndex);

  //delete the file from the old directory
  //remove file form old directory by setting unused to true
  file_Dir[file_entrylist_index].unused = TRUE; 
  // //update old file directory on disk 
  writeblock(&file_dir_block, file_dir_index);

  //free the memory allocated by strdup() 
  free(dest_original_rest); 
  free(file_original_rest);
}



void copy_file(const char* source_path, const char* dest_path){
  //copy the contents of the file located at the source path 
  //into a new file located at the dest_path

  //open source file in "r" mode 
  MyFILE *source_file = myfopen(source_path, "r"); 
  //check if file has opened successfuly
  if (source_file == NULL){
    printf("File %s could not be opend\n", source_path); 
    return; 
  }
  //open destination file in "w" mode 
  MyFILE *dest_file = myfopen(dest_path, "w"); 
  //check if file has opened successfuly
  if (dest_file == NULL){
    printf("File %s could not be opend\n", dest_path); 
    return; 
  }

  //read the real file character by character until end of file (EOF) is reached
  int byte; 
  while ((byte = myfgetc(source_file)) != EOF){
    //save the character in destination file  
    myfputc(byte, dest_file); 
  }

  //close the source file 
  myfclose(source_file); 
  //close the destination file 
  myfclose(dest_file); 
}


void copy_virtual_real(const char* virtual_path, const char* real_filename, int to_virtual){
  //copy a file from real disk to virtual disk or from virtual disk to real disk
  //check which way the file should be copied 
  if (to_virtual == TRUE){
    printf("Copy: %s from real to virtual: %s\n", real_filename, virtual_path);

    //open the real file in "r" mode 
    FILE *real_file_pointer = fopen(real_filename, "r"); 
    //check if file has opend successfuly 
    if (real_file_pointer == NULL){
      printf("File %s could not be opend on real disk\n", real_filename); 
      return; 
    }
    //open virtual file in "w" mode 
    MyFILE *virtual_file_pointer = myfopen(virtual_path, "w"); 
    //check if file has opened successfuly
    if (virtual_file_pointer == NULL){
      printf("File %s could not be opend on virtual disk\n", virtual_path); 
      return; 
    }

    //read the real file character by character until end of file (EOF) is reached
    int byte;
    while ((byte = fgetc(real_file_pointer)) != EOF){
      //save each character to the virtual file 
      myfputc(byte, virtual_file_pointer); 
    }

    //close the real file 
    fclose(real_file_pointer); 
    //close the virtual file 
    myfclose(virtual_file_pointer); 
  }else{
    printf("Copy: %s from virtual to real: %s\n", virtual_path, real_filename);

    //open the real file in "w" mode 
    FILE *real_file_pointer = fopen(real_filename, "w"); 
    //check if file has opend successfuly 
    if (real_file_pointer == NULL){
      printf("File %s could not be opend on real disk\n", real_filename); 
      return; 
    }
    //open virtual file in "r" mode 
    MyFILE *virtual_file_pointer = myfopen(virtual_path, "r"); 
    //check if file has opened successfuly
    if (virtual_file_pointer == NULL){
      printf("File %s could not be opend on virtual disk\n", virtual_path); 
      return; 
    }

    //read the real file character by character until end of file (EOF) is reached
    int byte; 
    while ((byte = myfgetc(virtual_file_pointer)) != EOF){
      //save the character in the real file 
      fputc(byte, real_file_pointer); 
    }

    //close the real file 
    fclose(real_file_pointer); 
    //close the virtual file 
    myfclose(virtual_file_pointer); 
  }
}



void myremdir(const char* path){
  //remove an exisitng directory if the direcoty is empty (only . and .. in directory)
  //when the directory is deleted the entry in the parent directory is set to unused
  //and all FAT table blocks used by the directory are set to unused, but the 
  //content is not overwritten on the disk

  //load the current directry from virtual disk 
  diskblock_t dir_block;
  fatentry_t dirIndex = currentDirIndex;  
  readblock(&dir_block, dirIndex); 
  direntry_t *dir_list = dir_block.dir.entrylist;
   
  //check if absolute path 
  if (path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk into the dir block
    readblock(&dir_block, rootDirIndex); 
    //set the dirIndex to rootDirIndex7
    dirIndex = rootDirIndex; 
    //set the dir to root dir entrylist 
    dir_list = dir_block.dir.entrylist; 
  }

  //Go through path and move dir into the required directory
  char *next_path_string; 
  char *rest = strdup(path); //create a mutalbe copy of the path
  char *current_path_string; 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;
  //get the first string in the path
  next_path_string = strtok_r(rest, "/", &rest); 

  //go through directories in path until final directory is reached
  int found_dir = FALSE; 
  while (next_path_string != NULL){
    //save the current string
    current_path_string = next_path_string;
    //move to next string in path 
    next_path_string = strtok_r(rest, "/", &rest); 
    //if the next path string is not NULL, this means the current path string is a directory
    if (next_path_string != NULL){
      //change start dir into next directory, if does not exist return 
      //find the directory_t in entrylist which has name == current_path_string
      //need to go through all blocks that belong to the directory
      int curr_block_index = dirIndex;
      while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
        //for every entry in the dir entry list
        for (int i=0; i < dir_block.dir.nextEntry; i++){
          //check if name matches 
          if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].unused == FALSE){
            //found dir, update variables 
            dirIndex = dir_list[i].firstblock; 
            //read the directory block from virtual disk 
            readblock(&dir_block, dirIndex); 
            dir_list = dir_block.dir.entrylist; 
            found_dir = TRUE; 
            break; 
          }
        }
        if (found_dir == FALSE){
          //move to next block 
          curr_block_index = FAT[curr_block_index]; 
          if (curr_block_index != ENDOFCHAIN){
            //load the next block 
            readblock(&dir_block, curr_block_index); 
            dir_list = dir_block.dir.entrylist; 
          }
        }
      }

      if (found_dir == FALSE){
        // //if no such directory was found, return NULL as directory does not exists
        printf("No such directory found\n"); 
        //free the memory allocated by strdup() 
        free(original_rest); 
        return; 
        }
      found_dir = FALSE; 
    }
  }
  //find the directory in the specified directory 
  int curr_block_index = dirIndex; 
  //loop through blocks until end of chain is reached 
  while (curr_block_index != ENDOFCHAIN){
    //for every entry in directory entry list
    for (int i=0; i < dir_block.dir.nextEntry; i++){
      //check if directory with matching name is found
      if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].isdir == TRUE && dir_list[i].unused == FALSE){
        //check if the directory is empty 
        diskblock_t temp_dir_block; 
        fatentry_t temp_dirIndex = dir_list[i].firstblock; 
        readblock(&temp_dir_block, temp_dirIndex); 

        //go through the direcotry block by block and count the number of elements 
        int num_dir_elements = 0; 
        int temp_curr_block_index = temp_dirIndex; 
        while (temp_curr_block_index != ENDOFCHAIN){
          for (int j=0; j < temp_dir_block.dir.nextEntry; j++){
            //check if element is allocated 
            if (temp_dir_block.dir.entrylist[j].unused == FALSE){
              //if an entry, increment counter 
              num_dir_elements++; 
            }
          }
          //move to next directory block 
          temp_curr_block_index = FAT[temp_curr_block_index];
          if (temp_curr_block_index != ENDOFCHAIN){
            //load the next block 
            readblock(&temp_dir_block, temp_curr_block_index);  
          }
        }
        //direcotry has more than 2 elements it is not empty (it always has . and ..) 
        if (num_dir_elements > 2){
          printf("Directory: %s is not empty so cannot be removed\n", current_path_string); 
          //free the memory allocated by strdup() 
          free(original_rest); 
          return; 
        }
        //if empty, remove directory
        //remove directory from directory by setting unused to true
        dir_list[i].unused = TRUE; 

        //remove directory from FAT table
        //get the first block of the directory 
        int fat_block = dir_list[i].firstblock; 
        int temp_fat_block; 
        //go through chain until ENDOFCHAIN is reached and set each block to UNUSED
        while (fat_block != ENDOFCHAIN){
          //get the next block from the current block 
          temp_fat_block = FAT[fat_block]; 
          //set the current block to unused
          FAT[fat_block] = UNUSED;
          //set the current block to the next block in the chain
          fat_block = temp_fat_block; 
        }
        //write the updated directory to the virtual disk 
        writeblock(&dir_block, curr_block_index); 

        //update FAT table on the virtual disk 
        copyFAT(); 
        
        printf("Removed Directory: %s\n", dir_list[i].name);
        //free the memory allocated by strdup() 
        free(original_rest); 
        return; 
      } 
      
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&dir_block, curr_block_index); 
      dir_list = dir_block.dir.entrylist; 
    }
  } 
  printf("Directory: %s was not found in the specified directory\n", current_path_string);
  //free the memory allocated by strdup() 
  free(original_rest);  
}


void myremove(const char* path){
  //removes an existing file using the path 
  //when a file is removed its directory entry is set to unused 
  //and all FAT table blocks that file uses are set to unused, but 
  //the content is not overwritten on the disk

  //load the current directry from virtual disk 
  diskblock_t dir_block;
  fatentry_t dirIndex = currentDirIndex;  
  readblock(&dir_block, dirIndex); 
  direntry_t *dir_list = dir_block.dir.entrylist;
   
  //check if absolute path 
  if (path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk into the dir block
    readblock(&dir_block, rootDirIndex); 
    //set the dirIndex to rootDirIndex7
    dirIndex = rootDirIndex; 
    //set the dir to root dir entrylist 
    dir_list = dir_block.dir.entrylist; 
  }

  //Go through path and move dir into the required directory
  char *next_path_string; 
  char *rest = strdup(path); //create a mutalbe copy of the path 
  char *current_path_string; 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;
  //get the first string in the path
  next_path_string = strtok_r(rest, "/", &rest); 

  //go through directories in path until final directory is reached
  int found_dir = FALSE; 
  while (next_path_string != NULL){
    //save the current string
    current_path_string = next_path_string;
    //move to next string in path 
    next_path_string = strtok_r(rest, "/", &rest); 
    //if the next path string is not NULL, this means the current path string is a directory
    if (next_path_string != NULL){
      //change start dir into next directory, if does not exist return 
      //find the directory_t in entrylist which has name == current_path_string
      //need to go through all blocks that belong to the directory
      int curr_block_index = dirIndex;
      while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
        //for entry in directory entry list
        for (int i=0; i < dir_block.dir.nextEntry; i++){
          //check if name matches 
          if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].unused == FALSE){
            //found dir, update variables 
            dirIndex = dir_list[i].firstblock; 
            //read the directory block from virtual disk 
            readblock(&dir_block, dirIndex); 
            dir_list = dir_block.dir.entrylist; 
            found_dir = TRUE; 
            break; 
          }
        }
        if (found_dir == FALSE){
          //move to next block 
          curr_block_index = FAT[curr_block_index]; 
          if (curr_block_index != ENDOFCHAIN){
            //load the next block 
            readblock(&dir_block, curr_block_index); 
            dir_list = dir_block.dir.entrylist; 
          }
        }
      }

      if (found_dir == FALSE){
        // //if no such directory was found, return NULL as directory does not exists
        printf("No such directory found\n"); 
        //free memory allocated by strdup() 
        free(original_rest); 
        return; 
        }
      found_dir = FALSE; 
    }
  }
  //check if file exists in current directory by checking all blocks of the directory
  //if the file exists, remove it
  int curr_block_index = dirIndex; 
  //loop through blocks until end of chain is reached 
  while (curr_block_index != ENDOFCHAIN){
    //for every entry in the directory entry list
    for (int i=0; i < dir_block.dir.nextEntry; i++){
      //check if name matches 
      if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].unused == FALSE){
        //file was found 
        printf("Removing File: %s\n", current_path_string);

        //remove file from directory by setting unused to true
        dir_list[i].unused = TRUE; 

        //remove file from FAT table
        //get the first block of the file 
        int file_block = dir_list[i].firstblock; 
        int temp_block; 
        //go through chain until ENDOFCHAIN is reached and set each block to UNUSED
        while (file_block != ENDOFCHAIN){
          //get the next block from the current block 
          temp_block = FAT[file_block]; 
          //set the current block to unused
          FAT[file_block] = UNUSED;
          //set the current block to the next block in the chain
          file_block = temp_block; 
        }
        //write the updated directory to the virtual disk 
        writeblock(&dir_block, curr_block_index); 

        //update FAT table on the virtual disk 
        copyFAT(); 

        //free memory allocated by strdup() 
        free(original_rest); 
        //return  
        return; 
      } 
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&dir_block, curr_block_index); 
      dir_list = dir_block.dir.entrylist; 
    }
  }
  printf("File: %s was not found in the specified directory\n", current_path_string);
  //free memory allocated by strdup() 
  free(original_rest);  
}


void mychdir(const char *path){
  //change the global directory to the directory specified 
  //in path
  printf("Changing directory to: %s\n", path); 

  //define start directory  
  fatentry_t startDirIndex = currentDirIndex;
  diskblock_t start_dir_block;
  //read the start directory block from the virtual disk 
  readblock(&start_dir_block, startDirIndex);  
  direntry_t *startDir = start_dir_block.dir.entrylist;

  //check if absolute path 
  if (path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk 
    readblock(&start_dir_block, rootDirIndex); 
    //set the startDirIndex to rootDirIndex7
    startDirIndex = rootDirIndex; 
    //set the startDir to root dir entrylist 
    startDir = start_dir_block.dir.entrylist; 
  }

  //variables for spliting the string 
  char *dir_name; 
  char *rest = strdup(path); // Create a mutable copy of the path 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;
  //go through every directory in the path
  while ((dir_name = strtok_r(rest, "/", &rest))){
    //check if path exists 
    int found_dir = FALSE; 
    int curr_block_index = startDirIndex;
    while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
      //for entry in directory entrylist 
      for (int i=0; i < start_dir_block.dir.nextEntry; i++){
        //check if name matches, the directory entry is a directory and is not deleted
        if (strcmp(startDir[i].name, dir_name) == 0 && startDir[i].isdir == TRUE && startDir[i].unused == FALSE){
          //found dir, update variables 
          startDirIndex = startDir[i].firstblock; 
          //read the directory block from virtual disk 
          readblock(&start_dir_block, startDirIndex); 
          startDir = start_dir_block.dir.entrylist; 
          found_dir = TRUE; 
          break; 
        }
      }
      if (found_dir == FALSE){
        //move to next block 
        curr_block_index = FAT[curr_block_index]; 
        if (curr_block_index != ENDOFCHAIN){
          //load the next block 
          readblock(&start_dir_block, curr_block_index); 
          startDir = start_dir_block.dir.entrylist; 
        }
      }
    }
    //if director was not found, cannot change into it 
    if (found_dir == FALSE){
      printf("Cannot change into directory as directory does not exist\n"); 
      //free memory allocated by strdup() 
      free(original_rest); 
      return; 
    }
    //set FALSE for next dir in path 
    found_dir = FALSE; 
  } 
  //move directory 
  currentDir = startDir; 
  currentDirIndex = startDirIndex; 

  //free memory allocated by strdup() 
  free(original_rest); 
}


char **mylistdir(const char* path){
  //move to directory defined by path and generate a list of 
  //char pointters that point to all elements in the directory 

  //define start directory  
  fatentry_t startDirIndex = currentDirIndex;
  diskblock_t start_dir_block;
  //read the start directory block from the virtual disk 
  readblock(&start_dir_block, startDirIndex);  
  direntry_t *startDir = start_dir_block.dir.entrylist;

  //check if absolute path 
  if (path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk 
    readblock(&start_dir_block, rootDirIndex); 
    //set the startDirIndex to rootDirIndex7
    startDirIndex = rootDirIndex; 
    //set the startDir to root dir entrylist 
    startDir = start_dir_block.dir.entrylist; 
  }

  /*go through each directory in path, until final directory is reached*/
  //variables for spliting the string 
  char *dir_name; 
  char *rest = strdup(path); // Create a mutable copy of the path 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;
  //move to wanted directory 
  int found_dir = FALSE; 
  while ((dir_name = strtok_r(rest, "/", &rest))){
    //need to go through all blocks that belong to the directory
    //go through dir block by block 
    int curr_block_index = startDirIndex; 
    while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
      //for directory entry in directory entry list
      for (int i=0; i < start_dir_block.dir.nextEntry; i++){
        //check if name matches 
        if (strcmp(startDir[i].name, dir_name) == 0 && startDir[i].unused == FALSE){
          //found dir, update start directory
          startDirIndex =  startDir[i].firstblock; 
          //read the start directory block from the virtual disk 
          readblock(&start_dir_block, startDirIndex);  
          startDir = start_dir_block.dir.entrylist; 
          found_dir = TRUE;  
          break; 
        } 
      }
      //move to next block 
      if (found_dir == FALSE){
        curr_block_index = FAT[curr_block_index]; 
        if (curr_block_index != ENDOFCHAIN){
          //load the next block 
          readblock(&start_dir_block, curr_block_index);
          startDir = start_dir_block.dir.entrylist; 
        }
      }
    }
    if (found_dir == FALSE){
      //if no such directory was found, return NULL as directory does not exists
      printf("No such directory!\n");
      //free memory allocated by strdup() 
      free(original_rest);  
      return NULL; 
      }
    found_dir = FALSE; 
  }

  //count the number of blocks needed
  diskblock_t temp_dir_block = start_dir_block; 
  int num_elements = 1; //need at least 1 for the NULL element as termination 
  int temp_curr_dir_block = startDirIndex; 
  //go through directory block by block 
  while (temp_curr_dir_block != ENDOFCHAIN){
    //add the number of elements in block, excluding unused elements 
    for (int i=0; i < temp_dir_block.dir.nextEntry; i++){
      //if element is unused (deleted) do not include it
      if (temp_dir_block.dir.entrylist[i].unused == FALSE){
        num_elements += 1; 
      }
    }
    //move to next block 
    temp_curr_dir_block = FAT[temp_curr_dir_block]; 
    if (temp_curr_dir_block != ENDOFCHAIN){
      //load the next block 
      readblock(&temp_dir_block, temp_curr_dir_block);
    }
  }
   
  // Allocate memory for the list of directory entries
  char **directoryEntries = (char **)malloc(sizeof(char *) * (num_elements));
  // Check for memory allocation failure
  if (directoryEntries == NULL) {
    perror("Memory allocation error");
    //free memory allocated by strdup() 
    free(original_rest); 
    return NULL;
  }

  //go through current directory list and fill in the directoryEntries array 
  int index_counter = 0; 
  //go through directory block by block 
  int curr_block_index = startDirIndex; 
  while (curr_block_index != ENDOFCHAIN){
    //for entry in directory entry list
    for (int i = 0; i < start_dir_block.dir.nextEntry; i++) { 
      //only if the file is not unused (not deleted)
      if (startDir[i].unused == FALSE){
        //Create a mutable copy of the name
        directoryEntries[index_counter] = strdup(startDir[i].name);
        index_counter++; 
      }
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&start_dir_block, curr_block_index);
      startDir = start_dir_block.dir.entrylist; 
    }
  }
  //terminate the list with NULL
  directoryEntries[index_counter] = NULL;

  //free memory allocated by strdup() 
  free(original_rest); 
  //return pointer to list of directory entries
  return directoryEntries;
}


void mymkdir(const char* path){
  //create the directory (and parent directories if needed) 
  //defined by path
  
  //define start directory  
  fatentry_t startDirIndex = currentDirIndex;
  diskblock_t start_dir_block;
  //read the start directory block from the virtual disk 
  readblock(&start_dir_block, startDirIndex);  
  direntry_t *startDir = start_dir_block.dir.entrylist;

  //check if absolute path 
  if (path[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk 
    readblock(&start_dir_block, rootDirIndex); 
    //set the startDirIndex to rootDirIndex7
    startDirIndex = rootDirIndex; 
    //set the startDir to root dir entrylist 
    startDir = start_dir_block.dir.entrylist; 
  }

  //variables for spliting the string 
  char *dir_name; 
  char *rest = strdup(path); // Create a mutable copy of the path 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;
  //go through every directory in the path
  while ((dir_name = strtok_r(rest, "/", &rest))){
    //go throuhg the all blocks of the directory
    int found_dir = FALSE; 
    int curr_block_index = startDirIndex;
    while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
      //for entry in directory entry list 
      for (int i=0; i < start_dir_block.dir.nextEntry; i++){
        //check if name matches, entry is a directory and entry is not deleted
        if (strcmp(startDir[i].name, dir_name) == 0 && startDir[i].isdir == TRUE && startDir[i].unused == FALSE){
          //found dir, update variables 
          startDirIndex = startDir[i].firstblock; 
          //read the directory block from virtual disk 
          readblock(&start_dir_block, startDirIndex); 
          startDir = start_dir_block.dir.entrylist; 
          found_dir = TRUE; 
          break; 
        }
      }
      if (found_dir == FALSE){
        //move to next block 
        curr_block_index = FAT[curr_block_index]; 
        if (curr_block_index != ENDOFCHAIN){
          //load the next block 
          readblock(&start_dir_block, curr_block_index); 
          startDir = start_dir_block.dir.entrylist; 
        }
      }
    }

    //if directory exist, move into directory, else make new 
    //if directory does not exist 
    if (found_dir == FALSE){
      //temporarily save the parent directory index to save in ..
      fatentry_t partenStartDirIndex = startDirIndex;

      //get the directory block with a free directory entry 
      startDirIndex = getFreeDirEntryBlock(startDirIndex);
      //load the directory block that has a free directory entry 
      readblock(&start_dir_block, startDirIndex); 
      startDir = start_dir_block.dir.entrylist;

      //get the next free directory entry, 
      int free_entry_index = start_dir_block.dir.nextEntry; 
      for (int i = 0; i < start_dir_block.dir.nextEntry; i++){
        //if there is an unused element in the directory
        if (start_dir_block.dir.entrylist[i].unused == TRUE){
          free_entry_index = i; 
          break; 
        }
      }
      //if there was no available element, add to the end 
      if (free_entry_index == start_dir_block.dir.nextEntry){
        //move the next entry counter
        start_dir_block.dir.nextEntry++;
      }
      //reset the memory of the name char array 
      memset(startDir[free_entry_index].name, '\0', MAXNAME); 
      //copy the directory name to directory entry 
      strcpy(startDir[free_entry_index].name, dir_name);
      //set unused to FALSE 
      startDir[free_entry_index].unused = FALSE; 
      //set isdir to TRUE  
      startDir[free_entry_index].isdir = TRUE;
      //initialize the file length to 0 because its a directory 
      startDir[free_entry_index].filelength = 0;  
      //get unallocated FAT block 
      int block_num = getFreeFATBlockIndex(); 
      if (block_num == -1){
        //no available FAT blocks 
        printf("No Space left on disk\n");
        //free memory allocated by strdup() 
        free(original_rest); 
        return;
      }
      //Set the FAT block to ENDOFCHAIN
      FAT[block_num] = ENDOFCHAIN; 
      //set the first block entry in the directory 
      startDir[free_entry_index].firstblock = block_num;
      //write the new directory block to virtual disk 
      writeblock(&start_dir_block, startDirIndex); 

      //move inside the new directory
      startDirIndex = block_num;
      readblock(&start_dir_block, startDirIndex);
      startDir = start_dir_block.dir.entrylist;  
      //intialize the new directory
      start_dir_block.dir.isdir = TRUE;
      start_dir_block.dir.nextEntry = 0;

      /*CGS A5-A4 update*/
      //add the "." enrtry  in the new directory 
      int dot_entry_index = start_dir_block.dir.nextEntry;
      start_dir_block.dir.nextEntry++; //move the nextentry pointer to next element
      //reset the memory of the name char array 
      memset(startDir[dot_entry_index].name, '\0', MAXNAME); 
      //copy the name of the directory and set the other values 
      strcpy(startDir[dot_entry_index].name, ".");
      startDir[dot_entry_index].unused = FALSE; 
      startDir[dot_entry_index].isdir = TRUE; 
      startDir[dot_entry_index].modtime = 0; 
      startDir[dot_entry_index].filelength = 0; 
      startDir[dot_entry_index].firstblock = startDirIndex; //point to itself

      //add the ".." entry in the new directory that points to the parent 
      int dotdot_entry_index = start_dir_block.dir.nextEntry; 
      start_dir_block.dir.nextEntry++; //move the nextentry pointer to next element
      //reset the memory of the name char array 
      memset(startDir[dotdot_entry_index].name, '\0', MAXNAME); 
      //copy the name of the directory and set the other values
      strcpy(startDir[dotdot_entry_index].name, "..");
      startDir[dotdot_entry_index].unused = FALSE; 
      startDir[dotdot_entry_index].isdir = TRUE; 
      startDir[dotdot_entry_index].modtime = 0; 
      startDir[dotdot_entry_index].filelength = 0; 
      startDir[dotdot_entry_index].firstblock = partenStartDirIndex; //point to parent 
      //write the new directory block to virtual disk 
      writeblock(&start_dir_block, startDirIndex); 
      /*end update*/
    }
  } 
  //update the FAT block on the virtual disk 
  copyFAT(); 

  //free memory allocated by strdup() 
  free(original_rest); 
}


int myfgetc(MyFILE *stream){
  //read a single character of the file

  //check if the end of the buffer has been reached 
  if (stream->pos == BLOCKSIZE){
    //get the next block index 
    int next_block = FAT[stream->blockno]; 
    if (next_block == ENDOFCHAIN){
      return EOF; 
    }
    //update the block number and reset the position
    stream->blockno = next_block; 
    stream->pos = 0; 

    //read the next block into the buffer 
    readblock(&stream->buffer, stream->blockno); 
  }

  //check if end of file has been reached 
  if (isEndOfFile(stream->firstblockno, stream->blockno, stream->pos, stream->dirblockno)){
    //if end of file, return EOF 
    return EOF; 
  }

  //read the byte at the current position 
  int byte = stream->buffer.data[stream->pos]; 
  //increment the position
  stream->pos++; 

  //return the byte 
  return byte; 
}

int isEndOfFile(const int start_block, const int current_block, int current_pos, int dir_block_index){
  //check if the current position in the file is the end of the file 
  
  //find the number of blocks between start block and current block 
  //initialize the next block number 
  int next_block_no = start_block; 
  //initialize the number of blocks 
  int num_blocks = 0;
  //go through FAT table and find the current block 
  while (next_block_no != current_block){
    next_block_no = FAT[next_block_no]; 
    num_blocks++; 
  }
  //calculate number of bytes read so far 
  int num_bytes_read = num_blocks*BLOCKSIZE + current_pos; 

  //get the file length from the directory entry  
  int file_length = 0; 
  //load the current directry from virtual disk 
  diskblock_t dir_block;
  readblock(&dir_block, dir_block_index); 
  //check if the file exists in the entries in the current directory
  //go through all blocks that belong to the directory
  int curr_block_index = dir_block_index;
  while (curr_block_index != ENDOFCHAIN){
    for (int i=0; i < dir_block.dir.nextEntry; i++){
      //check if first block  matches 
      if (dir_block.dir.entrylist[i].firstblock == start_block && dir_block.dir.entrylist[i].unused == FALSE){
        //file was found, load file_length
        file_length = dir_block.dir.entrylist[i].filelength;
        // check if we have read more bytes than the file length
        if (num_bytes_read > file_length){
          return 1; 
        }
        return 0;
      } 
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&dir_block, curr_block_index); 
    }
  }
  return 0;
}


//myfclose function 
void myfclose(MyFILE *stream){
  /*
  if file was opened in write mode, write the buffer to the last block 
  of the chain and update file length. If opened in read mode, this is not 
  needed as read mode only allows the user to read and not modify the file  
  */
  if (strcmp(stream->mode, "w") == 0){
    //fill the unused part of the with '\0' to avoid clutter 
    for (int i=stream->pos; i < BLOCKSIZE; i++){
      stream->buffer.data[i] = '\0'; 
    }

    //wirte the buffer to the block
    writeblock(&stream->buffer, stream->blockno); 

    //calculate the length of the file 
    //get the number of blocks 
    int num_blocks = get_num_blocks(stream->firstblockno); 
    //calculate the number of characters in file
    int file_length = (num_blocks-1)*BLOCKSIZE + stream->pos; 

    //find the file in the directory entrylist 
    //load the current directry from virtual disk 
    diskblock_t dir_block;
    readblock(&dir_block, stream->dirblockno); 
    //check if the file exists in the entries in the current director
    //go through all blocks that belong to the directory
    int curr_block_index = stream->dirblockno;
    while (curr_block_index != ENDOFCHAIN){
      //for every entry in the directory entry list
      for (int i=0; i < dir_block.dir.nextEntry; i++){
        //check if first block  matches 
        if (dir_block.dir.entrylist[i].firstblock == stream->firstblockno && dir_block.dir.entrylist[i].unused == FALSE){
          //file was found 
          //update the files filelength and modtime 
          dir_block.dir.entrylist[i].filelength = file_length; 
          //write the updated block to virtual disk 
          writeblock(&dir_block, curr_block_index); 
          //free the stream and return 
          free(stream); 
          return;
        } 
      }
      //move to next block 
      curr_block_index = FAT[curr_block_index]; 
      if (curr_block_index != ENDOFCHAIN){
        //load the next block 
        readblock(&dir_block, curr_block_index); 
      }
    }
  }
  //free the stream (file) pointer 
  free(stream); 
}


//myfputc function 
void myfputc(int b, MyFILE *stream){
  //write a single character to the file 

  //write a byte to stream, only allowed when in write mode 
  if (strcmp(stream->mode, "w") != 0) {
    printf("File is not opened in write mode. Cannot write to the file.\n");
    return;
  }
  //check if the end of the buffer has been reached 
  if (stream->pos == BLOCKSIZE){
    //reached the end of the buffer, write block to buffer and create new block 
  
    //write block to the current block number on disk
    writeblock(&stream->buffer, stream->blockno); 

    //add new block from FAT to stream 
    int block_num = getFreeFATBlockIndex(); 
    if (block_num == -1){
      //no available FAT blocks 
      free(stream);
      printf("Could not alloced write block, no Space left on disk\n");
    }
    //update old FAT block to point to new FAT block 
    FAT[stream->blockno] = block_num; 
    //Set the new FAT block to ENDOFCHAIN
    FAT[block_num] = ENDOFCHAIN; 
    //update the FAT block on the virtual disk 
    copyFAT(); 

    //move to next block 
    stream->blockno = block_num; 
    //write byte 
    stream->buffer.data[0] = b; 
    //update the pos of the stream buffer 
    stream->pos = 1; //starting from the second element   
  }else{
    //not at the end, can simply write byte 
    stream->buffer.data[stream->pos] = b; 
    //increment the pos
    stream->pos++; 
  }
}


int get_num_blocks(const int start_block){
  //count the number of blocks that are in a chain

  //initialize the next block number 
  int next_block_no = start_block; 
  //initialize the number of blocks 
  int num_blocks = 1; 
  //go through FAT table until ENDOFCHAIN 
  while (FAT[next_block_no] != ENDOFCHAIN){
    next_block_no = FAT[next_block_no]; 
    num_blocks++; 
  }
  return num_blocks; 
}


//myfopen function 
MyFILE* myfopen(const char *filename, const char *mode){
  //open the file defined by filename (which can be a relative or 
  // absolute path) in the specified mode. If the directories in the 
  //path do not exist, create the directories and then add the file. 
  //if something goes wrong print error message and return NULL 

  // Check if the mode is "r" or "w", if neither, cannot perform operation 
  if (strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0) {
    printf("Mode: %s not supported", mode); 
    return NULL; 
  }

  //create file descriptor 
  MyFILE *file = (MyFILE *)malloc(sizeof(MyFILE));
  if (file == NULL){
    printf("Failed to allocated memory for file\n"); 
    return NULL; 
  }
  strcpy(file->mode, mode); //file->mode (pointer) or file.mode (normal)
  //set pos to 0, to the first byte in the block 
  file->pos = 0; 

  //load the current directry from virtual disk 
  diskblock_t dir_block;
  fatentry_t dirIndex = currentDirIndex;  
  readblock(&dir_block, dirIndex); 
  direntry_t *dir_list = dir_block.dir.entrylist;
  
  /*adapted so that an absolute or relative path can be used (CGS B3-B1)*/ 
  //check if absolute path 
  if (filename[0] == '/'){
    //change from the current directory to the root directory 
    //read the root directory from the virtual disk into the dir block
    readblock(&dir_block, rootDirIndex); 
    //set the dirIndex to rootDirIndex7
    dirIndex = rootDirIndex; 
    //set the dir to root dir entrylist 
    dir_list = dir_block.dir.entrylist; 
  }

  //Go through filename (which can be a path, and move dir in the required directory)
  char *next_path_string; 
  char *rest = strdup(filename); //create a mutalbe copy of the path 
  char *current_path_string; 
  //Save the original pointer so that the memory can be freed 
  char *original_rest = rest;

  //get the first string in the path
  next_path_string = strtok_r(rest, "/", &rest); 

  //go through directories in path until final directory is reached
  int found_dir = FALSE; 
  while (next_path_string != NULL){
    //save the current string
    current_path_string = next_path_string;
    //move to next string in path 
    next_path_string = strtok_r(rest, "/", &rest); 
    //if the next path string is not NULL, this means the current path string is a directory
    if (next_path_string != NULL){
      //change start dir into next directory, if does not exist return NULL
      //need to go through all blocks that belong to the directory
      int curr_block_index = dirIndex;
      while (curr_block_index != ENDOFCHAIN && found_dir == FALSE){
        //for every element in the directory entry list
        for (int i=0; i < dir_block.dir.nextEntry; i++){
          //check if name matches, check if directory and check that element is not deleted
          if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].isdir == TRUE && dir_list[i].unused == FALSE){
            //found dir, update variables 
            dirIndex = dir_list[i].firstblock; 
            //read the directory block from virtual disk 
            readblock(&dir_block, dirIndex); 
            dir_list = dir_block.dir.entrylist; 
            found_dir = TRUE; 
            break; 
          }
        }
        if (found_dir == FALSE){
          //move to next block 
          curr_block_index = FAT[curr_block_index]; 
          if (curr_block_index != ENDOFCHAIN){
            //load the next block 
            readblock(&dir_block, curr_block_index); 
            dir_list = dir_block.dir.entrylist; 
          }
        }
      }

      if (found_dir == FALSE){
        // //if no such directory was found, return NULL as directory does not exists
        // return NULL; 
       
        /*A5-A4 adaptation, create directory if it does not exist*/
        // create new directory in the current directory and return the index
        dirIndex = create_dir(dirIndex, current_path_string);
        
        // printf("test_dirIndex = %d\n", test_dirIndex);
        if (dirIndex == -1){
          printf("Something went wrong, could not create directory: %s", current_path_string); 
          //free memory allocated by strdup() 
          free(original_rest); 
          return NULL; 
        }
        //read the directory block from virtual disk 
        readblock(&dir_block, dirIndex); 
        dir_list = dir_block.dir.entrylist; 
        /*End A5-A4 adaptation*/
        }
      found_dir = FALSE; 
    }
  }
  /*end of change for CGS B3-B1*/

  //check if the file exists in the entries in the current directory 
  //need to go through all blocks that belong to the directory
  int curr_block_index = dirIndex; 
  while (curr_block_index != ENDOFCHAIN){
    //for every element in the directory entry list
    for (int i=0; i < dir_block.dir.nextEntry; i++){
      //check if name matches 
      if (strcmp(dir_list[i].name, current_path_string) == 0 && dir_list[i].unused == FALSE){
        //file was found, load in file details to file descriptor
        file->blockno = dir_list[i].firstblock;
        file->firstblockno = dir_list[i].firstblock;
        file->dirblockno = dirIndex; 
        //if read only mode, read files contents into buffer 
        if (strcmp(mode, "r") == 0){
          //copy block to buffer
          readblock(&file->buffer, file->blockno); 
        }
        //free memory allocated by strdup() 
        free(original_rest); 
        //return file 
        return file; 
      } 
    }
    //move to next block 
    curr_block_index = FAT[curr_block_index]; 
    if (curr_block_index != ENDOFCHAIN){
      //load the next block 
      readblock(&dir_block, curr_block_index); 
      dir_list = dir_block.dir.entrylist; 
    }
  }
  
  //if was not found and in w mode, crate new file 
  if (strcmp(mode, "w") == 0){
    //get the next block that has a free directory element
    dirIndex = getFreeDirEntryBlock(dirIndex);
    readblock(&dir_block, dirIndex); 
    dir_list = dir_block.dir.entrylist;

    //get the next free directory entry, 
    int free_entry_index = dir_block.dir.nextEntry; 
    for (int i = 0; i < dir_block.dir.nextEntry; i++){
      //if there is an unused element in the directory
      if (dir_block.dir.entrylist[i].unused == TRUE){
        free_entry_index = i; 
        break; 
      }
    }
    //if there was no available element, add to the end 
    if (free_entry_index == dir_block.dir.nextEntry){
      //move the next entry counter
      dir_block.dir.nextEntry++;
    }

    //reset the memory of the name char array 
    memset(dir_list[free_entry_index].name, '\0', MAXNAME); 
    //copy the file name to directory entry 
    strcpy(dir_list[free_entry_index].name, current_path_string); 
    //set unused to FALSE 
    dir_list[free_entry_index].unused = FALSE; 
    //set isdir to FALSE 
    dir_list[free_entry_index].isdir = FALSE; 
    //set modification time to 0 as it has not been modifed (modtime used to write back to disk)
    dir_list[free_entry_index].modtime = 0; 
    // time(&dir_list[i].modtime); 
    //initialize the file length to 0 because the file is intially empty 
    dir_list[free_entry_index].filelength = 0;  

    //get unallocated FAT block 
    int block_num = getFreeFATBlockIndex(); 
    if (block_num == -1){
    //no available FAT blocks 
      free(file);
      printf("No Space left on disk\n");
      //free memory allocated by strdup() 
      free(original_rest); 
      return NULL;
    }

    //assign block number and firstblockno
    file->blockno = block_num;
    file->firstblockno = block_num;  
    //assign the dirblockno to the file 
    file->dirblockno = dirIndex; 

    //Set the FAT block to ENDOFCHAIN
    FAT[block_num] = ENDOFCHAIN; 
    //update the FAT block on the virtual disk 
    copyFAT(); 
   
    //set the first block entry in the directory 
    dir_list[free_entry_index].firstblock = file->blockno;

    //write the new directory block to virtual disk 
    writeblock(&dir_block, dirIndex);

    //free memory allocated by strdup() 
    free(original_rest); 

    //return the file pointer 
    return file;
  }

  printf("File not found and unable to create it\n");
  //free memory allocated by strdup() 
  free(original_rest); 
  return NULL;
}

int getFreeFATBlockIndex(){
  //get the index of the next free FAT block in the FAT Table

  //loop the blocks, dont need to check the firstblock, the root dir 
  //block and the FAT table entry blocks as they are always used
  for(int i=(MAXBLOCKS/FATENTRYCOUNT)+2; i < MAXBLOCKS; i++){
    //if a fat block is unused, return its index
    if (FAT[i] == UNUSED){
      return i; 
    }
  }
  return -1;
}


fatentry_t getFreeDirEntryBlock(fatentry_t dir_index){
  //go through the blocks of the directory and find a block that 
  //has a free element in the directory entry list. If now such 
  //block exists, create new directory block and return index of 
  //the new block

  //load the current directry from virtual disk 
  diskblock_t dir_block;
  readblock(&dir_block, dir_index); 

  //go through dir block by block
  int curr_block_index = dir_index; 
  int next_block_index = FAT[curr_block_index]; 
  while (dir_block.dir.nextEntry >= DIRENTRYCOUNT && next_block_index != ENDOFCHAIN){
    //go through the entrylist and check if there is an unused element 
    for (int i=0; i < dir_block.dir.nextEntry; i++){
      if (dir_block.dir.entrylist[i].unused == TRUE){
        //return the index of this block as it has a free element 
        printf("Found unused element in entrylist\n"); 
        return curr_block_index; 
      }
    }
    //load the next block into dir block, 
    readblock(&dir_block, next_block_index);
    curr_block_index = next_block_index;
    next_block_index = FAT[curr_block_index];
  }

  //go through the entrylist of the last dir block and check if there is an unused element 
  for (int i=0; i < dir_block.dir.nextEntry; i++){
  if (dir_block.dir.entrylist[i].unused == TRUE){
    //return the index of this block as it has a free element 
    return curr_block_index; 
    }
  }

  //if the last block is also full 
  if (dir_block.dir.nextEntry >= DIRENTRYCOUNT){
    //create a new block 

    //get unallocated FAT block 
    int block_num = getFreeFATBlockIndex(); 
    if (block_num == -1){
      //no available FAT blocks 
      printf("No Space left on disk\n");
      return -1;
    }
    //if new block was allocated, link the blocks 
    FAT[curr_block_index] = block_num; 
    FAT[block_num] = ENDOFCHAIN;
    //move to new block 
    curr_block_index = block_num;
    

    //if new block was allocated, read the block 
    readblock(&dir_block, curr_block_index);
    //reset the block data otherwise the disk may contain old data
    for (int i=0; i < BLOCKSIZE; i++){
      dir_block.data[i] = '\0'; 
    }
    //initialize new block 
    dir_block.dir.isdir = TRUE; 
    dir_block.dir.nextEntry = 0; 
    //write the new directory block to virtual disk 
    writeblock(&dir_block, curr_block_index);

    //update the FAT block on the virtual disk 
    copyFAT(); 
  }

  //return the index of the directory block that has an free element 
  return curr_block_index; 
}


fatentry_t create_dir(fatentry_t dir_index, char* new_dir_name){
  //takes in a directory (as a directory index) and a new director name 
  //and creates a new directory in the given directory 

  //get the next free directory block
  diskblock_t dir_block;
  int dirIndex = getFreeDirEntryBlock(dir_index);
  readblock(&dir_block, dirIndex); 
  direntry_t *dir = dir_block.dir.entrylist;

  //get the next free directory entry, 
  int free_entry_index = dir_block.dir.nextEntry; 
  for (int i = 0; i < dir_block.dir.nextEntry; i++){
    //if there is an unused element in the directory
    if (dir_block.dir.entrylist[i].unused == TRUE){
      free_entry_index = i; 
      break; 
    }
  }
  //if there was no available element, add to the end 
  if (free_entry_index == dir_block.dir.nextEntry){
    //move the next entry counter
    dir_block.dir.nextEntry++;
  }

  //reset the memory of the name char array 
  memset(dir[free_entry_index].name, '\0', MAXNAME); 
  //create the new directory 
  strcpy(dir[free_entry_index].name, new_dir_name); 
  dir[free_entry_index].unused = FALSE; 
  dir[free_entry_index].isdir = TRUE;
  dir[free_entry_index].filelength = 0;  
  //get unallocated FAT block 
  int block_num = getFreeFATBlockIndex(); 
  if (block_num == -1){
    //no available FAT blocks 
    printf("No Space left on disk\n");
    return -1;
  }
  //Set the FAT block to ENDOFCHAIN
  FAT[block_num] = ENDOFCHAIN; 
  //write the FAT blocks to virtual disk 
  copyFAT(); 
  //set the first block entry in the directory 
  dir[free_entry_index].firstblock = block_num;
    
  //write the new directory block to virtual disk 
  writeblock(&dir_block, dirIndex);
  
  //move inside the new directory
  dirIndex = block_num;
  readblock(&dir_block, dirIndex);
  dir = dir_block.dir.entrylist;  
  //reset the block data otherwise the disk may contain old data
  for (int i=0; i < BLOCKSIZE; i++){
    dir_block.data[i] = '\0'; 
  }
  //intialize the new directory
  dir_block.dir.isdir = TRUE;
  dir_block.dir.nextEntry = 0;

  //add the "." enrtry  in the new directory 
  int dot_entry_index = dir_block.dir.nextEntry;
  dir_block.dir.nextEntry++; //move the nextentry pointer to next element
  //reset the memory of the name char array 
  memset(dir[dot_entry_index].name, '\0', MAXNAME); 
  //copy the directory name and set the other values 
  strcpy(dir[dot_entry_index].name, ".");
  dir[dot_entry_index].unused = FALSE; 
  dir[dot_entry_index].isdir = TRUE;  
  dir[dot_entry_index].filelength = 0; 
  dir[dot_entry_index].firstblock = dirIndex; //point to itself

  //add the ".." entry in the new directory that points to the parent 
  int dotdot_entry_index = dir_block.dir.nextEntry; 
  dir_block.dir.nextEntry++; //move the nextentry pointer to next element
  //reset the memory of the name char array 
  memset(dir[dotdot_entry_index].name, '\0', MAXNAME); 
  //copy the directory name and set the other values 
  strcpy(dir[dotdot_entry_index].name, "..");
  dir[dotdot_entry_index].unused = FALSE; 
  dir[dotdot_entry_index].isdir = TRUE; 
  dir[dotdot_entry_index].filelength = 0; 
  dir[dotdot_entry_index].firstblock = dir_index; //point to parent 

  //write the new directory block to virtual disk 
  writeblock(&dir_block, dirIndex); 

  //return the dir index of the newly created directory
  return dirIndex; 
}


/* read and write FAT
 * 
 * please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 * 
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
 *                FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
 */

/* implement format()
 */
void format ( ){
  //format the virtual disk by clearing all the content 
  //and intializing the first block, the root directory
  //and the FAT blocks on the virtual disk and the global 
  //FAT so that it can be used later to allocate blocks

   diskblock_t block ;
   direntry_t  rootDir ;
   int         pos             = 0 ;
   int         fatentry        = 0 ;
   int         fatblocksneeded =  (MAXBLOCKS / FATENTRYCOUNT ) ;

  //reset the full virtual disk 
  //for every block of the virtual disk 
  for (int i = 0; i < MAXBLOCKS; i++){
    //for every element of the virtual disk block
    for (int j=0; j < BLOCKSIZE; j++){
      //set the data to \0 to remove clutter
      virtualDisk[i].data[j] = '\0'; 
    } 
  }


   /* prepare block 0 : fill it with '\0',
    * use strcpy() to copy some text to it for test purposes
	* write block 0 to virtual disk
	*/
  //initialize disk block 0
  for (int i=0; i < BLOCKSIZE; i++){
    block.data[i] = '\0'; 
  }
  //copy the wanted text into block 0
  strcpy(block.data, "CS3026 Operating Systems Assessment");
  //write block to virtual disk 
  writeblock(&block, 0); 

	/* prepare FAT table
	 * write FAT blocks to virtual disk
	 */
  
  //intialize FAT entries 
  FAT[0] = ENDOFCHAIN; //first block cannot be used 
  //intialize all remaining FAT entries to UNUSED
  for (int i=1; i < MAXBLOCKS; i++){
    FAT[i] = UNUSED; 
  }
  
  //allocate blocks for FAT in FAT 
  for (int i=1; i < fatblocksneeded; i++){
    FAT[i] = i+1; //link to next block 
  }
  //add end chain at last block 
  FAT[fatblocksneeded] = ENDOFCHAIN; 
  //reserve block for root directory
  FAT[fatblocksneeded+1] = ENDOFCHAIN; 
  //update root dir index
  rootDirIndex = fatblocksneeded+1; 
  //write the FAT blocks to virtual disk 
  copyFAT(); 

	 /* prepare root directory
	  * write root directory block to virtual disk
	  */

  //reset the block text
  for (int i=0; i < BLOCKSIZE; i++){
    block.data[i] = '\0'; 
  }
  //initilalize the root block
  block.dir.isdir = TRUE; 
  //next entry specifies the next free entry in the dir
  //next entry: first entry in the element list 
  block.dir.nextEntry = 0; 

  
  /*CGS A5-A4 updat */
  //add the "." enrtry  in the new directory 
  int dot_entry_index = block.dir.nextEntry;
  block.dir.nextEntry++; //move the nextentry pointer to next element
  strcpy(block.dir.entrylist[dot_entry_index].name, ".");
  block.dir.entrylist[dot_entry_index].unused = FALSE; 
  block.dir.entrylist[dot_entry_index].isdir = TRUE; 
  block.dir.entrylist[dot_entry_index].modtime = 0; 
  block.dir.entrylist[dot_entry_index].filelength = 0; 
  block.dir.entrylist[dot_entry_index].firstblock = rootDirIndex; //point to itself

  /*add the ".." entry in the new directory that points to the parent
  since root does not have a parent it also points to root, just as 
  in UNIX*/ 
  int dotdot_entry_index = block.dir.nextEntry; 
  block.dir.nextEntry++; //move the nextentry pointer to next element
  strcpy(block.dir.entrylist[dotdot_entry_index].name, "..");
  block.dir.entrylist[dotdot_entry_index].unused = FALSE; 
  block.dir.entrylist[dotdot_entry_index].isdir = TRUE; 
  block.dir.entrylist[dotdot_entry_index].modtime = 0; 
  block.dir.entrylist[dotdot_entry_index].filelength = 0; 
  block.dir.entrylist[dotdot_entry_index].firstblock = rootDirIndex; //point to itself 
  /*end update*/

  //write the root directory to the virtual disk 
  writeblock(&block, rootDirIndex); 
  //set currentDirIndex to rootDirIndex
  currentDirIndex = rootDirIndex;
  //set the currentDir
  currentDir = block.dir.entrylist; 
}


//function that copies FAT into the virtual disk 
void copyFAT(){
  //copy the part of the FAT that is currently used to the virtual disk

  //find the last used FAT block
  int last_used_FAT_index = FATENTRYCOUNT; 
  for (int i=FATENTRYCOUNT; i < MAXBLOCKS; i++){
    if (FAT[i] != UNUSED){
      last_used_FAT_index = i; 
    }
  }
  //get the number of full blocks 
  int num_blocks_needed = last_used_FAT_index / FATENTRYCOUNT; 
  //if the modulo is greater than 0, an aditional block is needed 
  if (last_used_FAT_index % FATENTRYCOUNT != 0){
    num_blocks_needed += 1; 
  }

  //create a new block
  diskblock_t block; 
  //remove the clutter of the block
  for (int i=0; i < BLOCKSIZE; i++){
    block.data[i] = '\0'; 
  }
  //copy the needed number of FAT blocks to the virtual disk 
  int fatentry = 0; 
  for (int i=0; i < num_blocks_needed; i++){
    //initialize a block with fat entries 
    for (int j=0; j <FATENTRYCOUNT; j++){
      //add FAT data to block 
      block.fat[i] = FAT[fatentry]; 
      //move to next FAT entry
      fatentry++; 
    }
    //Write FAT block to virtual disk
    writeblock(&block, i+1);
  }
}

/* use this for testing
 */
void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

