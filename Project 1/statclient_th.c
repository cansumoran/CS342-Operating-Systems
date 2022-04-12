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

int main()
{

	mqd_t request_mq; //message queue for sending requests to the server
	mqd_t result_mq; //message queue for receiving results from the server
	struct mq_attr result_attr; 
	struct mq_attr request_attr; 

	int n_result, n_request;
	char *buf_result_ptr, *buf_request_ptr;
	int buflen_request, buflen_result;

	request_mq = mq_open(MQ_REQUEST, O_RDWR | O_CREAT, 0666, NULL);
    result_mq = mq_open(MQ_RESULT, O_RDWR | O_CREAT, 0666, NULL);

	if (request_mq == -1) {
		perror("can not create request message queue\n");
		exit(1);
	} 
    if (result_mq == -1) {
		perror("can not create result message queue\n");
		exit(1);
	}

	printf("request message queue created, mq id = %d\n", (int) request_mq);
	printf("result message queue created, mq id = %d\n", (int) result_mq);

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
		//get the input from the user
		char input[100]; 
   		scanf(" %[^\n]%*c", input);
		printf("request: %s \n", input);
		if(strcmp(input, "exit") == 0) {
			n_request = mq_send(request_mq, (char *) input, strlen(input), 0);
			if (n_request == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
		printf("mq_send success, item size = %d\n",(int) strlen(input));
			printf("exiting");
			break;
		}
		//send the input to the server
		n_request = mq_send(request_mq, (char *) input, strlen(input), 0);
		if (n_request == -1) {
			perror("mq_send failed\n");
			exit(1);
		}
		printf("mq_send success, item size = %d\n",(int) strlen(input));

		//wait for the response from the server
		n_result = mq_receive(result_mq, (char *) buf_result_ptr, buflen_result, NULL);
		if (n_result == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}
		char* result = buf_result_ptr;
		char delim_space[] = "*";
		char *ptr_space = strtok(result, delim_space);
		int file_number;
		sscanf(ptr_space, "%d", &file_number);	
		printf("file number: %d\n", file_number);	

		//first results from first file
		ptr_space = strtok(NULL, delim_space);
		char delim[] = "-";
		char *ptr = strtok(ptr_space, delim);
		printf("received item from file %d -> = %s\n", 1, ptr);

		if(file_number > 1) {
			for(int i = 1; i < file_number; i++) {
				n_result = mq_receive(result_mq, (char *) buf_result_ptr, buflen_result, NULL);
				if (n_result == -1) {
					perror("mq_receive failed\n");
					exit(1);
				}
				char* other_results = buf_result_ptr;
				char *other_ptr = strtok(other_results, delim_space);
				other_ptr = strtok(NULL, delim_space);
				char *ptr_res = strtok(other_ptr, delim);
				printf("received item from file %d -> = %s\n", i + 1, ptr_res);
			}
		}		
		printf("\n");
	}
	printf("good bye");
	mq_close(result_mq);
	mq_close(request_mq);
	mq_unlink(MQ_RESULT);
	mq_unlink(MQ_REQUEST);
	free(buf_result_ptr);
	free(buf_request_ptr);
	return 0;
}
