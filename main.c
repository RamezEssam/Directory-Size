#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifdef _WIN32
	#include <sys/stat.h>
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

void format_size(long long bytes) 
{
    if (bytes < 1024LL * 1024LL) 
	{
        printf("%.2f KB\n", bytes / 1024.0);
    } else if(bytes < 1024LL * 1024LL * 1024LL)
	{
        printf("%.2f MB\n", bytes / (1024.0 * 1024.0));
    }else
	{
		printf("%.2f GB\n", bytes / (1024.0 * 1024.0 * 1024.0));
	}
}

int get_file_size(const char *filepath, long long *size)
{
	#ifdef _WIN32
		#define stat_t struct _stat64
		#define stat_fn _stat64
	#else
		#define stat_t struct stat
		#define stat_fn stat
	#endif

	stat_t st;

	if (stat_fn(filepath, &st) == 0) 
	{
		*size = (long long)st.st_size;
		return 1;
	}else
	{
		return 0;
	}
		
}

#ifdef _WIN32
int get_dir_size(const char *path, long long *dirsize, int verbose) 
{
    WIN32_FIND_DATAA data;
    HANDLE hFind;
    char search[MAX_PATH];
    char fullpath[MAX_PATH];
	
	long long file_size = 0;

    // Build search pattern: path\*
    snprintf(search, MAX_PATH, "%s\\*", path);

    hFind = FindFirstFileA(search, &data);
    if (hFind == INVALID_HANDLE_VALUE) 
	{
        return 0;
    }

    do 
	{
        // Skip . and ..
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0)
            continue;

        // Build full path
        snprintf(fullpath, MAX_PATH, "%s\\%s", path, data.cFileName);

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
            // Recurse into subdirectory
            get_dir_size(fullpath, dirsize, verbose);
        } else 
		{
            // File only
			if(!get_file_size(fullpath, &file_size))
			{
				printf("ERROR getting file size: %s", fullpath);
			}else
			{
				if(verbose)
				{
					printf("%s    size: ", fullpath);
					format_size(file_size);
				}

				*dirsize += file_size;
			}


        }

    } while (FindNextFileA(hFind, &data));

    FindClose(hFind);
	
	return 1;
}
#else
int get_dir_size(const char *path, long long *dirsize, int verbose) 
{
    DIR *dir = opendir(path);
    if (!dir) return 0;

    struct dirent *entry;
    char fullpath[4096];
    struct stat st;
	
	long long file_size = 0;

    while ((entry = readdir(dir)) != NULL) 
	{
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (stat(fullpath, &st) != 0)
            continue;

        if (S_ISREG(st.st_mode)) 
		{
			if(!get_file_size(fullpath, &file_size)) 
			{
				printf("ERROR getting file size: %s", fullpath);
			}else
			{
				if(verbose)
				{
					printf("%s    size: ", fullpath);
					format_size(file_size);
				}
				*dirsize += file_size;
			}
			
        } else if (S_ISDIR(st.st_mode)) 
		{
            get_dir_size(fullpath);
        }
    }

    closedir(dir);
	return 1;
}
#endif







int main(int argc, char **argv)
{	
	if(argc < 2)
	{
		printf("Usage: dirsize <directory-path> [-v]\n");
		return 1;
	}
	
	int verbose = 0;
	
	if(argc >= 3)
	{
		if(strcmp(argv[2], "-v") == 0) 
		{
			verbose = 1;
		}else
		{
			printf("Usage: dirsize <directory-path> [-v]\n");
			return 1;
		}
		
	}
	
	const char *dirpath = argv[1];
	long long dirsize;
	if(!get_dir_size(dirpath, &dirsize, verbose))
	{
		printf("ERROR getting directory size\n");
		return 1;
	}
	printf("Total Size: ");
	format_size(dirsize);
	return 0;
}