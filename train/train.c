#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "train.h"
#define WAITING_TIME 2
#define MAX_GARAGE 2

/* définition des conditions et des mutex */
pthread_mutex_t mutex_gare = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_voie_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_aiguillage = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_voies = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_garage_tgv = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_garage_gl = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tunnel = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_display = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gare_est;
pthread_cond_t gare_ouest;
pthread_cond_t voie_m_est;
pthread_cond_t aiguillage_m_est;
pthread_cond_t aiguillage_m_ouest;
pthread_cond_t aiguillage_p_est;
pthread_cond_t aiguillage_p_ouest;
pthread_cond_t cond_tunnel;
pthread_cond_t garage_gl_est;
pthread_cond_t garage_gl_ouest;
pthread_cond_t garage_tgv_est;
pthread_cond_t garage_tgv_ouest;
pthread_cond_t voies_est;
pthread_cond_t voies_ouest;

/* définition des variables globales (compteurs)*/
int nb_gare_est=0;
int nb_gare_ouest=0;
int nb_voie_m_est=0;
int nb_aiguillage_m_ouest=0;
int nb_aiguillage_m_est=0;
int nb_aiguillage_p_ouest=0;
int nb_aiguillage_p_est=0;
int nb_garage_tgv_est=0;
int nb_garage_tgv_ouest=0;
int nb_garage_gl_est=0;
int nb_garage_gl_ouest=0;
int nb_tunnel=0;
int nb_voies_est=0;
int nb_voies_ouest=0;


void * main_train(void *arg){
	/* Initialisation des trains :
	La fonction rand() utilise une horloge, il ne faut donc pas que les threads l'accèdent en même temps.
	On attend une durée aléatoire<10s avant de créer un train aux attributs aléatoires.
	*/
	Train train;
	train.id=(int)arg;
	srand(time(NULL));
	int r_sleep = rand()%10;
	sleep(r_sleep);
	int r_sens = rand()%2;
	int r_type = rand()%3;
	train.sens = r_sens;
	train.type = r_type;
	
	/* Implémentation de la priorité : 
	Le thread prend une valeur de priorité différente selon le type du train qu'il représente*/
	if(train.type==TGV)
		pthread_setschedprio(pthread_self(),30);
	if(train.type==GL)
		pthread_setschedprio(pthread_self(),20);
	if(train.type==M)
		pthread_setschedprio(pthread_self(),10);

	/* Traitement des trains pour le sens Est */
	if(train.sens==EST){
		train.pos=POS_EST;
		toString(train);
		gare(train);
		aiguillage_P0(train);
		aiguillage_P1(train);
		tunnel(train);
		voie(train);
	}
	
	/*Trairement des trains pour le sens Ouest*/
	if(train.sens==OUEST){
		train.pos=POS_OUEST;
		toString(train);
		voie(train);
		tunnel(train);
		aiguillage_P1(train);
		aiguillage_P0(train);
		gare(train);
	}
	pthread_exit(0);

}

void toString(Train train){
	pthread_mutex_lock(&mutex_display);
	printf("Train n°%d \t", train.id);
	if(train.sens==EST)
		printf("EST \t");
	else if(train.sens==OUEST)
		printf("OUEST \t");
	if(train.type==M)
		printf("M \t");
	else if(train.type==TGV)
		printf("TGV \t");
	else if(train.type==GL)
		printf("GL \t");
	switch(train.pos){
		case POS_EST:
		printf("EST");
		break;
		case GARE:
		printf("GARE");
		break;
		case AIGUILLAGE:
		printf("AIGUILLAGE");
		break;
		case GARAGE:
		printf("GARAGE");
		break;
		case TUNNEL:
		printf("TUNNEL");
		break;
		case VOIES:
		printf("VOIES");
		break;
		case VOIE_M:
		printf("VOIE MARCHANDE");
		break;
		case POS_OUEST:
		printf("OUEST");
		break;
		default:
		printf("ERROR : NO POSITION FOUND");
		break;
	}
	printf("\n");
	pthread_mutex_unlock(&mutex_display);
}

