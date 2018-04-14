#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "train.h"


int main (int argc, char *argv[]){
	int nb_train = atoi(argv[1]);
	pthread_t train[nb_train];
	int i=0;
	for(i=0;i<nb_train;i++){
		if (pthread_create(&train[i],NULL,main_train,(void *)i) != 0){
			fprintf(stderr,"Erreur dans la création du train %d\n",i);
			exit(EXIT_FAILURE);
		}
		printf("Train n°%d créé (id : %d) \n",i,train[i]);
	}
	for(i=0;i<nb_train;i++){
		pthread_join(train[i],NULL);
	}
}
