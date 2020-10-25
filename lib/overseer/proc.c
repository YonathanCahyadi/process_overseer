#include "proc.h"
#include <linux/limits.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <string.h>
#include "../global/macro.h"

/**
 * @brief  Split the line given in then buf
 * @note   This function will split the line given in the buff into: 
 *         start of memory address, end of memory address, permission, offset, device, indoe, and path name.
 *         After the spliting done it will store those val in the params given
 * @param  buf: The line to be splited
 * @param  *addr_start: pointer to store start of memory address
 * @param  *end_addr: pointer to store end of memory address
 * @param  permision[8]: pointer to store permission
 * @param  *offset: pointer to store offset
 * @param  dev[10]: pointer to store device
 * @param  *inode: pointer to store inode
 * @param  path_name[PATH_MAX]: pointer to store path name
 * @return None
 */
void spliter(char* buf, 
            unsigned long *addr_start, 
            unsigned long *end_addr, 
            char permision[8], 
            unsigned long *offset, 
            char dev[10], 
            int *inode, 
            char path_name[PATH_MAX]){
    int i = 0;
    int ori = 0;

    /** get the start and end addr */
    char s_addr_tmp[20];
    CLEAR_CHAR_BUFFER(s_addr_tmp, 20);
    char e_addr_tmp[20];
    CLEAR_CHAR_BUFFER(e_addr_tmp, 20);


    /** the start addr */
    while(buf[i] != '-'){ /** store the start addr */
        s_addr_tmp[i - ori] = buf[i];
        i++;
    }
    s_addr_tmp[i] = '\0';


    /** the end addr */
    while(buf[i] == '-') i++; /** adjust the index */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the end addr */
        e_addr_tmp[i - ori] = buf[i];
        i++;
    }
    e_addr_tmp[i - ori] = '\0';


    /** permision */
    char perm_tmp[8];
    CLEAR_CHAR_BUFFER(perm_tmp, 8);
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the permision */
        perm_tmp[i - ori] = buf[i];
        i++;
    }
    perm_tmp[i - ori] = '\0';


    /** offset */
    char offset_tmp[20];
    CLEAR_CHAR_BUFFER(offset_tmp, 20);
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the offset */
        offset_tmp[i - ori] = buf[i];
        i++;
    }
    offset_tmp[i - ori] = '\0';

    /** device */
    char dev_tmp[10];
    CLEAR_CHAR_BUFFER(dev_tmp, 10);
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the device  */
        dev_tmp[i - ori] = buf[i];
        i++;
    }
    dev_tmp[i - ori] = '\0';


  
    /** inode */
    char inode_tmp[30];
    CLEAR_CHAR_BUFFER(inode_tmp, 30);
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the inode */
        inode_tmp[i - ori] = buf[i];
        i++;
    }
    inode_tmp[i - ori] = '\0';

    /** path name */
    char pathname_tmp[PATH_MAX];
    CLEAR_CHAR_BUFFER(pathname_tmp, PATH_MAX);
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the pathname */
        pathname_tmp[i - ori] = buf[i];
        i++;
    }
    pathname_tmp[i - ori] = '\0';

    /** store the val in the given param */
    *addr_start = strtoul(s_addr_tmp, NULL, DEAFULT_HEX_BASE);
    *end_addr = strtoul(e_addr_tmp, NULL, DEAFULT_HEX_BASE);
    strncpy(permision, perm_tmp, 8);
    *offset = strtoul(offset_tmp, NULL, DEAFULT_HEX_BASE);
    strncpy(dev, dev_tmp, 10);
    *inode = atoi(inode_tmp);
    strncpy(path_name, pathname_tmp, PATH_MAX);
}

/**
 * @brief  Calculate the memory usage of the given pid
 * @note   This function will open the /proc/{pid}/maps file and calculate the used memory of that process
 * @param  pid: the process id 
 * @return The total used memory
 */
unsigned long mem_usage(int pid){
    /** open the proc/[pid]/maps file */
    char maps_path[500];
    CLEAR_CHAR_BUFFER(maps_path, 500);
    sprintf(maps_path, DEFAULT_PROC_MAPS_PATH, pid);
    FILE* file = fopen(maps_path, "r");
   
    unsigned long used_mem = 0ul;
    
    if(file){ /** if file exist */
        char buf[LINE_MAX_LENGTH];
        while(!feof(file)){
            fgets(buf, LINE_MAX_LENGTH, file);
            // printf("%s", buf);
            unsigned long addr_start, addr_end, offset;
            int inode;
            char perm[8], dev[10], pathname[PATH_MAX];
            CLEAR_CHAR_BUFFER(perm, 8);
            CLEAR_CHAR_BUFFER(dev, 10);
            CLEAR_CHAR_BUFFER(pathname, PATH_MAX);

            spliter(buf, &addr_start, &addr_end, perm, &offset, dev, &inode, pathname);

            /** calculate the used mem with inode 0 */
            if(inode == 0){
                used_mem += (addr_end - addr_start);
            }
        }
        fclose(file);
    }
    return used_mem;
}

/**
 * @brief  Calculate the precentage of memory usage based on the total available memory
 * @note   This function will give the percentage of used memory based on the available total ram
 * @param  mem_usage: the memory usage
 * @return the percentage
 */
float mem_usage_percentage(unsigned long mem_usage){

    /** percentage of memory used by the process */
	struct sysinfo info;
	sysinfo(&info);

    return ((float)mem_usage / (float)info.totalram) * 100.0f;
}
