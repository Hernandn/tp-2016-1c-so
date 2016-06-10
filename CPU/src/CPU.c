/*
 ============================================================================
 Name        : CPU.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"

void sig_handler(int signo)
{
    if (signo == SIGUSR1){
    	end_signal_received = 1;
    }
}

int main(void){

	//inicio handler de SIGNAL SIGUSR1 para desconectar el CPU
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
	        printf("\ncan't catch SIGUSR1\n");

	Configuration* config = configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	arg_struct args;
	args.config=config;

	iniciarEjecucionCPU(&args);

	return 0;
}

