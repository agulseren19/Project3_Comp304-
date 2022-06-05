#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <fstream>
#include <list>
#include <iostream>
#include "fat.h"
#include "fat_file.h"
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>


/**
 * Write inside one block in the filesystem.
 * @param  fs           filesystem
 * @param  block_id     index of block in the filesystem
 * @param  block_offset offset inside the block
 * @param  size         size to write, must be less than BLOCK_SIZE
 * @param  buffer       data buffer
 * @return              written byte count
 */
int mini_fat_write_in_block(const FAT_FILESYSTEM *fs, const int block_id, const int block_offset, const int size, const void * buffer) {
	assert(block_offset >= 0);
	assert(block_offset < fs->block_size);
	assert(size + block_offset <= fs->block_size);

	int written = 0;

	// TODO: write in the real file.
	if((block_offset < 0) || (block_offset >= fs->block_size) || (size + block_offset > fs->block_size) ){
		return written;
	}
	if(fs ==NULL){
		return written;
	}


	if(fs->fd<0){
		perror("Cannot open virtual filesystem");
		exit(-1);
	}
	if (lseek(fs->fd,fs->block_size*block_id+block_offset,SEEK_SET) < 0) {
		perror("error in lseek");
		exit(-1);
	}
	if (write(fs->fd,buffer,size) < 0) {
		perror("error in write");
		exit(-1);
	}
	else{
		written+=size;
	}

	return written;
}

/**
 * Read inside one block in the filesystem
 * @param  fs           filesystem
 * @param  block_id     index of block in the filesystem
 * @param  block_offset offset inside the block
 * @param  size         size to read, must fit inside the block
 * @param  buffer       buffer to write the read stuff to
 * @return              read byte count
 */
int mini_fat_read_in_block(const FAT_FILESYSTEM *fs, const int block_id, const int block_offset, const int size, void * buffer) {
	assert(block_offset >= 0);
	assert(block_offset < fs->block_size);
	assert(size + block_offset <= fs->block_size);

	int readd = 0;

	// TODO: read from the real file.
	if((block_offset < 0) || (block_offset >= fs->block_size) || (size + block_offset > fs->block_size) ){
		return readd;
	}
	if(fs ==NULL){
		return readd;
	}
	if(fs->fd<0){
		perror("Cannot create virtual disk file");
		exit(-1);
	}
	if (lseek(fs->fd,fs->block_size*block_id+block_offset,SEEK_SET) < 0) {
		perror("error in lseek");
		exit(-1);
	}
	if (read(fs->fd,buffer,size) < 0) {
		perror("error in read");
		exit(-1);
	}
	else{
		readd+=size;
	}

	return readd;
}


/**
 * Find the first empty block in filesystem.
 * @return -1 on failure, index of block on success
 */
int mini_fat_find_empty_block(const FAT_FILESYSTEM *fat) {
	// TODO: find an empty block in fat and return its index.
	long unsigned int i;
	if(fat!=NULL){
		for ( i=0; i<fat->block_map.size(); i++) {
			if (fat->block_map[i]==EMPTY_BLOCK) {
				return i;
			}

		}
	}
	return -1;
}

/**
 * Find the first empty block in filesystem, and allocate it to a type,
 * i.e., set block_map[new_block_index] to the specified type.
 * @return -1 on failure, new_block_index on success
 */
int mini_fat_allocate_new_block(FAT_FILESYSTEM *fs, const unsigned char block_type) {
	int new_block_index = mini_fat_find_empty_block(fs);
	if (new_block_index == -1)
	{
		fprintf(stderr, "Cannot allocate block: filesystem is full.\n");
		return -1;
	}
	fs->block_map[new_block_index] = block_type;
	return new_block_index;
}

void mini_fat_dump(const FAT_FILESYSTEM *fat) {
	printf("Dumping fat with %d blocks of size %d:\n", fat->block_count, fat->block_size);
	for (int i=0; i<fat->block_count;++i) {
		printf("%d ", (int)fat->block_map[i]);
	}
	printf("\n");

	for (int i=0; i<fat->files.size(); ++i) {
		mini_file_dump(fat, fat->files[i]);
	}
}

static FAT_FILESYSTEM * mini_fat_create_internal(const char * filename, const int block_size, const int block_count) {
	FAT_FILESYSTEM * fat = new FAT_FILESYSTEM;
	fat->filename = filename;
	fat->block_size = block_size;
	fat->block_count = block_count;
	fat->block_map.resize(fat->block_count, EMPTY_BLOCK); // Set all blocks to empty.
	fat->block_map[0] = METADATA_BLOCK;
	int fat_fd=open(filename,O_RDWR|O_CREAT,0777);
	//if (fat_fd == NULL) {
	if(fat_fd<0){
		perror("Cannot create virtual disk file");
		exit(-1);
	}
	fat->fd=fat_fd;
	return fat;
}

