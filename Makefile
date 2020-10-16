COMPILER=gcc
FLAGS=-Wall -g
CURRENT_DIR = $(shell pwd)
CONTROLER_LIB_PATH=$(CURRENT_DIR)/lib/controler/utility.h $(CURRENT_DIR)/lib/controler/utility.c $(CURRENT_DIR)/lib/global/data_structure.h $(CURRENT_DIR)/lib/global/data_structure.c  $(CURRENT_DIR)/lib/global/macro.h $(CURRENT_DIR)/lib/global/macro.c  $(CURRENT_DIR)/lib/global/network.h $(CURRENT_DIR)/lib/global/network.c
CONTORLER_SRC_FILE=$(CURRENT_DIR)/controler.c
CONTORLER_OUT_FILE=$(CURRENT_DIR)/controler.out
OVERSEER_LIB_PATH=$(CURRENT_DIR)/overseer.h $(CURRENT_DIR)/lib/overseer/utility.h $(CURRENT_DIR)/lib/overseer/utility.c $(CURRENT_DIR)/lib/overseer/queue.h $(CURRENT_DIR)/lib/overseer/queue.c $(CURRENT_DIR)/lib/overseer/logging.h $(CURRENT_DIR)/lib/overseer/logging.c $(CURRENT_DIR)/lib/global/data_structure.h $(CURRENT_DIR)/lib/global/data_structure.c  $(CURRENT_DIR)/lib/global/macro.h $(CURRENT_DIR)/lib/global/macro.c  $(CURRENT_DIR)/lib/global/network.h $(CURRENT_DIR)/lib/global/network.c 
OVERSEER_SRC_FILE=$(CURRENT_DIR)/overseer.c
OVERSEER_OUT_FILE=$(CURRENT_DIR)/overseer.out

all: controler overseer

controler: $(CONTORLER_SRC_FILE) $(CONTROLER_LIB_PATH)
	$(COMPILER) $(FLAGS) -o $(CONTORLER_OUT_FILE) $(CONTORLER_SRC_FILE) $(CONTROLER_LIB_PATH)

overseer: $(OVERSEER_SRC_FILE) $(OVERSEER_LIB_PATH)
	$(COMPILER) $(FLAGS) -pthread -o $(OVERSEER_OUT_FILE) $(OVERSEER_SRC_FILE) $(OVERSEER_LIB_PATH)
