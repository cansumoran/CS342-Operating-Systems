#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


struct burst {
    int burst_no;
    int arrival_time;
    int remaining_time;
    int running_time;
    //finished = 0 -> burst not finished yet
    //finished = -1 -> burst not started yet
    //finished = 1 -> burst finished 
    int finished; 
};

int is_done(struct burst* burst, int count){
	for (int i = 0; i < count; i++)
		if (burst[i].finished != 1)
			return 0;
	//printf("All done.\n");
	return 1;
}

void print_burst(struct burst burst) {
    printf("Burst no: %d Arrival time: %d Remaining time: %d Running time: %d \n", burst.burst_no, burst.arrival_time, burst.remaining_time, burst.running_time);
}

int main(int argc, char** argv)
{
    //get the input file and time quantum 
	char input_file[100];
	int quantum = 0;
	if(argc > 1) {
        	strcpy(input_file, argv[1]);
        	if (argc > 2){
        		quantum = atoi(argv[2]);
        	}
        	else
        		printf("No quantum.\n");
    	}
    	else {
        	printf("No arguments \n");
		exit(0); 
    	}   
    
    	struct burst bursts_fcfs[1000];
    	struct burst bursts_sjf[1000];
    	struct burst bursts_srjf[1000];
    	struct burst bursts_rr[1000];
    	
    	int avg_turnaround_fcfs = 0;
    	int avg_turnaround_sjf = 0;
    	int avg_turnaround_srjf = 0;
    	int avg_turnaround_rr = 0;
    	
    	
    	//read the input file
    	FILE* file = fopen (input_file, "r");
    	
    	int burst_no = 0;
    	int arrival_time = 0;
    	int burst_length = 0;
    	int count = 0;
    	int shortest_burst_index;
    	int shortest_burst_len;
    	int all_done;
    	int time;	
    	int turnaround;
    	
    	while(fscanf(file,"%d %d %d", &burst_no, &arrival_time, &burst_length)>0) {
    		
    		// fcfs
        	bursts_fcfs[count].burst_no = burst_no;
        	bursts_fcfs[count].arrival_time = arrival_time;
        	bursts_fcfs[count].remaining_time = burst_length;
        	bursts_fcfs[count].running_time = 0;
        	bursts_fcfs[count].finished = -1;
        	
        	// sjf
        	bursts_sjf[count].burst_no = burst_no;
        	bursts_sjf[count].arrival_time = arrival_time;
        	bursts_sjf[count].remaining_time = burst_length;
        	bursts_sjf[count].running_time = 0;
        	bursts_sjf[count].finished = -1;
        	
        	// srjf
        	bursts_srjf[count].burst_no = burst_no;
        	bursts_srjf[count].arrival_time = arrival_time;
        	bursts_srjf[count].remaining_time = burst_length;
        	bursts_srjf[count].running_time = 0;
        	bursts_srjf[count].finished = -1;
        	
        	// rr
        	bursts_rr[count].burst_no = burst_no;
        	bursts_rr[count].arrival_time = arrival_time;
        	bursts_rr[count].remaining_time = burst_length;
        	bursts_rr[count].running_time = 0;
        	bursts_rr[count].finished = -1;
        	
        	count++;
    	}
   	fclose(file);
    
    //first come first serve (fcfs)
    all_done = is_done(bursts_fcfs, count);
    time = 0;	// total time passed
    turnaround = 0;
    
	burst_no = 0;
    
    while(!all_done){	
    	time++;
    
    	if (bursts_fcfs[burst_no].finished == -1)	// not started
    		bursts_fcfs[burst_no].finished = 0;
    	
    	//printf("Time is %d, job %d is running\n", time, bursts_fcfs[burst_no].burst_no);
    	
    	bursts_fcfs[burst_no].remaining_time--;
    	bursts_fcfs[burst_no].running_time++;
    	
    	if (bursts_fcfs[burst_no].remaining_time == 0){	// finished
		    bursts_fcfs[burst_no].finished = 1;
		    turnaround = turnaround + time - bursts_fcfs[burst_no].arrival_time;
			//printf("Turnaround sum after burst %d is %d\n", burst_no, turnaround);
			burst_no++;
		}
    	
    	all_done = is_done(bursts_fcfs, count);
    }
    
    avg_turnaround_fcfs = round(turnaround * 1.0 / count);


    //shortest job first (sjf)
    int bursts_end = 0;
    time = 0;
    int burst_running = 0;
    int running_burst_index = -1;
    while(!bursts_end) {
        //no burst is running currently, you can start a new burst
        if(!burst_running) {
            //printf("no burst is running \n");
            //find which burst to run
            int shortest_burst_index = -1;
            int shortest_burst_len = 4001;
            for(int i = 0; i < count; i ++) {
                if(bursts_sjf[i].finished == -1) { 
                    if(bursts_sjf[i].arrival_time <= time) {
                        //printf("burst %d is ready to run \n", i);
                        //printf("burst %d length: %d \n", i, bursts_sjf[i].remaining_time);
                        if (bursts_sjf[i].remaining_time < shortest_burst_len) {
                            shortest_burst_index = i;
                            shortest_burst_len = bursts_sjf[i].remaining_time;
                            //printf("burst %d is shortest \n", i);
                        }
                    }
                }
            }
            if(shortest_burst_index != -1) {
                burst_running = 1;
                running_burst_index = shortest_burst_index;
                bursts_sjf[running_burst_index].finished = 0;
                bursts_sjf[running_burst_index].running_time = time - bursts_sjf[running_burst_index].arrival_time;
                //printf("running burst %d, time: %d, arrival time: %d, waited for: %d, length: %d \n", running_burst_index, time, bursts_sjf[running_burst_index].arrival_time,bursts_sjf[running_burst_index].running_time, bursts_sjf[running_burst_index].remaining_time);
            }
        }
        else { //there is a burst running
            if(bursts_sjf[running_burst_index].remaining_time > 0) {
                bursts_sjf[running_burst_index].remaining_time--;
                bursts_sjf[running_burst_index].running_time++;
                //printf("LIVE: burst %d, remaining: %d, running for: %d current time: %d \n", running_burst_index, bursts_sjf[running_burst_index].remaining_time, bursts_sjf[running_burst_index].running_time, time);
                //check if this is the last cycle of burst 
                if(bursts_sjf[running_burst_index].remaining_time == 0) {
                    bursts_sjf[running_burst_index].finished = 1;
                    burst_running = 0;
                    //printf("DONE: burst %d, completed in: %d current time: %d \n", running_burst_index, bursts_sjf[running_burst_index].running_time, time);
                    time--;
                    //check if any more bursts are left
                    bursts_end = 1;
                    for(int i = 0; i < count; i++) {
                        if(bursts_sjf[i].finished == -1) {
                            bursts_end = 0;
                            //printf("not done yet! : burst %d \n", i);
                        }
                    }
                }
            }
        }
        time++;
    }
    turnaround = 0;
    for(int i = 0; i < count; i++) {
        //print_burst(bursts_sjf[i]);
        turnaround += bursts_sjf[i].running_time;
    }
    //printf("turnaround time from sjf: %d \n", turnaround);
    avg_turnaround_sjf = round(turnaround * 1.0 / count);

    //shortest remaining job/time first (srjf)

    all_done = is_done(bursts_srjf, count);
    time = 0;	// total time passed
    turnaround = 0;
  
	while(!all_done){
    	time++;
    	
    	shortest_burst_index = -1;
    	shortest_burst_len = 4001;
    	
    	// choose which one to run
    	for (int i = 0; i < count; i++)
		    if(bursts_srjf[i].arrival_time < time) 
			    if (bursts_srjf[i].finished != 1)
		            if (bursts_srjf[i].remaining_time < shortest_burst_len) {
		                shortest_burst_index = i;
		        		shortest_burst_len = bursts_srjf[i].remaining_time;
	            	}
                   
		//printf("Time is %d, job %d is running\n", time, bursts_srjf[shortest_burst_index].burst_no);

	    bursts_srjf[shortest_burst_index].remaining_time--;
		bursts_srjf[shortest_burst_index].running_time++;
	
		if (bursts_srjf[shortest_burst_index].remaining_time == 0){
			bursts_srjf[shortest_burst_index].finished = 1;
			turnaround = turnaround + time - bursts_srjf[shortest_burst_index].arrival_time;
			//printf("Turnaround sum after burst %d is %d\n", shortest_burst_index, turnaround);
		}

    		all_done = is_done(bursts_srjf, count);
    	}
    
    	avg_turnaround_srjf = round(turnaround * 1.0 / count);
    

    //round robin (rr) -- time quantum only used here
    bursts_end = 0;
    time = 0;
    burst_running = 0;
    int current_time_quantum = 0;
    running_burst_index = -1;
    int did_run[count];
    for(int i = 0; i < count; i++) {
        did_run[i] = 0;
    }
    while(!bursts_end) {
        //no burst is running currently, you can start a new burst
        if(!burst_running) {
            current_time_quantum = 0;
            //printf("no burst is running \n");
            //find which burst to run
            for(int i = 0; i < count; i ++) {
                if(bursts_rr[i].finished != 1) {
                    if(did_run[i] == 0) { 
                        if(bursts_rr[i].arrival_time <= time) {
                            //printf("burst %d is ready to run \n", i);
                            //printf("burst %d length: %d \n", i, bursts_rr[i].remaining_time);
                            burst_running = 1;
                            running_burst_index = i;
                            break;
                        }
                    }
                }
            }
        }
        else { //there is a burst running
            if(bursts_rr[running_burst_index].remaining_time > 0 && current_time_quantum < quantum) {
                did_run[running_burst_index] = 1;
                bursts_rr[running_burst_index].remaining_time--;
                bursts_rr[running_burst_index].running_time = time - bursts_rr[running_burst_index].arrival_time;
                current_time_quantum++;
                //printf("LIVE: burst %d, remaining: %d, running for: %d current time: %d \n", running_burst_index, bursts_rr[running_burst_index].remaining_time, bursts_rr[running_burst_index].running_time, time);
                //check if this is the last cycle of burst 
                if(bursts_rr[running_burst_index].remaining_time == 0) {
                    bursts_rr[running_burst_index].finished = 1;
                    burst_running = 0;
                    //printf("DONE: burst %d, completed in: %d current time: %d \n", running_burst_index, bursts_rr[running_burst_index].running_time, time);
                    time--;
                    current_time_quantum = 0;
                    //check if any more bursts are left
                    bursts_end = 1;
                    for(int i = 0; i < count; i++) {
                        if(bursts_rr[i].finished == -1) {
                            bursts_end = 0;
                            //printf("not done yet! : burst %d \n", i);
                        }
                    }
                    if(bursts_end == 0) {
                        //check for reset
                        int reset = 1;
                        //check if there are more bursts that have not been run yet in this turn
                        for (int i = 0; i < count; i++) {
                            if(i != running_burst_index && bursts_rr[i].finished != 1 && bursts_rr[i].arrival_time <= time) {
                                if(did_run[i] == 0){
                                    reset = 0;
                                }
                            }
                        }
                        if(reset) {
                            for(int i = 0; i < count; i++) {
                                did_run[i] = 0;
                            }
                        }
                    }
                }
            }
            if(current_time_quantum == quantum) {
                burst_running = 0;
                //should we reset the did run for this turn
                int reset = 1;
                //check if there are more bursts that have not been run yet in this turn
                for (int i = 0; i < count; i++) {
                    if(i != running_burst_index && bursts_rr[i].finished != 1 && bursts_rr[i].arrival_time <= time) {
                        if(did_run[i] == 0){
                            reset = 0;
                        }
                    }
                }
                if(reset) {
                    for(int i = 0; i < count; i++) {
                        did_run[i] = 0;
                    }
                }
                current_time_quantum = 0;
                time--;
            }
        }
        time++;
    }

    turnaround = 0;
    for(int i = 0; i < count; i++) {
        //print_burst(bursts_rr[i]);
        turnaround += bursts_rr[i].running_time;
    }
    //printf("turnaround time from round robin: %d \n", turnaround);
    avg_turnaround_rr = round(turnaround * 1.0 / count);

    printf("FCFS\t%d\nSJF\t%d\nSRJF\t%d\nRR\t%d\n", 
	avg_turnaround_fcfs, avg_turnaround_sjf, avg_turnaround_srjf, avg_turnaround_rr);
    
}

