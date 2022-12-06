#define FUSE_USE_VERSION 30

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <fuse.h>

/* The size of each file in the target directory. 
   You can change it. */
#define  SINGLE_FILE_SIZE 10

int  files_count   = 0;
int  written_bytes = 0;

int  curr_file_idx 		   = -1;
int  curr_file_content_idx = -1;

char files_content[10][SINGLE_FILE_SIZE];
char files_list[10][100];
char target_directory[300];


/* Get the name for each file - the current date. */
void get_actual_date(char * target)
{
    time_t t     = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(target, "%d-%02d-%02d--%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}


/* Create file in target directory. */
int create_file(const char * content)
{
    int   fd;
	FILE* fp;
	char  date[100];
	char  file_name[500];

    int sum_bytes       = 0;
	int left_count      = strlen(content);
	int total_count     = strlen(content);
	int processed_bytes = 0;

    get_actual_date(date);
    sprintf(file_name, "%s/%s", target_directory, date);
    fp = fopen(file_name, "w");
    if(fp == NULL) 
        return -1;
    fd = fileno(fp);

    do {
        processed_bytes = write(fd, content, left_count);
        if(processed_bytes == -1)
            return -1;
        
        sum_bytes += processed_bytes;
        left_count -= processed_bytes;
    } while(sum_bytes != total_count);

    close(fd);
}


/* Remove an empty character at the end of the text. */
void remove_last_char(char * string)
{   
    int del_index = strlen(string) - 1;
    memmove(&string[del_index],  &string[del_index + 1], strlen(string) - del_index);
}


/* Add file to list of files (files_list[] structure). */
void add_file(const char *filename)
{
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}


/* Check if the file is in the existing list. */
int is_file( const char *path )
{
	path++; 
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}


/* Get file index from list and return it. */
int get_file_index( const char *path )
{
	path++; 
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}


/* All writeinfile functions call this function. */
void write_to_file(const char *path, const char *new_content, int size)
{
	int file_idx = get_file_index(path);
	if (file_idx == -1) 
		return;

	if(strlen(files_content[file_idx]) + size == SINGLE_FILE_SIZE) {
		strncat(files_content[file_idx], new_content, size);
		create_file(files_content[file_idx]);
		strcpy(files_content[file_idx], "");
	}
	else if(strlen(files_content[file_idx]) + size < SINGLE_FILE_SIZE) {
		strncat(files_content[ file_idx ], new_content, size);
		remove_last_char(files_content[file_idx]);
	}
	else {
		/* strlen + size > SINGLE_FILE_SIZE */
		int left_size = SINGLE_FILE_SIZE - strlen(files_content[file_idx]);
		strncat(files_content[file_idx], new_content, left_size);
		create_file(files_content[file_idx]);
		
		/* There may be an overflow of one file, 
		   since the computer can do everything too quickly and instead of the second file, 
		   everything will be written to the already full one. */
		sleep(2);
		strcpy(files_content[file_idx], "");
		
		int movement = left_size;
		int iter_count = (size-left_size) / SINGLE_FILE_SIZE;
		int afteriter  = (size-left_size) % SINGLE_FILE_SIZE;
		
		if(size - left_size < SINGLE_FILE_SIZE) {
			strncat(files_content[file_idx], new_content + movement, size - left_size);
			remove_last_char(files_content[file_idx]);
			return ;
		}
		for(int i = 0; i < iter_count; i++) {
			strncat(files_content[file_idx], new_content + movement, SINGLE_FILE_SIZE);
			create_file(files_content[file_idx]);
			/* There may be an overflow of one file, 
			   since the computer can do everything too quickly and instead of the second file, 
			   everything will be written to the already full one. */
			sleep(2);
			strcpy(files_content[file_idx], "");
			movement = movement + SINGLE_FILE_SIZE;
		}
		strncat(files_content[file_idx], new_content + movement, afteriter);
		remove_last_char(files_content[file_idx]);
	}
}


/* All getattr functions (e.g ls) call this function. */
static int do_getattr( const char *path, struct stat *st )
{
	st->st_uid   = getuid(); 
	st->st_gid   = getgid(); 
	st->st_atime = time( NULL ); 
	st->st_mtime = time( NULL ); 
	
	if ( strcmp( path, "/" ) == 0)
	{
		st->st_mode  = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
	else if ( is_file( path ) == 1 )
	{
		st->st_mode  = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size  = 1024;
	}
	else
	{
		return -ENOENT;
	}
	
	return 0;
}


/* e.g ls function implementation. */
static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	filler( buffer, ".", NULL, 0 ); 
	filler( buffer, "..", NULL, 0 ); 
	
	if ( strcmp( path, "/" ) == 0 ) 
	{	
		for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
			filler( buffer, files_list[ curr_idx ], NULL, 0 );
	}
	
	return 0;
}


/* All read functions call this function. */
static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	int file_idx  = get_file_index(path);
	if ( file_idx == -1 )
		return -1;
	
	char* content = files_content[ file_idx ];
	memcpy( buffer, content + offset, size );
	return strlen( content ) - offset;
}


/* Create file. */
static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
    if(files_count != 0) 
        return -1;
    files_count++;

	path++;
	add_file( path );
	return 0;
}


/* All writeinfile functions call this function. */
static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	write_to_file(path, buffer, size);
    printf("Size - %ld :: file content - %s\n", size, files_content[0]);
	
	return size;
}


/* Setup my functions. */
static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .mknod		= do_mknod,
    .write		= do_write,
};


int main( int argc, char *argv[] )
{
	printf("put target directory\n>>");
	scanf("%s", target_directory);
	
	return fuse_main(argc, argv, &operations, NULL);
}
