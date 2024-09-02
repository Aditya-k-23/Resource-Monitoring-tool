//Library inclusions
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/utsname.h>
#include<utmp.h>
#include<sys/sysinfo.h>
#include<unistd.h>
#include<sys/resource.h>
#include<string.h>
#include<getopt.h>

//Additional Functions
void cpu_info(){
	printf("Number of Cores: %d \n", get_nprocs());
	double avgload[1];
	getloadavg(avgload, 1);
	printf("CPU Utilization: %.2f %% \n", avgload[0]*100);
}

void cpu_info_graphic(){
	double avgload[1];
	getloadavg(avgload, 1);
	float percent = avgload[0]*100;
	if(percent > 100){
		percent = 100.00;
	}
	int lines = (int)(percent + 1);
	printf("\t||");
	for (int i =0; i<lines; i++){
		printf("|");
	}
	printf("( %.2f %% ) \n", percent);
}

void memory_info(bool graphic, float *prev_mem, float *curr_mem, int i){
	struct sysinfo buff;
	float change;
	const double gigabyte = 1024 * 1024 * 1024;
	sysinfo(&buff);
	*curr_mem = (buff.totalram - buff.freeram)/gigabyte;
	if(i == 0){
		*prev_mem = *curr_mem;
	}
	printf("%.2f / %.2f GB -- ", (buff.totalram - buff.freeram)/gigabyte, buff.totalram/gigabyte); //Physical used/total
	printf("%.2f / %.2f GB", ((buff.totalram + buff.totalswap) - (buff.freeram + buff.freeswap))/gigabyte, (buff.totalram + buff.totalswap)/gigabyte);//Virtual used/total
	if(graphic == true){
		printf("   |");
		if((*prev_mem - *curr_mem) == 0){
			change = 0.0;
			printf("o");
		}
		if(*prev_mem > *curr_mem){
			change = (*prev_mem - *curr_mem);
			int k = (int)(change * 100);
			for(int e =0; e<k; e++){
				printf(":");
			}
		}
		if(*prev_mem < *curr_mem){
			change = (*curr_mem - *prev_mem);
			int k = (int)(change * 100);
			for(int e =0; e<k; e++){
				printf("#");
			}
		}
		printf("* %.2f ( %.2f )", change, *curr_mem);
	}
	*prev_mem = *curr_mem;
	printf("\n");
}

void self_usage(){
	struct rusage use;
	int who = RUSAGE_SELF;
	getrusage(who, &use);
	printf("Memory Usage: %ld Kb \n",use.ru_maxrss);
}

void system_info(){
	struct utsname buff;
	uname(&buff);
	printf("### System Information ###\n");
	printf("System Name = %s \n", buff.sysname);
	printf("Machine Name = %s \n", buff.nodename);
	printf("Version = %s \n", buff.version);
	printf("Release = %s \n", buff.release);
	printf("Architecture = %s \n", buff.machine);
}

void sessions_info(){
	struct utmp *buff;
	setutent();
	buff = getutent();
	printf("### Sessions/Users ###\n");
	while(buff!=NULL){
		if(buff->ut_type == USER_PROCESS || buff->ut_type == INIT_PROCESS){
			printf("%s\t", buff->ut_user);
			printf("%s\t", buff->ut_line);
			printf(" (");
			printf("%s", buff->ut_host);
			printf(")\n");
		}
		buff = getutent();
	}

}

void section_divide(){
	printf("-------------------------------\n");
}

//Main Function
int main(int argc, char* argv[]){
	//Variable declarations
	int N = 10;
	int t = 1;
	float *prev_mem = malloc(sizeof(float *));
	float *curr_mem = malloc(sizeof(float *));
	bool graphic = false;
	
	//Flag options
	static struct option long_options[] = {
		{"user", 0, .val = 'u'},
		{"system", 0, .val = 'm'},
		{"tdelay", 1, .val = 't'},
		{"graphics", 0, .val = 'g'},
		{"samples", 1, .val = 's'}
	};
	for(;;){
		int opt = getopt_long(argc, argv, "t:s:mug", long_options, NULL);
		if(opt == -1)
			break;
		switch(opt){
		case 'u':
			sessions_info();
			return 0;
			break;
		case 't':
			t = atoi(optarg);
		case 's':
			N = atoi(optarg);
		case 'g':
			graphic = true;
			break;
		case 'm':
			printf("Number of samples: %d -- every %d secs\n", N, t);
			section_divide();
			printf("### Memory ### (Physical Used/Tot -- Virtual Used/Tot)\n");
			for(int i=0; i<N; i++){
				memory_info(graphic, prev_mem, curr_mem, i);
				printf("\e7");
				for(int j = 0; j<N-i-1; j++){
					printf("\n");
				}
				section_divide();
				system_info();
				sleep(t);
				if(i == N-1){
					break;
				}
				printf("\e[J");
				printf("\e8");
			}
			return 0;
			break;
		}
	}

	//Report current sampling specification
	printf("Number of samples: %d -- every %d secs\n", N, t);
	section_divide();

	//Report self utlization of memory
	self_usage();
	section_divide();

	//Report memory utilization stats
	printf("### Memory ### (Physical Used/Tot -- Virtual Used/Tot)\n");
	for(int i=0; i<N; i++){
		memory_info(graphic, prev_mem, curr_mem, i);

		printf("\e7"); //Save cursor position
		for(int j = 0; j<N-i-1; j++){
			printf("\n");
		}
		section_divide();

		//Report User information
		sessions_info();
		section_divide();

		//Report CPU Utilization information in non-graphical format
		cpu_info();
		//Extre space in case of graphical output requirement
		if(graphic == true){
			if(i == N-1){
				printf("\e7");
			}
			for(int j = 0; j<N; j++){
				printf("\n");
			}
		}
		section_divide();

		//Report System Information stats
		system_info();

		//Introduce time delay
		sleep(t);
		if(i == N-1 && graphic == false){
			break;
		}
		printf("\e[J"); //Erase from cursor position
		printf("\e8"); //Return to saved cursor position
	}
	//Report CPU Utilization samples in case of graphical output requirement
	if(graphic == true){
		for(int i=0; i<N; i++){
			cpu_info_graphic();
			printf("\e7");
			for(int j =0; j< N-i-1; j++){
				printf("\n");
			}
			section_divide();
			system_info();
			sleep(t);
			if(i == N-1){
				break;
			}
			printf("\e]J");
			printf("\e8");
			}
	}
	//Some parts of the above code are repeated to create an illusion effect using ESCodes 
	section_divide();
	free(prev_mem);
	free(curr_mem);
	return  0;
}


