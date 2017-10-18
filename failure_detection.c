/*
Single Author info:
All of us contributed equally
  rrshah3 Rhythm R Shah
  smsejwan Shalini M Sejwani
  spshriva Shalki Shrivastava
Group info:
  rrshah3 Rhythm R Shah
  smsejwan Shalini M Sejwani
  spshriva Shalki Shrivastava
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

int n;
char **storedIP;
char **storedPort;
int sendokay=0;
int okayrcvd =0;
char *myipaddress;
int myindex;
int serverkill=0;
int mytimer;
int seedrcvd;
int gossipb;
int badnodes;
int iterations;
int *info;
int *timestamp;
int P;  //time between failures
int B;  //number of bad nodes
int F;  //dead time
pthread_mutex_t mutex;
int T;
int count=0;
int poorme =0;
int myhb=0;

void *Server()
{
  int rcvdinfo[n+1];
  int dfd;
  int udpSocket, nBytes,lines=0;
  char buffer[1024];
  struct sockaddr_in serverAddr, clientAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;
  int i;

  
  //get IP address of machine
  int n1;
  struct ifreq ifr;
  char array[] = "eth0";
  n1 = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
  ioctl(n1, SIOCGIFADDR, &ifr);
  close(n1);
  //display result
  myipaddress = inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
  // fprintf(sfile," my IP Address is %s - %s\n" , array , myipaddress );


  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = 0;
  serverAddr.sin_addr.s_addr = inet_addr(myipaddress);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverStorage;

  //get port number
  int portno;
  struct sockaddr_in sin;
  socklen_t length = sizeof(sin);
  if (getsockname(udpSocket, (struct sockaddr *)&sin, &length) == -1)
      perror("getsockname");
  else
  {
      portno = ntohs(sin.sin_port);
      // printf("port number %d\n", portno);
  }

//writing my ip address and port to the end points file
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  fp  = fopen("endpoints.txt","a+");
    fprintf(fp,"%s %d\n",myipaddress,portno );
  fclose(fp);

  //allocating memory for storedip and storedport
  storedIP = malloc(n*sizeof(char*));
  storedPort = malloc(n*sizeof(char*));
  int k;
  for(k=0; k<n; k++)
  {
    storedIP[k] = malloc(50*sizeof(char));
    storedPort[k] = malloc(50*sizeof(char));
  }

  //getting myindex and copying ip and port till myindex
  fp = fopen("endpoints.txt", "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    char find = ' ';
    const char *ptr = strchr(line, find);
    int index;
    if(ptr) 
    {
        index = ptr - line;
    }
    // memcpy( storedPort[lines],line+index+1, read-index-2); // printf("original length %d",strlen(storedPort[lines]));    // storedPort[lines]+='\0';     // printf("original length %d",strlen(storedPort[lines]));
    // memcpy( storedIP[lines],line, index );    // printf("ip address:%s and length is %d\n",storedIP[lines],strlen(storedIP[lines]));    // printf("port is:%s and length is %d\n",storedPort[lines],strlen(storedPort[lines]) );
    lines++;
  }
  fclose(fp);


  myindex = lines;
    pthread_mutex_lock(&mutex);
  for (k=0;k<n;k++)
    info[k]=0;
  // info[0] = myindex;
  pthread_mutex_unlock(&mutex);



  if(myindex==n)
    sendokay = 1;

  nBytes = recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
  if(strcmp(buffer,"okay")==0)    
  {
    okayrcvd = 1;
          mytimer = 0; 
    // srand(seedrcvd+myindex);
    // printf("okay Received\n");
  }

  while(1)
  {
    // printf("fdjs\n");
    nBytes = recvfrom(udpSocket,rcvdinfo,(n+1)*sizeof(int),0,(struct sockaddr *)&serverStorage, &addr_size);
    // printf("Received message\n");
    int p;
    if(poorme!=1)
    {
      pthread_mutex_lock(&mutex);
      // printf("received is:\n");
      // for(p=0;p<=n;p++)
      //   printf("received info is: %d\n",rcvdinfo[p] );
      for(p=0; p<n;p++){
        // if(p==rcvdinfo[0])
        // {
        //     info[p]++;
        //     timestamp[p] = mytimer;

        // }
        // else
        // {
          if(info[p]<rcvdinfo[p])
          {
            timestamp[p] = mytimer;
            info[p] = rcvdinfo[p];
          }
        // }
      }
      pthread_mutex_unlock(&mutex);
    } 
  }

  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{

  if(argc != 9)
  {
    printf("too few arguments\n");
    exit(1);
  }

  pthread_mutex_init(&mutex,NULL);
  n= atoi(argv[1]);
  gossipb = atoi(argv[2]);
  iterations = atoi(argv[3]);
  F = atoi(argv[4]);
  B = atoi(argv[5]);
  P = atoi(argv[6]);
  seedrcvd = atoi(argv[7]);
  T = atoi(argv[8]);
  info = malloc(n*sizeof(int));
  timestamp = malloc(n*sizeof(int));
  int abc;
  int activenodes=n;
  int dead[n];
  for(abc=0;abc<n;abc++)
    dead[abc] = 0;
  // if(n-B <= gossipb)
  // {
  //   printf("give proper parameters. As n(%d) - B(%d) <= b(%d) some nodes wont have enough nodes to send heartbeat messages to.\n",n,B,gossipb );
  //   exit(1);
  // }
  
  pthread_t thread;
  int rc;
  rc = pthread_create(&thread, NULL, Server, NULL);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }

  // printf("going to busy waiting\n");
  while( sendokay==0 && okayrcvd==0);
  // printf("OUT of busywaiting with sendokay:%d and okayrcvd:%d\n",sendokay,okayrcvd);

  int lines = 0;
  FILE *fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  fp = fopen("endpoints.txt", "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    char find = ' ';
    const char *ptr = strchr(line, find);
    int index;
    if(ptr) 
    {
        index = ptr - line;
    }
    // printf("index:%d, read:%d\n",index,read-index-2 );
    // printf("line is %s\n",line );
    memcpy( storedPort[lines],line+index+1, read-index-2); // printf("original length %d",strlen(storedPort[lines]));    // storedPort[lines]+='\0';     // printf("original length %d",strlen(storedPort[lines]));
    memcpy( storedIP[lines],line, index );    
    // printf("ip address:%s and length is %d\n",storedIP[lines],strlen(storedIP[lines]));    
    // printf("port is:%s and length is %d\n",storedPort[lines],strlen(storedPort[lines]) );
    lines++;
  }
  fclose(fp);


  // printf("the ip and port in endpoints are:\n");

  int clientSocket, portNum, nBytes;
  char buffer[1024];
  struct sockaddr_in serverAddr[n];
  socklen_t addr_size[n];

  /*Create UDP socket*/
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  int l;
  for(l=0;l<n;l++)
  {
    // if(l==myindex) continue;
    // printf("%d\tIP address:%s\tPort:%s\n",l,storedIP[l],storedPort[l] );
    serverAddr[l].sin_family = AF_INET;
    serverAddr[l].sin_port = htons(atoi(storedPort[l]));
    // printf("port %d\n",atoi(storedPort[0]) );
    serverAddr[l].sin_addr.s_addr = inet_addr(storedIP[l]);
    // printf("ip \n");
    memset(serverAddr[l].sin_zero, '\0', sizeof serverAddr[l].sin_zero);  
    addr_size[l] = sizeof serverAddr[l];
  }

  int m;
  char okay[5] = {'o','k','a','y','\0'};
  // printf("array okay is %s\n",okay );
  if(sendokay ==1)
    for(m=0;m<=myindex-1;m++)
        sendto(clientSocket,okay,5,0,(struct sockaddr *)&serverAddr[m],addr_size[m]);

  
  
  int failrandom;
  int failed=0;
  int random1;
  int flag[n],flagfail[n],x=0,flag1=0,recepient[gossipb],neighborlist[gossipb],random;
  for(x=0;x<n;x++)
        flagfail[x]=0;
  

  unsigned int seed1 = seedrcvd;
  unsigned int seed2 = seedrcvd + myindex;
  int pqr;

