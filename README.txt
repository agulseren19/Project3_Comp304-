GITHUB LINK: https://github.com/agulseren19/Project3_Comp304-

								REPORT
Our code works completely and we passed all the test cases. 

								FAT_FILESYSTEM 

We added a variable to FAT_FILESYSTEM struct: int fd. We opened a .fat file in mini_fat_create_internal and set the fd there. 

mini_fat_write_in_block
We wrote mini_fat_write_in_block and mini_fat_read_in_block so that we can use them as helpers in save and load. At first, we checked if block offset is less than 0 and more than or equal to block size and size added to block size is larger than block size. If one of these conditions are true, function should return 0. We used lseek to change the cursor position to the place where the write should occur which is given as the input. Block size multiplied by block id additional to block offset shows the place to write. We use write function to write to buffer into file at the given size.  If write is successful,size is added to written and written is returned.

mini_fat_read_in_block
Firstly, we checked if block offset is less than 0, more than or equal to block size and size added to block size is larger than block size. If one of these conditions are true, function should return 0. We used lseek to change the cursor position to the place where the read should occur which is given as the input. Block size multiplied by block id additional to block offset shows the place to read. We use read function to read from file to buffer as much as given “size” bytes. If read is successful, size is added to readd and readd is returned. 

mini_fat_find_empty_block
The empty blocks are shown in the block map as EMPTY_BLOCK(0). Hence, we iterated through the block map and returned the index if EMPTY_BLOCK is reached, else we returned -1.

mini fat create(filename, block size, block count) 
We created file with filename and assigned it to fd in mini_fat_create_internal as explained above. We used ftruncate to set the size to block size multiplied by block count.

mini fat save(fs) 
We used mini_fat_write_in_block here. In order to save a virtual disk to file on real disk, we wrote on fat->fd which was given as input. We recorded the size of block size as size in order to use it for offset. At first, we wrote filesystem metadata to 0th block.We wrote block size with offset 0, block sound with offset size(because block size occupies this much). Because block map is a vector, we iterated through it and wrote each to file with offset which is set according to index. Then we moved on with storing file metadata. For each file in fat, metadata_block_id shows the index where metadata of file is stored. We recorded everything about files to metadata_block_id’th block. We recorded the file size with 0 offset. We wrote length of filename with size offset. We wrote filename with offset 2*size( due to size and length of filename). We wrote each block id (data blocks id) iteratively. 

mini fat load(filename)
We created a new FAT_FILESYSTEM and set its fd to fd of file with this filename. Instead of using fat_fd, we saved fd in fat and used that in read functions. We created variables in order to record the ones we read into these variables. Block size and block count are already given as variables. We set the newly created fat’s block size and count to these. Now, because we know the block count, we can use it to iterate through the block map. We iterated through the block map and read size bytes, set fat->block_map[a] to read value.We saved each FILE_ENTRY_BLOCK’s index to metadata_block_ids. We also incremented file count. We loaded filesystem metadata this way. We can move on with loading file metadata. Now, we know metadata_block_ids. We can iterate through metadata_block_ids. We read 0th offset in metadata_block_ids[a] block and save it to fat->files[i]->size. We read size’th offset and save it into sizebuffer which shows the length of the filename. This is needed in order to know how much we should read to be able to read the filename. Hence now, we read sizebuffer bytes and record it into the filename. We should also set files vector of FAT_FILES in FAT_FILESYSTEM. We create new files using mini_file_create and the readen filename. We push this file to fat->files and we set the files’ name. We set the fat files’ metadata_block_id to current metadata_block_id. Hence, we initialized fat file’s variables. Next we loaded data block ids of files. We used a for loop which iterated size of file/block size times because that showed how many data blocks that file has. All in all, we loaded fat from file and set each variable of fat in order. 


                                                		FILE

mini_file_open(fs, filename, is_write)
Here we first search if a file with the given filename is present in the fs or not. For that we used the given mini_file_find(fs, filename) which return NULL if it does not exist. Also we opened a new FAT_OPEN_FILE to return if everything went successfully. We first started by checking if the return value of mini_file_find is NULL, if it is we created a new File with mini_file_create_file (fs, filename). In the Mini_file_create_file, if there is empty block in the fs, this void returns a new file. Thus we created a new file if there is no file in the fs with the given filename. Else we directly returned NULL. If in the mini_file_find we did not get NULL, meaning there exist the filename in fs, we checked if the given bool is true. If the given bool (is_write) is true, we iterated over the fd->open_handles to check if there is another open file. And if there is we returned NULL, else we set the parameters of OPEN_FILE and added to the vector open_handles and returned it. 


mini_file_write (fs, open_file, size, buffer)
Here we checked many cases. Firstly we returned 0 if the given size parameter is negative. Also we checked the size of the open_file->file to see if it is a newly created file. Then if it is this will be the first time the data of the file will be written thus we tried to allocate new block with mini_fat_allocate_new_block. If there is an empty block in fs, we added the new block id to the file->block_ids vector. 
Then we checked if the given size parameter plus our current position exceeds our current block size or not. 
* If not we calculated to which block we will write with open_file->file->block_ids[open_file->position / block_size] and our offset with open_file->position % block_size. Since we now have block id and offset we directly used mini_fat_write_in_block(fs, block_id, block_offset, size, buffer). Here we also checked if the write command is done to the end of the file or if the write is an override by checking the place of our position cursor. If it is not an override we incremented the size and returned the written_bytes to be the given size parameter. Also we changed our position with mini_file_seek.
* If it exceeds the block size we write to the file one by one with a while loop. In each iteration we checked if we exceed the current block or not with the following statement open_file->position % block_size== 0). If we exceed we tried to allocate block and added to the block_ids. While writing in each iteration with the position cursor we calculated our block_id and block_offset as we did previously and used mini_fat_write_in_block but this time the size parameter was 1. Lastly in each iteration we moved ur position cursor by 1. 


mini_file_read (fs, open_file, size, buffer)
Here we first checked if position + given size parameter exceeds the open_file->file size or not. If it exceeds we set a new possible_read parameter with the file size minus position. Because we can only read until the end of the file. We did something similar to mini_file_write, we read the file one by one (byte). In each iteration we set the position to be position + 1 and at the end we returned the new variable possible_read. 


mini_file_seek(fs, open_file, offset, from_start)
Before seeking and changing the position cursor, we first check many conditions and if one of them gives true we directly returned false.
* If it is from start
   * offset>size: This means that the offset (wanted position to go) exceed our current file size thus it is out of boundary of out file. 
   * offset<0: This also means we are trying to go to a negative location
* If it is not from start
   * Position + offset > size : this means that from our position if we move our cursor by the offset size we will exceed our file size. 
   * Position + offset < 0 : we are trying to go to a negative position. 
If non of the statement returned true, we proceeded with setting the openfile->position with offset if it is from start and position+offset if it not from start. Then lastşy we returned true. 


mini_file_delete(fs,filename)
At the beginning we checked if there exist a file with the given filename in the fs to be deleted. If there is no file with filename we directly returned false and gave an error. Then we checked if the file is open or not by checking its open_handles’ size. We thought that if the vector size is 0 then there is no open file. Thus we first set its block_ids in the block_map to EMPTY_BLOCK. Also we make its metadata_block to be EMPTY_BLOCk in the block map. Lastly we deleted the file from fs->files vector.
