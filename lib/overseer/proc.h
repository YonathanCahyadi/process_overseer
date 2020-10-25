#ifndef LIB_OVERSEER_PROC_H_
#define LIB_OVERSEER_PROC_H_

#define LINE_MAX_LENGTH  (PATH_MAX + 100)
#define DEFAULT_PROC_MAPS_PATH "/proc/%d/maps"
#define DEAFULT_HEX_BASE 16

unsigned long mem_usage(int pid);

float mem_usage_percentage(unsigned long mem_usage);

#endif /** LIB_OVERSEER_PROC_H_ */