// printf("gossip b is %d\n",gossipb );
  
  int gossipc=0;
  int min=0;
  while(gossipc<iterations || mytimer<T)
  {

    if(gossipc == iterations ) gossipc =0;
    if(mytimer==T) break;
    
    for(x=0;x<n;x++)
      flag[x]=0;

    x=0;
    if(gossipb<=activenodes)
      min = gossipb;
    else
    {
      // printf("deadlock condition\n");
      min = activenodes;
    }
    while(x<min)
    {
      flag1=0;
      // printf("x is %d\n",x );
      while(flag1==0)
      {
        /*Chance of deadlock i.e. N-B <= b */
        /*NEED TO HANDLE THIS CASE*/

        random1 = rand_r(&seed2);
        random = random1%n;
        if(dead[random]==0 && flag[random]==0 && random != myindex-1)
        {
          // printf("random is %d\n",random );
          // printf("in if\n");
          recepient[x] = random;
          flag[random] = 1;
          flag1 = 1;
        }
      }
      x++;

    }

  
   if(poorme != 1)
    {
      // printf("for iteration %d\n",gossipc );
      // printf("message will be sent to\n");
      // for(x=0;x<gossipb;x++)
      // printf("%d\n",recepient[x]);
      for(m=0;m<gossipb;m++)
      {
        pthread_mutex_lock(&mutex);
        // printf("ddss 1\n");
        // printf("sending:\n");
        // for(x=0;x<=n;x++)
        //   printf("info is: %d\n",info[x]);
        sendto(clientSocket,info,n*sizeof(int),0,(struct sockaddr *)&serverAddr[recepient[m]],addr_size[recepient[m]]);
        // sleep(1);
        // printf("fsds 2\n");
        pthread_mutex_unlock(&mutex);
      }
      // printf("messages sent\n");
    }
  
    sleep(1);
    mytimer++;
    pthread_mutex_lock(&mutex);
    info[myindex-1] += 1;
    pthread_mutex_unlock(&mutex);
    gossipc++;
    if(mytimer%P == 0 && failed<B)
    {
      // printf("gotta fail a node.\n");
      flag1=0;
      while(flag1==0)
      {
        random1 = rand_r(&seed1);
        random = random1%n;
        if(flagfail[random] == 0)
        {
          if(random == myindex-1) {
            poorme=1;
            flagfail[random] = 1;
            flag1 =1;
            // printf("will fail node %d\n",random );
            failed++;
            dead[myindex-1] = 1;
            // printf("jumping as i m dead! :'(\n");
            goto awww;
          }
          flagfail[random] = 1;
          flag1 =1;
          // printf("will fail node %d\n",random );
          failed++;
        }
      }
    }
    
    for(pqr=0;pqr<n;pqr++)
    {
      if(pqr==myindex-1) continue;
      if(mytimer-timestamp[pqr] >= F)
      {
        dead[pqr] = 1;
        activenodes--;
        //info[pqr] = 0;
        // printf("node %d has died at: %d with heartbeat: %d\n",pqr, timestamp[pqr], info[pqr] );
      }
    }

  }

  awww:
  // printf("after aww\n");
  while(mytimer<T)
  {
    sleep(1);
    mytimer++;
    for(pqr=0;pqr<n;pqr++)
    {
      if(pqr==myindex-1) continue;
      if(mytimer-timestamp[pqr] >= F)
      {
        dead[pqr] = 1;
        //info[pqr] = 0;
        // printf("node %d has died at: %d with heartbeat: %d\n",pqr, timestamp[pqr], info[pqr] );
      }
    }
  }
  // sleep(15);
  // printf("main printing  final info:\n");
  char name[20];
  memset(name,0,20);
  sprintf(name,"list%d",myindex);
  FILE *ax;
  ax = fopen(name,"w");
  char* status;
  if(dead[myindex-1] == 1) 
    status = "FAIL";
  else 
    status = "OK"; 
  fprintf(ax,"%s\n",status );
  pthread_mutex_lock(&mutex);
  for(m=0;m<n;m++)
    fprintf(ax,"%d %d\n",m+1,info[m]);
  pthread_mutex_unlock(&mutex);

  // remove("endpoints.txt");
  return 0;
}