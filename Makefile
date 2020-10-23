CC=gcc
FLAGS=-Wall -g
CURRENT_DIR = .


all: controler.out overseer.out

#--------------------------------------- GLOBAL ---------------------------------------#
GLOBAL_LIB_PATH= $(CURRENT_DIR)/lib/global/
GLOBAL_OBJ= $(GLOBAL_LIB_PATH)data_structure.o $(GLOBAL_LIB_PATH)network.o $(GLOBAL_LIB_PATH)macro.o 

$(GLOBAL_LIB_PATH)macro.o: $(GLOBAL_LIB_PATH)macro.h $(GLOBAL_LIB_PATH)macro.c
	$(CC) -c $(GLOBAL_LIB_PATH)macro.c -o $@

$(GLOBAL_LIB_PATH)data_structure.o: $(GLOBAL_LIB_PATH)data_structure.h $(GLOBAL_LIB_PATH)data_structure.c
	$(CC) -c $(GLOBAL_LIB_PATH)data_structure.c -o $@

$(GLOBAL_LIB_PATH)network.o: $(GLOBAL_LIB_PATH)network.h $(GLOBAL_LIB_PATH)network.c
	$(CC) -c $(GLOBAL_LIB_PATH)network.c -o $@


#--------------------------------------- CONTROLER ---------------------------------------#
CONTROLER_LIB_PATH= $(CURRENT_DIR)/lib/controler/
CONTROLER_OBJ= $(CONTROLER_LIB_PATH)utility.o
CONTROLER_OUT= controler.out
CONTROLER_SRC_OBJ= controler.o
CONTROLER_LIB_DEP= $(GLOBAL_OBJ) $(CONTROLER_OBJ)

$(CONTROLER_LIB_PATH)utility.o: $(CONTROLER_LIB_PATH)utility.h $(CONTROLER_LIB_PATH)utility.c
	$(CC) -c $(CONTROLER_LIB_PATH)utility.c -o $@

$(CONTROLER_SRC_OBJ): controler.c
	$(CC) -c controler.c -o $@

$(CONTROLER_OUT): $(CONTROLER_SRC_OBJ) $(CONTROLER_LIB_DEP)
	$(CC) -g -Wall -o $@ $(CONTROLER_SRC_OBJ) $(CONTROLER_LIB_DEP)


#--------------------------------------- OVERSEER ---------------------------------------#
OVERSEER_LIB_PATH= $(CURRENT_DIR)/lib/overseer/
OVERSEER_OBJ= $(OVERSEER_LIB_PATH)logging.o $(OVERSEER_LIB_PATH)utility.o $(OVERSEER_LIB_PATH)queue.o $(OVERSEER_LIB_PATH)executor.o $(OVERSEER_LIB_PATH)proc.o
OVERSEER_OUT= overseer.out
OVERSEER_SRC_OBJ= overseer.o
OVERSEER_LIB_DEP= $(GLOBAL_OBJ) $(OVERSEER_OBJ)

$(OVERSEER_LIB_PATH)logging.o: $(OVERSEER_LIB_PATH)logging.h $(OVERSEER_LIB_PATH)logging.c
	$(CC) -c $(OVERSEER_LIB_PATH)logging.c -o $@

$(OVERSEER_LIB_PATH)utility.o: $(OVERSEER_LIB_PATH)utility.h $(OVERSEER_LIB_PATH)utility.c
	$(CC) -c $(OVERSEER_LIB_PATH)utility.c -o $@

$(OVERSEER_LIB_PATH)queue.o: $(OVERSEER_LIB_PATH)queue.h $(OVERSEER_LIB_PATH)queue.c
	$(CC) -c $(OVERSEER_LIB_PATH)queue.c -o $@

$(OVERSEER_LIB_PATH)executor.o: $(OVERSEER_LIB_PATH)executor.h $(OVERSEER_LIB_PATH)executor.c
	$(CC) -c $(OVERSEER_LIB_PATH)executor.c -o $@

$(OVERSEER_LIB_PATH)proc.o: $(OVERSEER_LIB_PATH)proc.h $(OVERSEER_LIB_PATH)proc.c
	$(CC) -c $(OVERSEER_LIB_PATH)proc.c -o $@

$(OVERSEER_SRC_OBJ): overseer.h overseer.c
	$(CC) -c overseer.c -o $@

$(OVERSEER_OUT): $(OVERSEER_SRC_OBJ) $(OVERSEER_LIB_DEP)
	$(CC) -g -Wall -pthread -o $@ $(OVERSEER_SRC_OBJ) $(OVERSEER_LIB_DEP)



#--------------------------------------- OTHER ---------------------------------------#
clean:
	rm -f $(CONTROLER_OUT)
	rm -f $(OVERSEER_OUT)
	rm -f $(CONTROLER_SRC_OBJ)
	rm -f $(OVERSEER_SRC_OBJ)
	rm -f $(GLOBAL_OBJ)
	rm -f $(CONTROLER_OBJ)
	rm -f $(OVERSEER_OBJ)
	