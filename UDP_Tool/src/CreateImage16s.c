#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <time.h>

#include <unistd.h>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>

#include "rp.h"


#define BUFLEN 16384  //Max length of buffer // 16*1024
#define PORT 7536   //The port on which to listen for incoming data
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#define nbline 256

double SMAX = 255;
double SMIN = -255;

    int tbuff, i,j  = 0;
    int CheckUDP = 0;

// preparing the UDP connection

//
// ASUS : 192.168.1.47
// Port : 8888
//

void die(char *s)
{
    perror(s);
    exit(1);
}

long timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}



// Pitaya code in itself
int main(int argc, char **argv){
	clock_t tStart, tTop, tEnd;
	/*time_t     now;
	struct tm  ts;

	char       bufdate[24];
	char       buftime[24];
	time(&now);
	ts = *localtime(&now);
    	strftime(buftime, sizeof(buftime), "%H%M%S", &ts);
	strftime(bufdate, sizeof(bufdate), "%y%m%d", &ts);
	int LTime = atoi(buftime);
	int LDate = atoi(bufdate);
   	printf("Starting at : %i\n", LTime);
	printf("Starting at : %i\n", LDate);
	printf("Starting at : %i\n", LDate+LTime);*/
	FILE * fm;
	fm = fopen ("moy.txt", "w+");

	//int PORT = 7536;
	printf("DEBUG COMMENCE\n");
	/* Initialize UDP */
	struct sockaddr_in si_me, si_other;
	int s;
	int slen = sizeof(si_other);
	//char buf[BUFLEN];
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
	die("socket");
	}

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_other.sin_family = AF_INET;
	si_me.sin_port = htons(PORT); // Port
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.s_addr = inet_addr("192.168.1.23"); //specific binding to listenser
	//si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){die("bind");}
	/* End of UDP initialization */




	/*number of acquision*/
	uint32_t N = 8;
	int LineImage = 66;
	/*size of the acquisition buffer*/
	uint32_t buff_size = 16384;
	uint32_t HalfSignal = buff_size/2;
	/*allocation of buffer size in memory*/

	tStart = clock();
	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}


		

	for (int k=0; k < nbline ; k++){

	float *buff = (float *)malloc(buff_size * sizeof(float));
	int *ibuff = (int *)malloc(buff_size * sizeof(int));

	/*initialise to 0 the buffer*/
	for (int i=0 ; i<buff_size ; i++){buff[i]=0.0;}
		rp_AcqSetDecimation(RP_DEC_64);
		uint32_t buff_sizeW = 12000;
		float *tempW = (float *)malloc(buff_sizeW * sizeof(float));
		float *buffW = (float *)malloc(buff_sizeW * sizeof(float));
	
		rp_acq_trig_state_t stateW = RP_TRIG_STATE_TRIGGERED;
		/*start acquisition must be set before trigger initiation*/
		rp_AcqStart();
		/*allocation of temporary buffer size in memory*/

	
		/*trigger source -- Channel B, Negatif*/ 
		rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
		/*level of the trigger activation in volt*/
		double PWMTriggerLevel = 0.05;
		rp_AcqSetTriggerLevel(PWMTriggerLevel);
		/*waiting for trigger*/
		while(1){
			rp_AcqGetTriggerState(&stateW);
			if(stateW == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}		
		/*putt acquisition data in the temporary buffer*/
		rp_AcqGetOldestDataV(RP_CH_1, &buff_sizeW, tempW);
		/*additionning the N signals*/
		for (int j = 0; j < buff_sizeW; j++){
			buffW[j]+=tempW[j];
		}
		//printf("%i\n", k);
		/*release memory*/
		free(tempW);
		int PWM_Position = 0;
		for (i=0 ; i<buff_sizeW ; i++) {
			if (buffW[i] > PWMTriggerLevel ){PWM_Position++;}	
			//printf("%f ",buffW[i]);
		}
		PWM_Position = PWM_Position -2000; //lowering the base, staying over max though
		//printf("PWM : %i\n",PWM_Position);




		// On commence alors l'acquisition des échantillons
		/*init trigger state*/
		/* decimation n (=1,8,64...) : frequency = 125/n MHz*/
		rp_AcqSetDecimation(RP_DEC_1);
		rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
		tTop = clock();
		for (int i=0 ; i<N ; i++) {

			/*start acquisition must be set before trigger initiation*/
			rp_AcqStart();

			/*allocation of temporary buffer size in memory*/
			float *temp = (float *)malloc(buff_size * sizeof(float));

			/*trigger source -- Channel B, Negatif*/ 
			rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHB_NE);

			/*level of the trigger activation in volt*/
			rp_AcqSetTriggerLevel(0.01); 

			/*acquisition trigger delay*/
			rp_AcqSetTriggerDelay(HalfSignal);
		
			/*waiting for trigger*/
			while(1){
				rp_AcqGetTriggerState(&state);
				if(state == RP_TRIG_STATE_TRIGGERED){
					break;
				}
			}		
			//printf("%i\n",i);
			/*putt acquisition data in the temporary buffer*/
			rp_AcqGetOldestDataV(RP_CH_2, &buff_size, temp);

			/*additionning the N signals*/
			for (int j = 0; j < buff_size; j++){
				buff[j]+=1000*temp[j];
			}

			/*release memory*/
			free(temp);
		}
	
		double MoyenneFichier = 0;
		for (int i=HalfSignal ; i<buff_size ; i++) {
			MoyenneFichier += buff[i];	
		}
		MoyenneFichier = MoyenneFichier / HalfSignal;
		//printf("MoyenneFichier : %f\n",MoyenneFichier);

		float XCarre=0.0;
		float SCarre=0.0;

		for (int i=HalfSignal ; i<buff_size ; i++) {
			XCarre = (buff[i]-MoyenneFichier);	
			SCarre += XCarre*XCarre;
		}
		SCarre = SCarre/HalfSignal;

		//printf("SCarre : %f\n",SCarre);
		/*Nettoyage et simplification du signal */
		double ttmp = 0;
		for (int i=0 ; i<buff_size ; i++) {
			ttmp = 255*(buff[i]- MoyenneFichier)*(SCarre/100.0);
			
			/* Ecretage moche */
			
	
			if(ttmp > (SMAX)){
				ttmp = SMAX;
			}
			if(ttmp < (SMIN)){
				ttmp = SMIN;
			}		
						
			


			/* Fin ecretage */
			ibuff[i] = ttmp;
			//printf("%i\t",ibuff[i]);
			// On ecrit dans le fichier les int
			fprintf(fm, "%i\t", (int)ibuff[i]);
		}
		free(buff);
		fprintf(fm, "\n");

		/* End of cleaning*/

		//Sending the image
		int sampling = 16;
		int samples = 256;
		int PointsNb = buff_size/sampling;

		int *iToBuff = (int *)malloc(PointsNb * sizeof(int));
		double *tmp = (double *)malloc(PointsNb * sizeof(double));


		for (int i = 0 ; i < PointsNb ; ++i) {
			tmp[i] = 0.0;
			for (int j = 0 ; j < sampling ; ++j) {

		   		 tmp[i] += (ibuff[i*sampling+j]*ibuff[i*sampling+j]) ;
			
			}
			iToBuff[i] = (int)(((tmp[i] / sampling)));
			//printf("%i ", iToBuff[i]);
		}
		free(ibuff);



		// Checking for a file 
		



		

		/* sending to UDP */
		//int DATA_Line = 0; // Number of the line in the image (ie 1 to 40 in our case)
		int DATA_Image = 0; //Number of the image in the acq 
		int MaxPointsPerPacket = 256; // (in bytes)
		int NbPackets = PointsNb/MaxPointsPerPacket;
		
		int TaillerBuffer = samples+3+4;
		
		int UDPBuffer[TaillerBuffer];
		/*
		printf("\nTaillerBuffer : %i\n",TaillerBuffer);
		printf("buff number of points : %i\n",buff_size);
		printf("Nb of points of raw signal : %i\n",PointsNb);


		printf("Sampling (sum the squares over XX points) : %i\n",sampling);
		printf("Number of points in the data : %i\n",samples);
		printf("UDP Port used : %i\n",PORT);
		printf("Packet size (in bytes) : %i\n",TaillerBuffer);
		printf("Nb packet : %i\n",NbPackets);
		
*/
		printf("Ligne k : %i\n",k);
		int DATA_Line = LineImage;
		for (int i=0 ; i<TaillerBuffer ; i++){UDPBuffer[i]=0;}
		for (i=0 ; i < NbPackets ; i++) { // iteration sur l'ensemble des packets du buffer

			UDPBuffer[0] = i; // Position of the packet inside the line
			UDPBuffer[1] = DATA_Image; // Number of the line
			UDPBuffer[2] = DATA_Line; //Number of the image
			UDPBuffer[3] = PWM_Position; //BUG @corriger
			UDPBuffer[4] = k; //BUG @corriger à la reception -- je voulais mettre datetimestamp
			tEnd = clock();
			UDPBuffer[5] =  (int)timediff(tStart, tTop); //BUG @corriger -- début de l'acquisition de la ligne
			UDPBuffer[6] = (int)timediff(tStart, tEnd); //BUG @corriger -- fin de l'acquisition de la ligne

			for (j=0; j < samples ; j++){ // iteration par points du packet
				UDPBuffer[7+j] = iToBuff[i*samples+j];
			}

			CheckUDP = sendto(s,UDPBuffer,sizeof(UDPBuffer),0,(struct sockaddr*) &si_other,slen);
	
			usleep(2);
			if (CheckUDP==-1) {die("sendto()");}
		}
	
		/* end of emission */
	free(iToBuff);
	free(tmp);
	usleep(2);
	}
	/* Release rp resources */
	fclose(fm);
	rp_Release();

	return 0;
}
