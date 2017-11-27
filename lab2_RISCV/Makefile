RV_SIM: RV_SIM.c
	gcc RV_SIM.c -Ofast -lm -o RV_SIM

.PHONY:clean
clean:
	rm -f RV_SIM

.PHONY:debug_memory
debug_memory:
	gcc RV_SIM.c -DDEBUG_MEMORY -Ofast -lm -o RV_SIM

.PHONY:debug_register
debug_register:
	gcc RV_SIM.c -DDEBUG_REGISTER -Ofast -lm -o RV_SIM

.PHONY:debug_content
debug_content:
	gcc RV_SIM.c -DDEBUG_CONTENT -Ofast -lm -o RV_SIM

.PHONY:debug
debug:
	gcc RV_SIM.c -DDEBUG -Ofast -lm -o RV_SIM 
