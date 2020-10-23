#include "queue.h"
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>

#include "../global/data_structure.h"
#include "../global/network.h"
#include "../global/macro.h"

static int num_of_queued_request = 0;
static request_queue_node* request_head;
static request_queue_node* request_tail;

static int num_of_active_process = 0;
static process_queue_node* process_head;
static process_queue_node* process_tail;

void queue_request(socket_addr* client_info, request* req) {
	/** create new quest node */
	request_queue_node* new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(request_queue_node));
	CHECK_MEM_ALLOCATION(new_node);

	/** populate the new node */
    new_node->client_info = client_info;
	new_node->req = req;
	new_node->next = NULL;



	/** add new node to the linked list */
	if (num_of_queued_request == 0) { /** linked list is empty */
		request_head = new_node;
		request_tail = new_node;
	} else { /** there is something on linked list */
		request_tail->next = new_node;
		request_tail = new_node;
	}

    /** increase the num of item stored in the linked list */
	num_of_queued_request++;


}

request_queue_node* deque_request(){
    request_queue_node* retval = NULL;

   
    /** if there is something on the linked list */
    if(num_of_queued_request > 0){
        retval = request_head;
        request_head = request_head->next;

        /** if the node was last of the list */
        if(request_head == NULL) request_tail = NULL;

        /** decrease the num of item stored in the linked list */
        num_of_queued_request--;
    }


    return retval;
}

void free_request_queue_node(request_queue_node *node){
    
    /** free allocated dynamic resources */
    free(node->client_info);
    free(node->req);
    free(node);
}

void free_request_queue(){
    request_queue_node* tmp;

   while (request_head != NULL){
       tmp = request_head;
       request_head = request_head->next;
       free_request_queue_node(tmp);
       num_of_queued_request--;
    }
}




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
    char e_addr_tmp[20];


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
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the permision */
        perm_tmp[i - ori] = buf[i];
        i++;
    }
    perm_tmp[i - ori] = '\0';


    /** offset */
    char offset_tmp[20];
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the offset */
        offset_tmp[i - ori] = buf[i];
        i++;
    }
    offset_tmp[i - ori] = '\0';

    /** device */
    char dev_tmp[10];
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the device  */
        dev_tmp[i - ori] = buf[i];
        i++;
    }
    dev_tmp[i - ori] = '\0';


  
    /** inode */
    char inode_tmp[30];
    while(buf[i] == '\t' || buf[i] == ' ') i++; /** adjust the index position */
    ori = i;

    while(buf[i] != '\t' && buf[i] != ' '){ /** store the inode */
        inode_tmp[i - ori] = buf[i];
        i++;
    }
    inode_tmp[i - ori] = '\0';

    /** path name */
    char pathname_tmp[PATH_MAX];
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

unsigned long mem_usage(int pid){
    /** open the proc/[pid]/maps file */
    char maps_path[500];
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


process_queue_node* get_process_queue_head(){
    return process_head;
}

void add_record(int pid, process_records *head){
    /** create new node */
    process_records *new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_records));
    CHECK_MEM_ALLOCATION(new_node);

    /** get the current time and store it */
    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);
    strftime(new_node->time, DEFAULT_TIME_LENGTH, " %Y-%m-%d %H:%M:%S", tm_info);

    /** get the current mem usage */
    new_node->mem_usage = mem_usage(pid);

    /** percentage of memory used by the process */
	struct sysinfo info;
	sysinfo(&info);
    new_node->percentage = ((float)new_node->mem_usage / (float)info.totalram) * 100.0f;


    /** assign the next address */
    new_node->next = NULL;

    if(head->next == NULL){ /** check if record only have 1  */
        head->next = new_node;
    }else{
        /** got to the last node */
        process_records *tmp = head;
        for(; tmp->next != NULL; tmp = tmp->next);
        tmp->next = new_node; /** add new record to the last node */
    }

}

process_records* get_last_record(process_records *head){
    process_records *tmp_record = head;
	for(;tmp_record->next != NULL; tmp_record = tmp_record->next);
    return tmp_record;
}

void print_record(process_records *head){
    process_records *tmp = head;

    for(; tmp->next != NULL; tmp = tmp->next){
        printf("%s -> %lu\n", tmp->time, tmp->mem_usage);
    }
}

process_records* get_last(process_records* head){
    process_records *tmp = head;
    
    /** traverse to the last record */
    for(; tmp->next != NULL; tmp = tmp->next);

    return tmp;
}


void queue_process(int pid, char* arguments){
    /** create new node */
    process_queue_node *new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_queue_node));
    CHECK_MEM_ALLOCATION(new_node);

    /** populate the new node */
    new_node->pid = pid; 
    strcpy(new_node->arguments, arguments);   
    new_node->next = NULL;

    /** initiate the record */
    process_records* record = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_records));
    CHECK_MEM_ALLOCATION(record);

    record->mem_usage = 0;
    record->next = NULL;

    /** get the current time and store it */
    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);
    strftime(record->time, DEFAULT_TIME_LENGTH, " %Y-%m-%d %H:%M:%S", tm_info);
    
    /** add the record to the new node */
    new_node->records = record;
    new_node->record_len = new_node->record_len + 1;


    /** add new node to the linked list */
	if (num_of_active_process == 0) { /** linked list is empty */
		process_head = new_node;
		process_tail = new_node;
	} else { /** there is something on linked list */
		process_tail->next = new_node;
		process_tail = new_node;
	}

    /** increase the num of item stored in the linked list */
	num_of_active_process++;

 
}

process_queue_node* deque_process(){
    process_queue_node* retval = NULL;

    /** if there is something on the linked list */
    if(num_of_active_process > 0){
        retval = process_head;
        process_head = process_head->next;

        /** if the node was last of the list */
        if(process_head == NULL) process_tail = NULL;

        /** decrease the num of item stored in the linked list */
        num_of_active_process--;
    }


    return retval;
}

void update_process_queue(){
    /** update the record */
    if(num_of_active_process > 0){
        process_queue_node *tmp;
        tmp = process_head;
        while(tmp != NULL){
            add_record(tmp->pid, tmp->records);
            tmp->record_len = tmp->record_len + 1;
            tmp = tmp->next;
        }
    }
}


void free_process_record(process_records *head){
    for(;head != NULL; head = head->next){
        if(head == NULL) break;
        free(head);
    }
}

void free_process_queue(){
    process_queue_node* tmp;

    while (process_head != NULL){
       tmp = process_head;
       process_head = process_head->next;
       free_process_record(tmp->records);
       free(tmp);
       num_of_active_process--;
    }

}

