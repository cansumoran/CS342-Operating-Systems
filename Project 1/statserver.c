#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <mqueue.h> 

const char* MQ_REQUEST = "/REQUEST";
const char* MQ_RESULT = "/RESULT";
           
int main(int argc, char** argv)
{
	//get the file number from the arguments
	int file_number = 0;
	if(argc > 2) {
		file_number = atoi(argv[1]);
		printf("file number: %d \n", file_number);
    }
    else {
        printf("No arguments \n");
		exit(0); 
    }          

	mqd_t request_mq; //message queue for receiving requests from the client
	mqd_t result_mq; //message queue for sending results to the client
	int n_request, n_result;

	struct mq_attr result_attr; 
	struct mq_attr request_attr; 

	char *buf_result_ptr, *buf_request_ptr;
	int buflen_request, buflen_result;

    result_mq = mq_open(MQ_RESULT, O_RDWR);    
	request_mq = mq_open(MQ_REQUEST, O_RDWR);

	if (request_mq == -1) {
		perror("can not open request message queue\n");
		exit(1);
	}
    if (result_mq == -1) {
		perror("can not open result message queue\n");
		exit(1);
	}
	printf("mq opened, mq id = %d\n", (int) result_mq);

	mq_getattr(result_mq, &result_attr);
	printf("result_mq maximum msgsize = %d\n", (int) result_attr.mq_msgsize);

	mq_getattr(request_mq, &request_attr);
	printf("request_mq maximum msgsize = %d\n", (int) request_attr.mq_msgsize);

	/* allocate large enough space for the buffer to store 
     an incoming message */
    buflen_result = result_attr.mq_msgsize;
	buf_result_ptr = (char *) malloc(buflen_result);
	buflen_request = request_attr.mq_msgsize;
	buf_request_ptr = (char *) malloc(buflen_request);
	
	while (1) {

		//receiving message from the client
		n_request = mq_receive(request_mq, (char *) buf_request_ptr, buflen_request, NULL);
		printf("buffer length: %d \n", n_request);
		if (n_request == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}
		printf("mq_receive success, message size = %d\n", n_request);
		printf("received item->id = %s\n", buf_request_ptr);

		int request_no = 0;
		if(buf_request_ptr[0] == 'a') {
			request_no = 1;
		}
		else if (buf_request_ptr[0] == 'r') {
			request_no = 2;
		}
		else if(buf_request_ptr[0] == 'e') {
			break;
		}
		int k = 0;

		//create child processes for each file
		for(int child = 0; child < file_number; child++) {
			k = 0;
			pid_t pid; 
			int fd[2];
			if (pipe(fd) == -1) { 
				printf("Pipe failed \n"); 
				return 1;
			}
			pid = fork();
			if (pid == 0) { 
				//start clock
				clock_t t;
  				t = clock();

				printf("child with id %d started \n", getpid());
				close(fd[0]);

				//get the file name
				char* file_name = argv[2 + child];

				//open the file
				FILE* fp = fopen (file_name, "r");
				int size = 0;
				while(!feof(fp))
				{
					char ch = fgetc(fp);
					if(ch == '\n')
					{
						size++;
					}
				}
				//size++;
				fclose(fp);
				FILE* file = fopen (file_name, "r");
				int file_content[size];
				int index = 0;
				fscanf (file, "%d", &file_content[index]); 
				index++;
				while (!feof (file))
				{  
					fscanf (file, "%d", &file_content[index]);      
					index++;
				}
				fclose (file); 

				//parse the request
				char request[100];
				for(int i = 0; i < n_request; i++) {
					request[i] = buf_request_ptr[i];
				}
				request[n_request] = '\0';
				
				char delim[] = " ";
				char *ptr = strtok(request, delim);

				int start = 0;
				int end = 0;

				//check the request
				if(strcmp(ptr, "count") == 0) {
					//the request can either be count or count <start> <end>
					ptr = strtok(NULL, delim);
					if(ptr == NULL)
					{
						//the request is "count"
						printf("request: count \n");
						int count = 0;
						for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
							count++;
						}
						printf("count is %d \n", count);
						printf("writing to pipe: \n");
						write(fd[1], &count, sizeof(int));
						close(fd[1]);
						printf("writing done\n");
					}
					else {
						//the request is "count <start> <end>"
						printf("request: count <start> <end> \n");
						sscanf(ptr, "%d", &start);
						ptr = strtok(NULL, delim);
						sscanf(ptr, "%d", &end);
						printf("start: %d end: %d \n", start, end);
						int count = 0;
						for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
							if(file_content[i] >= start && file_content[i] <= end)
								count++;
						}
						printf("count is %d \n", count);
						printf("writing to pipe: \n");
						write(fd[1], &count, sizeof(int));
						close(fd[1]);
						printf("writing done\n");
					}
				}
				else if(strcmp(ptr, "max") == 0) {
					//the request is max 
					printf("request: max \n");
					int max = 0;
					for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
							if(file_content[i] >= max)
								max = file_content[i];
					}
					printf("max is %d \n", max);
					printf("writing to pipe: \n");
					write(fd[1], &max, sizeof(int));
					close(fd[1]);
					printf("writing done\n");
				}
				else if(strcmp(ptr, "avg") == 0) {
					//the request can either be avg or avg <start> <end>
					ptr = strtok(NULL, delim);
					if(ptr == NULL)
					{
						//the request is "avg"
						printf("request: avg \n");
						int sum = 0; 
						int count = 0;
						double average = 0;
						printf("file len: %d \n",(int)( sizeof(file_content) / sizeof(file_content[0])));
						for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
							sum += file_content[i];
							count++;
						}
						if(count == 0) {
							average = 0;
						}
						else {
							average = (double) sum / count;
						}
						printf("average is %f \n", average);
						printf("writing to pipe: \n");
						write(fd[1], &average, sizeof(double));
						close(fd[1]);
						printf("writing done\n");
					}
					else {
						//the request is "avg <start> <end>"
						printf("request: avg <start> <end> \n");
						sscanf(ptr, "%d", &start);
						ptr = strtok(NULL, delim);
						sscanf(ptr, "%d", &end);
						printf("start: %d end: %d \n", start, end);
						int sum = 0; 
						int count = 0;
						double average = 0; 
						for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
							if(file_content[i] >= start && file_content[i] <= end) {
								sum += file_content[i];
								count++;
							}
						}
						if(count == 0) {
							average = 0;
						}
						else {
							average = (double) sum / count;
						}
						printf("average is %f \n", average);
						printf("writing to pipe: \n");
						write(fd[1], &average, sizeof(double));
						close(fd[1]);
						printf("writing done\n");
					}
				}
				else if(strcmp(ptr, "range") == 0) {
					//the request is "range <start> <end> <K>"
					printf("request: range <start> <end> <K> \n");
					ptr = strtok(NULL, delim);
					sscanf(ptr, "%d", &start);
					ptr = strtok(NULL, delim);
					sscanf(ptr, "%d", &end);
					ptr = strtok(NULL, delim);
					sscanf(ptr, "%d", &k);
					printf("start: %d end: %d k: %d\n", start, end, k);
					//Sort the array in ascending order
					int length = (int)( sizeof(file_content) / sizeof(file_content[0]));
					for (int i = 0; i < length; i++) {     
						for (int j = i+1; j < length; j++) {     
							if(file_content[i] > file_content[j]) {    
								int temp = file_content[i];    
								file_content[i] = file_content[j];    
								file_content[j] = temp;    
							}     
						}     
					}

					int k_elements[k];
					int ending_index= length - 1;
					int starting_index = 0;
					int found_ending = 0;
					int first_item = file_content[starting_index];
					if(first_item > end) {
						//no item can be found in range!
						int wrong = -1;
						printf("no item can be found in the range!");
						printf("writing to pipe: \n");
						if(write(fd[1], &wrong, sizeof(int)) > 0) {
							printf("writing done\n");
						}
					}
					else {
						for(int i = length - 1; i >= 0; i--) {
							int cur = file_content[i];
							if(cur > end) {
								continue;
							}
							else if (found_ending == 0){
								ending_index = i;
								found_ending = 1;
							}
							if(cur < start) {
								starting_index = i + 1;
								break;
							}
						}

						int k_count = 0;
						for(int i = ending_index; i >= starting_index; i--) {
							if(k_count >= k) {
								break;
							}
							else {
								k_elements[k_count] = file_content[i];
								k_count ++;
							}
						}
						for(int i = k_count -1; i >= 0; i--) {
							int value = k_elements[i];
							printf("item value: %d \n",value);
							printf("writing to pipe: %d\n", value);
							if(write(fd[1], &value, sizeof(int)) > 0) {
								printf("writing done\n");
							}
						}
						
					}
					close(fd[1]);
				}
				else {
					printf("invalid request \n"); 
				}
				 
				//end clock
				t = clock() - t;
   				double elapsed_time = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
   				printf("The child process id %d took %f seconds \n", getpid(), elapsed_time);
				
				printf ("exiting, myid=%d\n", getpid());
				exit(0);
			}
			//parent
			else {
				printf("back to parent \n");
    			close(fd[1]);
				
				if(request_no == 0) { //int result
					int result;
					//result cannot be longer than 10 digits since it is an int
					//but we put 100 as max for safety
					char output[100]; 
					sprintf(output, "%d*", file_number);
					while (read(fd[0], &result, sizeof(int)) > 0)
	    			{
            			printf ("Pipe received: %d \n", result);
	    			}
					char temp_res[100]; 
					sprintf(temp_res, "%d", result);
					strcat(output, temp_res);
					strcat(output, "-1");
					n_result = mq_send(result_mq, (char *) output, strlen(output), 0);
					if (n_result == -1) {
						perror("mq_send failed\n");
						exit(1);
					}
					printf("sending data %s \n", output);
					printf("mq_send success, item size = %d\n",(int) strlen(output)); 
				}
				else if(request_no ==1) { //double result
					double result;
					char output[100]; 
					sprintf(output, "%d*", file_number);
					while (read(fd[0], &result, sizeof(double)) > 0)
	    			{
            			printf ("Pipe received: %f \n", result);
					}
					char temp_res[100]; 
					sprintf(temp_res, "%f", result);
					strcat(output, temp_res);
					strcat(output, "-1");	
					n_result = mq_send(result_mq, (char *) output, strlen(output), 0);
					if (n_result == -1) {
						perror("mq_send failed\n");
						exit(1);
					}
					printf("sending data %s \n", output);
					printf("mq_send success, item size = %d\n",(int) strlen(output)); 			
				}
				else { //array result
					int result;
					char output[result_attr.mq_msgsize]; //max size of posix message
					sprintf(output, "%d*", file_number);
					while (read(fd[0], &result, sizeof(int)) > 0)
	    			{
            			printf ("Pipe received:  %d\n ", result);
						char temp[100]; 
						sprintf(temp, "%d ", result);
						printf("%s \n", temp);
						strcat(output, temp);
	    			}
					strcat(output, "-1");
					n_result = mq_send(result_mq, (char *) output, strlen(output), 0);
					if (n_result == -1) {
						perror("mq_send failed\n");
						exit(1);
					}
					printf("sending data %s \n", output);
					printf("mq_send success, item size = %d\n",(int) strlen(output)); 
				}
				printf ("\n"); 
				fflush (stdout); 
				close(fd[0]);
				
			}
		printf("\n");
		}
	}
	free(buf_result_ptr);
	free(buf_request_ptr);
	mq_close(result_mq);
	mq_close(request_mq);
	mq_unlink(MQ_RESULT);
	mq_unlink(MQ_REQUEST);
	return 0;
}