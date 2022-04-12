#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <mqueue.h> 
#include <pthread.h>

#define MAXTHREADS  10		/* max number of threads */

const char* MQ_REQUEST = "/REQUEST";
const char* MQ_RESULT = "/RESULT";
           
struct arg {
	int start;			
	int end;			
	int k;
	int operation;
	char* file_name;
	int t_index;		/* the index of the created thread */
	char result[100];
};

static void *calculations(void *arg_ptr)
{
	//start the clock
	clock_t t;
   	t = clock();

	//get the arguments
	int start = ((struct arg *) arg_ptr)->start;
	int end = ((struct arg *) arg_ptr)->end;
	int k = ((struct arg *) arg_ptr)->k;
	int operation = ((struct arg *) arg_ptr)->operation;
	char* file_name = ((struct arg *) arg_ptr)->file_name;
	strcpy(((struct arg *) arg_ptr)->result, "");
	printf("thread %d started\n", ((struct arg *) arg_ptr)->t_index);

	//parse the file 
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

	//perform the given operation
	if(operation == 0) { //count
		printf("request: count \n");
		int count = 0;
		for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
			count++;
		}
		printf("count is %d \n", count);
		//count cannot be longer than 10 digits since it is an int
		//but we put 100 as max for safety
		char output[100];
		strcpy(output, "");
		char temp_res[100]; 
		strcpy(temp_res, "");
		sprintf(temp_res, "%d", count);
		strcat(output, temp_res);
		strcat(output, "-1");
		strcpy(((struct arg *) arg_ptr)->result, output);
	}
	else if(operation == 1) { //count <start> <end>
		printf("request: count <start> <end> \n");
		int count = 0;
		for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
			if(file_content[i] >= start && file_content[i] <= end)
				count++;
		}
		printf("start: %d end: %d \n", start, end);
		printf("count is %d \n", count);
		char output[100]; 
		strcpy(output, "");
		char temp_res[100];
		strcpy(temp_res, "");
		sprintf(temp_res, "%d", count);
		strcat(output, temp_res);
		strcat(output, "-1");
		strcpy(((struct arg *) arg_ptr)->result, output);
	}
	else if(operation == 2) { //max
		printf("request: max \n");
		int max = 0;
		for(int i = 0; i < (int)( sizeof(file_content) / sizeof(file_content[0])); i++) {
			if(file_content[i] >= max)
				max = file_content[i];
		}
		printf("max is %d \n", max);
		char output[100];
		strcpy(output, "");
		char temp_res[100];
		strcpy(temp_res, "");
		sprintf(temp_res, "%d", max);
		strcat(output, temp_res);
		strcat(output, "-1");
		strcpy(((struct arg *) arg_ptr)->result, output);
	}
	else if(operation == 3) { //avg
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
		char output[100];
		strcpy(output, "");
		char temp_res[100];
		strcpy(temp_res, "");
		sprintf(temp_res, "%f", average);
		strcat(output, temp_res);
		strcat(output, "-1");
		strcpy(((struct arg *) arg_ptr)->result, output);
	}
	else if(operation == 4) { //avg <start> <end>
		printf("request: avg <start> <end> \n");
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
		printf("start: %d end: %d \n", start, end);
		printf("average is %f \n", average);
		char output[100];
		strcpy(output, "");
		char temp_res[100];
		strcpy(temp_res, "");
		sprintf(temp_res, "%f", average);
		strcat(output, temp_res);
		strcat(output, "-1");
		strcpy(((struct arg *) arg_ptr)->result, output);
	}
	else if(operation == 5) { //range <start> <end> <k>
		printf("request: range <start> <end> <k> \n");
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
			char output[100];
			strcpy(output, "");
			char temp_res[100];
			strcpy(temp_res, "");
			sprintf(temp_res, "%d", wrong);
			strcat(output, temp_res);
			strcpy(((struct arg *) arg_ptr)->result, output);
			printf("no item can be found in the range! \n");
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
			char output[8100]; //almost the default max size of posix message
			strcpy(output, "");
			for(int i = k_count -1; i >= 0; i--) {
				int value = k_elements[i];
				printf("item value: %d \n",value);
				char temp_res[100];
				sprintf(temp_res, "%d ", value);
				strcat(output, temp_res);
			}
			strcat(output, "-1");
			strcpy(((struct arg *) arg_ptr)->result, output);
		}
	}
	//end clock
	t = clock() - t;
	double elapsed_time = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
   	printf("The thread with id %d took %f seconds \n", ((struct arg *) arg_ptr)->t_index , elapsed_time);
	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	//get the file number from the arguments
	int file_number = 0;
	printf("arg number: %d \n",argc);
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

	pthread_t tids[MAXTHREADS];	/*thread ids*/
	struct arg t_args[MAXTHREADS];	/*thread function arguments*/

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

		if(buf_request_ptr[0] == 'e') {
			break;
		}

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
		int operation = 0;
		int k = 0;

		//check the request
		if(strcmp(ptr, "count") == 0) {
			//the request can either be count or count <start> <end>
			ptr = strtok(NULL, delim);
			if(ptr == NULL)
			{
				operation = 0;
			}
			else {
				//the request is "count <start> <end>"
				operation = 1;
				sscanf(ptr, "%d", &start);
				ptr = strtok(NULL, delim);
				sscanf(ptr, "%d", &end);
			}
		}
		else if(strcmp(ptr, "max") == 0) {
			//the request is max 
			operation = 2;
		}
		else if(strcmp(ptr, "avg") == 0) {
			//the request can either be avg or avg <start> <end>
			ptr = strtok(NULL, delim);
			if(ptr == NULL)
			{
				//the request is "avg"
				operation = 3;
			}
			else {
				//the request is "avg <start> <end>"
				sscanf(ptr, "%d", &start);
				ptr = strtok(NULL, delim);
				sscanf(ptr, "%d", &end);
				operation = 4;
			}
		}
		else if(strcmp(ptr, "range") == 0) {
			//the request is "range <start> <end> <K>"
			ptr = strtok(NULL, delim);
			sscanf(ptr, "%d", &start);
			ptr = strtok(NULL, delim);
			sscanf(ptr, "%d", &end);
			ptr = strtok(NULL, delim);
			sscanf(ptr, "%d", &k);
			operation = 5;
		}

		int ret;
		for(int i = 0; i < file_number; i++) {
			t_args[i].start = start;
			t_args[i].end = end;
			t_args[i].k = k;
			t_args[i].t_index = i;
			t_args[i].operation = operation;
			strcpy(t_args[i].result, "");

			//get the file name
			char* file_name = argv[2 + i];
			t_args[i].file_name = file_name;

			//create threads
			ret = pthread_create(&(tids[i]), NULL, calculations, (void *) &(t_args[i]));
			if (ret != 0) {
				printf("thread create failed \n");
				exit(1);
			}
			printf("thread %i with tid %u created\n", i, (unsigned int) tids[i]);
		}

		//joining the threads
		printf("main: waiting all threads to terminate\n");
		for (int i = 0; i < file_number; ++i) {
			ret = pthread_join(tids[i], NULL);
			if (ret != 0) {
				printf("thread join failed \n");
				exit(0);
			}
		}
		printf("main: all threads terminated\n");

		//sending back the result to client
		for(int i = 0; i < file_number; i++) {
			char temp[result_attr.mq_msgsize]; 
			sprintf(temp, "%d*", file_number);
			char* result = t_args[i].result;
			strcat(temp, result);
			n_result = mq_send(result_mq, (char *) temp, strlen(temp), 0);
			if (n_result == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
			printf("sending data %s \n", temp);
			printf("mq_send success, item size = %d\n",(int) strlen(temp)); 
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