/* Traitement des trains en gare */
void gare(Train train){

	if (train.type!=M){
		pthread_mutex_lock(&mutex_gare);
		if (train.sens==EST){
			if (nb_gare_est>1)
				pthread_cond_wait(&gare_est, &mutex_gare);
			nb_gare_est++;
			train.pos=GARE;
		}

		if (train.sens==OUEST){
			nb_aiguillage_p_ouest--;
			pthread_cond_signal(&aiguillage_p_ouest);
			train.pos=GARE;
			sleep(WAITING_TIME);
			printf("Le train %d est arrivé à destination ! (Gare ouest) \n", train.id);
			nb_gare_ouest--;
			train.pos=POS_OUEST;
			pthread_cond_signal(&gare_ouest);
		}
		pthread_mutex_unlock(&mutex_gare);
	}

	if (train.type==M){
		if(train.sens==EST){
			pthread_mutex_lock(&mutex_voie_m);
			if(nb_voie_m_est>1)
				pthread_cond_wait(&voie_m_est,&mutex_voie_m);
			nb_voie_m_est++;
			pthread_mutex_unlock(&mutex_voie_m);
		}
		train.pos=VOIE_M;
		if(train.sens==OUEST){
			printf ("Le train %d est arrivé à destination ! (Voie marchande ouest) \n", train.id);
			train.pos=POS_OUEST;
		}
	}

	toString(train);

	sleep(WAITING_TIME);
}

/* Traitement des trains sur les voies */
void voie(Train train){
	pthread_mutex_lock(&mutex_voies);
	pthread_mutex_lock(&mutex_tunnel);
	if (train.sens==OUEST){
		if(nb_voies_est!=0) 
			pthread_cond_wait(&voies_est,&mutex_voies);
		if(nb_tunnel!=0)
			pthread_cond_wait(&cond_tunnel,&mutex_tunnel);
		if(train.type==TGV){
			pthread_mutex_lock(&mutex_garage_tgv);
			if(nb_garage_tgv_est!=0)
				pthread_cond_wait(&garage_tgv_est,&mutex_garage_tgv);
			nb_voies_ouest++;
			nb_tunnel++; 
			nb_garage_tgv_ouest++;
			train.pos=VOIES;
			pthread_mutex_unlock(&mutex_garage_tgv);
		}
		if(train.type==GL){
			pthread_mutex_lock(&mutex_garage_gl);
			if(nb_garage_gl_est!=0)
				pthread_cond_wait(&garage_gl_est,&mutex_garage_gl);
			nb_voies_ouest++;
			nb_tunnel++;
			nb_garage_gl_ouest++;
			train.pos=VOIES;
			pthread_mutex_unlock(&mutex_garage_gl);
		}
		if(train.type==M){
			nb_voies_ouest++;
			nb_tunnel++;
			train.pos=VOIES;
		}
	}
	if(train.sens==EST){
		nb_tunnel--;
		pthread_cond_signal(&cond_tunnel);
		train.pos=VOIES;
		toString(train);
		nb_voies_est--;
		pthread_cond_signal(&voies_est);
		train.pos=POS_EST;
		printf("Le train %d est arrivé en destination de l'Est !\n", train.id);
	}
	toString(train);
	pthread_mutex_unlock(&mutex_tunnel);
	pthread_mutex_unlock(&mutex_voies);
}

/* Fonction de l'aiguillage P0 : on traite les trains selon leurs sens et leurs types 
On vérifie d'abord si une voie de garage peut recevoir le train avant de le faire passer*/
void aiguillage_P0(Train train){
	pthread_mutex_lock(&mutex_aiguillage);
	pthread_mutex_lock(&mutex_gare);
	if(train.sens==EST){
		if(train.type==TGV){
			pthread_mutex_lock(&mutex_garage_tgv);
			if (nb_aiguillage_p_ouest!=0)
				pthread_cond_wait(&aiguillage_p_ouest,&mutex_aiguillage);
			if (nb_garage_tgv_ouest!=0)
				pthread_cond_wait(&garage_tgv_ouest,&mutex_garage_tgv);
			nb_gare_est--;
			pthread_cond_signal(&gare_est);
			nb_aiguillage_p_est++;
			nb_garage_tgv_est++;
			train.pos=AIGUILLAGE;
			pthread_mutex_unlock(&mutex_garage_tgv);
		}

		if(train.type==GL){
			pthread_mutex_lock(&mutex_garage_gl);
			if (nb_aiguillage_p_ouest!=0)
				pthread_cond_wait(&aiguillage_p_ouest,&mutex_aiguillage);
			if (nb_garage_gl_ouest!=0)
				pthread_cond_wait(&garage_gl_ouest,&mutex_garage_gl);
			nb_gare_est--;
			pthread_cond_signal(&gare_est);
			nb_aiguillage_p_est++;
			nb_garage_gl_est++;
			train.pos=AIGUILLAGE;
			pthread_mutex_unlock(&mutex_garage_gl);	
		}
	
		if (train.type==M){
			pthread_mutex_lock(&mutex_voie_m);
			nb_voie_m_est--;
			pthread_cond_signal(&voie_m_est);
			train.pos=AIGUILLAGE;
			pthread_mutex_unlock(&mutex_voie_m);
		}
	}
	if(train.sens==OUEST){
		if(train.type==TGV){
			if(nb_gare_ouest>1)
				pthread_cond_wait(&gare_est,&mutex_gare);
			if(nb_aiguillage_p_est!=0)
				pthread_cond_wait(&aiguillage_p_est,&mutex_aiguillage);
			nb_aiguillage_p_ouest++;
			nb_gare_ouest++;
			nb_garage_tgv_ouest--;
			pthread_cond_signal(&garage_tgv_ouest);
			train.pos=AIGUILLAGE;
		}
		if(train.type==GL){
			if(nb_gare_ouest>1)
				pthread_cond_wait(&gare_est,&mutex_gare);
			if(nb_aiguillage_p_est!=0)
				pthread_cond_wait(&aiguillage_p_est,&mutex_aiguillage);
			nb_aiguillage_p_ouest++;
			nb_gare_ouest++;
			nb_garage_gl_ouest--;
			pthread_cond_signal(&garage_gl_ouest);
			train.pos=AIGUILLAGE;
		
		}
		if(train.type==M){
			if(nb_aiguillage_m_est!=0)
				pthread_cond_wait(&aiguillage_m_est,&mutex_aiguillage);
			train.pos=AIGUILLAGE;
		}
	}
	toString(train);
	pthread_mutex_unlock(&mutex_gare);
	pthread_mutex_unlock(&mutex_aiguillage);
}