/**
 * Create a new virtual disk file.
 * The file should be of the exact size block_size * block_count bytes.
 * Overwrites existing files. Resizes block_map to block_count size.
 * @param  filename    name of the file on real disk
 * @param  block_size  size of each block
 * @param  block_count number of blocks
 * @return             FAT_FILESYSTEM pointer with parameters set.
 */
FAT_FILESYSTEM * mini_fat_create(const char * filename, const int block_size, const int block_count) {

	FAT_FILESYSTEM * fat = mini_fat_create_internal(filename, block_size, block_count);
	// TODO: create the corresponding virtual disk file with appropriate size.

	if (ftruncate(fat->fd,block_size*block_count) < 0) {
		perror("Cannot size virtual disk file");
		exit(-1);
	}
	return fat;
}

/**
 * Save a virtual disk (filesystem) to file on real disk.
 * Stores filesystem metadata (e.g., block_size, block_count, block_map, etc.)
 * in block 0.
 * Stores file metadata (name, size, block map) in their corresponding blocks.
 * Does not store file data (they are written directly via write API).
 * @param  fat virtual disk filesystem
 * @return     true on success
 */
bool mini_fat_save(const FAT_FILESYSTEM *fat) {
	//FILE * fat_fd = fopen(fat->filename, "r+");
	//if (fat_fd == NULL) {
	//	perror("Cannot save fat to file");
	//	return false;
	//}
	// TODO: save all metadata (filesystem metadata, file metadata).
	int size=sizeof(fat->block_size);
	mini_fat_write_in_block(fat, 0, 0, size, &fat->block_size);

	mini_fat_write_in_block(fat, 0, size, size, &fat->block_count);
	for ( int i=0; i<fat->block_map.size(); i++) {
		mini_fat_write_in_block(fat, 0, (i+2)*size, size, &fat->block_map[i]);  }
	for ( int i=0; i<fat->files.size(); i++) {
		int sizebuffer=strlen(fat->files[i]->name);
		mini_fat_write_in_block(fat, fat->files[i]->metadata_block_id, 0, size, &fat->files[i]->size);
		mini_fat_write_in_block(fat, fat->files[i]->metadata_block_id, size, size, &sizebuffer);
		mini_fat_write_in_block(fat, fat->files[i]->metadata_block_id, 2*size, sizebuffer, fat->files[i]->name);
		for ( int j=0; j<fat->files[i]->block_ids.size(); j++) {
			mini_fat_write_in_block(fat, fat->files[i]->metadata_block_id, (2+j)*size+sizebuffer,size, &fat->files[i]->block_ids[j]);
		}
	}
	return true;
}

FAT_FILESYSTEM * mini_fat_load(const char *filename) {
	FILE * fat_fd = fopen(filename, "r+");
	if (fat_fd == NULL) {
		perror("Cannot load fat from file");
		exit(-1);
	}
	// TODO: load all metadata (filesystem metadata, file metadata) and create filesystem.
	//mini_fat_read_in_block(fat, 0, 0, sizeof( &fat->block_size), &obtainedsize);
	int block_size = 1024, block_count = 10;
	FAT_FILESYSTEM * fat = mini_fat_create_internal(filename, block_size, block_count);
	int size=4;
	int file_count=0;
	char name[400];
	std::vector<int> metadata_block_ids; // MetaData blocks.
	std::vector<int> block_ids; // Data blocks.
	//mini_fat_read_in_block(fat, 0, 0, size, &fat->block_size);
	fat->block_size=block_size;
	fat->block_count=block_count;
	//mini_fat_read_in_block(fat, 0, size, size, &fat->block_count);
	for ( int a=0; a<block_count; a++) {
		mini_fat_read_in_block(fat, 0, (a+2)*size, size, &fat->block_map[a]);  
		if(fat->block_map[a]==FILE_ENTRY_BLOCK){
			file_count++;
			metadata_block_ids.push_back(a);

		}
	}
	for ( int i=0; i<metadata_block_ids.size(); i++) {
		memset(name, 0, sizeof(name));
		int sizebuffer=0;


		// length of the filename
		mini_fat_read_in_block(fat, metadata_block_ids[i], size, size, &sizebuffer);
		//filename
		mini_fat_read_in_block(fat, metadata_block_ids[i], 2*size, sizebuffer, name);

		FAT_FILE * fatfile=mini_file_create(name);
		fat->files.push_back(fatfile);
		strcpy(fat->files[i]->name,name);
		fatfile->metadata_block_id = metadata_block_ids[i];

		// size		
		mini_fat_read_in_block(fat, metadata_block_ids[i], 0, size, &fat->files[i]->size);


		for ( int j=0; j<(fat->files[i]->size/block_size)+1; j++) {

			int blockid=0;

			mini_fat_read_in_block(fat, metadata_block_ids[i], (2+j)*size+sizebuffer,size, &blockid);
			fat->files[i]->block_ids.push_back(blockid);


		}

	}

	return fat;
}
