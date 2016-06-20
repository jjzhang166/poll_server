#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <arpa/inet.h>

#define _BACKLOG_ 5
#define _CLIENT_ 64

static void usage(const char* arg)
{
	printf("usage:%s [ip][port]",arg);
}

static int start(char *ip,int port)
{
	assert(ip);
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(1);
	}
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(port);
	local.sin_addr.s_addr=inet_addr(ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		exit(2);
	}

	if(listen(sock,_BACKLOG_)<0)
	{
		perror("listen");
		exit(3);
	}

	return sock;
}

int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		exit(1);
	}
	int i,maxi;   //client数组的最大下标
	int port=atoi(argv[2]);
	char *ip=argv[1];

	int listenfd=start(ip,port);
	int done=0;
	int new_sock=-1;
	struct sockaddr_in client;
	socklen_t len=sizeof(client);
	struct pollfd clientfds[_CLIENT_];

	clientfds[0].fd=listenfd;
	clientfds[0].events=POLLIN;
	int _timeout=5000;
	for(i=1;i<_BACKLOG_;++i)
	{
		clientfds[i].fd=-1;
	}
	maxi=0;

	while(!done)
	{
		switch(poll(clientfds,maxi+1,_timeout))
		{
			case 0:
				printf("time out\n");
				break;
			case -1:
				perror("poll");
				exit(1);
			default:
				{
				//	for( i=0;i<_CLIENT_ ;++i)
					{
						if(clientfds[0].revents&POLLIN)
						{
							new_sock=accept(listenfd,(struct sockaddr*)&client,&len);
							if(new_sock<0)
							{
								perror("accept");
								continue;
							}
							printf("get a connect...%d\n",new_sock);
							for(i=1;i<_CLIENT_;++i)
							{
								if(clientfds[i].fd<0)
								{
									clientfds[i].fd=new_sock;   //save descriptor
									break;
								}
								if(i==_CLIENT_)
								{
									close(new_sock);
								}
							}
							clientfds[i].events=POLLIN;
							if(i>maxi)
							{
								maxi=i;
							}
						}

					//	for(i=1;i<=maxi;++i)
						{
							if(clientfds[i].revents&POLLIN)
							{
								char buf[1024];
								ssize_t _s=read(clientfds[i].fd,buf,sizeof(buf)-1);
								if(_s>0)
								{
									buf[_s]='\0';
									printf("client: %s",buf);
								}
								else if(_s==0)
								{
									printf("client quit...\n");
									close(clientfds[i].fd);
									clientfds[i].fd=-1;
								}
							}
						}
					}
				}
				break;
		}
	}

//	int ret;
//	while(1)
//	{
//		ret=poll(clientfds,maxi+1,_timeout);
//		if(ret<0)
//		{
//			printf("poll error\n");
//			exit(1);
//
//		}
//		else if(ret==0)
//		{
//			perror("time out");
//			//exit(2);
//		}
//		if(clientfds[0].revents&POLLIN)
//		{
//			new_sock=accept(listenfd,(struct sockaddr*)&client,&len);
//			printf("get a connect...%d\n",new_sock);
//			for(i=1;i<_CLIENT_;++i)
//			{
//				if(clientfds[i].fd<0)
//				{
//					clientfds[i].fd=new_sock;   //save descriptor
//					break;
//				}
//			}
//			if(i==_CLIENT_)
//			{
//
//				close(new_sock);
//			}
//			clientfds[i].events=POLLIN;
//			if(i>maxi)
//			{
//				maxi=i;
//			}
//		}
//
//		for(i=1;i<=maxi;++i)
//		{
//			if(clientfds[i].fd<0)
//			{
//				continue;
//			}
//			if(clientfds[i].revents&POLLIN)
//			{
//				char buf[1024];
//				ssize_t _s=read(clientfds[i].fd,buf,sizeof(buf)-1);
//				if(_s>0)
//				{
//					buf[_s]='\0';
//					printf("client: %s",buf);
//				}
//				else if(_s==0)
//				{
//					printf("client quit...\n");
//					close(clientfds[i].fd);
//					clientfds[i].fd=-1;
//				}
//			}
//		}
//	}
	return 0;
}