/* Fonction de l'aiguillage P1 : on traite les trains selon leurs types, puis selon leurs sens*/
void aiguillage_P1(Train train){
	if(train.type==M){
		if(train.sens==EST){
			pthread_mutex_lock(&mutex_aiguillage);
			nb_aiguillage_m_est--;
			pthread_cond_signal(&aiguillage_m_est);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_aiguillage);
		}

		if(train.sens==OUEST){
			pthread_mutex_lock(&mutex_tunnel);
			nb_tunnel--;
			pthread_cond_signal(&cond_tunnel);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_tunnel);
		}
	}

	if(train.type==GL){
		pthread_mutex_lock(&mutex_garage_gl);
		if(train.sens==EST){
			pthread_mutex_lock(&mutex_aiguillage);
			nb_aiguillage_p_est--;
			pthread_cond_signal(&aiguillage_p_est);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_aiguillage);
		}
		
		if(train.sens==OUEST){
			pthread_mutex_lock(&mutex_tunnel);
			nb_tunnel--;
			pthread_cond_signal(&cond_tunnel);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_tunnel);
		}
		pthread_mutex_unlock(&mutex_garage_gl);
	}
	
	if(train.type==TGV){
		pthread_mutex_lock(&mutex_garage_tgv);	
		if(train.sens==EST){
			pthread_mutex_lock(&mutex_aiguillage);
			nb_aiguillage_p_est--;
			pthread_cond_signal(&aiguillage_p_est);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_aiguillage);
		}		

		if(train.sens==OUEST){
			pthread_mutex_lock(&mutex_tunnel);
			nb_tunnel--;
			pthread_cond_signal(&cond_tunnel);
			train.pos=GARAGE;
			pthread_mutex_unlock(&mutex_tunnel);
		}
		pthread_mutex_unlock(&mutex_garage_tgv);
	}
	toString(train);
}

/* Fonction de traitement du tunnel */
void tunnel(Train train){
	pthread_mutex_lock(&mutex_tunnel);
	if(train.sens==EST){
		pthread_mutex_lock(&mutex_voies);
		if(nb_voies_ouest!=0)
			pthread_cond_wait(&voies_ouest,&mutex_voies);
		if(nb_tunnel!=0)
			pthread_cond_wait(&cond_tunnel,&mutex_tunnel);
		if(train.type==GL){
			pthread_mutex_lock(&mutex_garage_gl);
			nb_garage_gl_est--;
			nb_tunnel++;
			nb_voies_est++;
			pthread_cond_signal(&garage_gl_ouest);
			pthread_mutex_unlock(&mutex_garage_gl);
		}

		if(train.type==TGV){
			pthread_mutex_lock(&mutex_garage_tgv);
			nb_garage_tgv_est--;
			nb_tunnel++;
			nb_voies_est++;
			pthread_cond_signal(&garage_tgv_ouest);
			pthread_mutex_unlock(&mutex_garage_tgv);
		}
		if(train.type==M){
			nb_tunnel++;
		}
		pthread_mutex_unlock(&mutex_voies);
	}
	if(train.sens==OUEST){
		nb_voies_ouest--;
		pthread_cond_signal(&voies_ouest);
	}
	train.pos=TUNNEL;
	toString(train);
	pthread_mutex_unlock(&mutex_tunnel);
}
