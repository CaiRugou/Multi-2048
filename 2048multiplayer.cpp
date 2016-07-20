/*
 ============================================================================
 Name        	      :	2048.cpp
 Original_Author      : Maurits van der Schee having MIT Licence of this project
 

 
 Description : Added multiplayer functionality to 2048.
 	       

 Our Team : OpenC-IIIT

 ============================================================================
 */



#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <bits/stdc++.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
 
// just choosing a random port 
#define PORT 31217

#define SIZE 4


int  score=0;
bool ended;

char Gserver[100];
char name[100];
uint16_t board[SIZE][SIZE];


std::set<std::string> usernames;
std::map<std::string,int> username_score;
std::vector<std::string> feeds;

void getColor(uint16_t value, char *color, size_t length) {
	uint8_t original[] = {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0};
	uint8_t *background = original+0;
	uint8_t *foreground = original+1;
	if (value > 0) while (value >>= 1) {
		if (background+2<original+sizeof(original)) {
			background+=2;
			foreground+=2;
		}
	}
	snprintf(color,length,"\033[38;5;%d;48;5;%dm",*foreground,*background);
}





void drawBoard(uint16_t board[SIZE][SIZE]) {
	int8_t x,y;
	char color[40], reset[] = "\033[m";
	printf("\033[H");
	printf("%d\n",ended);
	printf("2048.cpp %17d pts\n\n",score);

	for (y=0;y<SIZE;y++) {
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			printf("       ");
			printf("%s",reset);
		}
		printf("\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			if (board[x][y]!=0) {
				char s[8];
				snprintf(s,8,"%u",board[x][y]);
				int8_t t = 7-strlen(s);
				printf("%*s%s%*s",t-t/2,"",s,t/2,"");
			} else {
				printf("   ·   ");
			}
			printf("%s",reset);
		}
		printf("\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			printf("       ");
			printf("%s",reset);
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←,↑,→,↓ or q/r        \n");
	printf("\033[A");
}




int8_t findTarget(uint16_t array[SIZE],int8_t x,int8_t stop) {
	int8_t t;
	// if the position is already on the first, don't evaluate
	if (x==0) {
		return x;
	}
	for(t=x-1;t>=0;t--) {
		if (array[t]!=0) {
			if (array[t]!=array[x]) {
				// merge is not possible, take next position
				return t+1;
			}
			return t;
		} else {
			// we should not slide further, return this one
			if (t==stop) {
				return t;
			}
		}
	}
	// we did not find a
	return x;
}



bool slideArray(uint16_t array[SIZE]) {
	bool success = false;
	int8_t x,t,stop=0;

	for (x=0;x<SIZE;x++) {
		if (array[x]!=0) {
			t = findTarget(array,x,stop);
			// if target is not original position, then move or merge
			if (t!=x) {
				// if target is not zero, set stop to avoid double merge
				if (array[t]!=0) {
					score+=array[t]+array[x];
					stop = t+1;
				}
				array[t]+=array[x];
				array[x]=0;
				success = true;
			}
		}
	}
	return success;
}

void rotateBoard(uint16_t board[SIZE][SIZE]) {
	int8_t i,j,n=SIZE;
	uint16_t tmp;
	for (i=0; i<n/2; i++){
		for (j=i; j<n-i-1; j++){
			tmp = board[i][j];
			board[i][j] = board[j][n-i-1];
			board[j][n-i-1] = board[n-i-1][n-j-1];
			board[n-i-1][n-j-1] = board[n-j-1][i];
			board[n-j-1][i] = tmp;
		}
	}
}

bool moveUp(uint16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x;
	for (x=0;x<SIZE;x++) {
		success |= slideArray(board[x]);
	}
	return success;
}

bool moveLeft(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

bool findPairDown(uint16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x,y;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE-1;y++) {
			if (board[x][y]==board[x][y+1]) return true;
		}
	}
	return success;
}


int16_t countEmpty(uint16_t board[SIZE][SIZE]) {
	int8_t x,y;
	int16_t count=0;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(uint16_t board[SIZE][SIZE]) {

	bool ended=  true;
	if (countEmpty(board)>0) return false;
	if (findPairDown(board)) return false;
	rotateBoard(board);
	if (findPairDown(board)) ended = false;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return ended;
}

void addRandom(uint16_t board[SIZE][SIZE]) {
	static bool initialized = false;
	int8_t x,y;
	int16_t r,len=0;
	uint16_t n,list[SIZE*SIZE][2];

	if (!initialized) {
		srand(time(NULL));
		initialized = true;
	}

	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				list[len][0]=x;
				list[len][1]=y;
				len++;
			}
		}
	}

	if (len>0) {
		r = rand()%len;
		x = list[r][0];
		y = list[r][1];
		n = ((rand()%10)/9+1)*2;
		board[x][y]=n;
	}
}



// This function modifies the terminal settings of buffered input
void setBufferedInput(bool enable) {
	static bool enabled = true;
	static struct termios old;
	struct termios new1;

	if (enable && !enabled) {
		// restore the former settings
		tcsetattr(STDIN_FILENO,TCSANOW,&old);
		// set the new state
		enabled = true;
	} else if (!enable && enabled) {
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO,&new1);
		// we want to keep the old setting to restore them at the end
		old = new1;
		// disable canonical mode (buffered i/o) and local echo
		new1.c_lflag &=(~ICANON & ~ECHO);
		// set the new settings immediately
		tcsetattr(STDIN_FILENO,TCSANOW,&new1);
		// set the new state
		enabled = false;
	}
}



// This function starts a socket server at the PORT defined above and on the current machine


/*

	For communication between client and server, I am doing a simple thing,

	there are two types of message, and the different parts of the message are delimited by colon " : "

	code = 0 , registerUser() message, with name and score = 0 , formatted as------->   0 : name : 0 

	code = 1,  uploadScore()  message, with name and score of client, formatted as ------> 1: name : points

*/

void displayLeaderBoard(){
	char color[40], reset[] = "\033[m";
	
	system("clear");
	getColor(4,color,40);
		
	
	
	printf("\t\t\t\t    %s2048   CONTEST   LEADERBOARD%s\n\n",color,reset);
	printf("\t\t\t\t         Registered  Users:%d\n\n",(int) username_score.size());
	
	
	getColor(2,color,40);	

	
	if(username_score.empty() )
		printf("\t\t\t\t\t  %sNo active users%s\n",color,reset);

	std::vector< std::pair<int,std::string> > score_board;
	for(auto s : username_score)
		score_board.push_back(std::make_pair(s.second,s.first));
	
	std::sort(score_board.begin() , score_board.end() );
	std::reverse(score_board.begin() , score_board.end()) ;
	//std::reverse(score_board.begin(), score_board.end()) ;
	int i  =0 ;
	for(  auto s = score_board.begin() ; s!= score_board.end() && i<10  ; s++)
	{
//		std::cout<<"\t\t\t"<<std::string(20,' ')<<(*s).second << " " << (*s).first << std::endl;
		printf("\t\t\t%20s\t\t%d\n",(*s).second.c_str(),(*s).first);
		i++;
	}

	for( ;i<10;i++)
		printf("\n");

	while( feeds.size() > 10 )
		feeds.erase(feeds.begin());
	getColor(16,color,40);
	printf("\t\t\t\t\t  %s LIVE     FEEDS %s \n\n",color,reset);
	for( auto s : feeds )
		std::cout <<"\t\t\t\t\t"<< s << std::endl;
	
	

}



void createGameServer()
{

   displayLeaderBoard();
	
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];
   memset(mesg,0,sizeof(mesg));

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(PORT);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
	  //if(errno == EINTR)
		//  continue;
      printf("%s\n",mesg); 
     
      int code,temp_score;
      char nme[1000];
      sscanf(mesg,"%d : %s : %d",&code,nme,&temp_score);

      
      if(code == 0 )
	{
		
		memset(mesg,0,sizeof(mesg));
		if( usernames.find(nme) != usernames.end() )
		{
		//	sprintf(mesg,"Sorry, the username is already registered!!\n");
		}
		else
		{
			usernames.insert(nme);
			username_score[nme] = 0;
		//	sprintf(mesg,"Start playing the game\n");
			char feed[100];
			memset(feed,0,sizeof(feed));
			sprintf(feed,ANSI_COLOR_GREEN "%s has joined the game!!!" ANSI_COLOR_RESET,nme);
			feeds.push_back(std::string(feed));
		}
      		sendto(sockfd,mesg,sizeof(mesg),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	}

	else if( code == 1 )
	{


		memset(mesg,0,sizeof(mesg));

		if( usernames.find(nme) != usernames.end())	
		{

			char feed[100];
			memset(feed,0,sizeof(feed));

			if(temp_score == -1)
			{
				sprintf(feed,ANSI_COLOR_RED "%s game has ended!!!" ANSI_COLOR_RESET,nme);
				feeds.push_back(std::string(feed));
			}

			else if(username_score[nme] && temp_score == 0){
				sprintf(feed, ANSI_COLOR_BLUE	"%s has restarted the game" ANSI_COLOR_RESET ,nme);
				feeds.push_back(std::string(feed));
			}
			if( temp_score != -1 )
				username_score[nme]  = temp_score;
		}
		
			
	}	
			
			
      displayLeaderBoard();
      
   }




   exit(EXIT_SUCCESS);

}







//This function provides the continuous updates to the server about scores of the user
void uploadScore()
{


   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1000];
   char recvline[1000];

   

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(Gserver);
   servaddr.sin_port=htons(PORT);
	
   
   std::cout << ended << std::endl;

   memset(sendline,0,sizeof(sendline));
   if( ended || gameEnded(board))
	sprintf(sendline,"%d : %s : %d\n",1,name,-1);	
   else
   	sprintf(sendline,"%d : %s : %d\n",1,name,score);
   sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));



}

void registerUser(char name[])
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1000];
   char recvline[1000];

   

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(Gserver);
   servaddr.sin_port=htons(PORT);
   char tname[100];
   
   memset(tname,0,sizeof(tname));
   
   sprintf(tname,"0 : %s : 0\n",name );
   sendto(sockfd,tname,strlen(tname),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
   recvfrom(sockfd,recvline,10000,0,NULL,NULL);
   recvline[strlen(recvline)-1]  = '\n';
 
   fputs(recvline,stdout);
   


}



// cleaning up after game is finished, terminal characterstics are revived
void signal_callback_handler(int signum) {
	ended = true;
	uploadScore();
        printf("         TERMINATED         \n");
        setBufferedInput(true);
        printf("\033[?25h");
        exit(signum);
}



// we need to clean up our configuration of terminal if any sudden exit of program happens, so setting signal_callback_handler as the default handler for any unexpected exit.
void setupSignalHandler()
{

        signal(SIGINT, signal_callback_handler);
        signal(SIGSTOP,signal_callback_handler);
        signal(SIGABRT,signal_callback_handler);
        signal(SIGQUIT,signal_callback_handler);
        signal(SIGTERM,signal_callback_handler);
        signal(SIGKILL,signal_callback_handler);
        signal(SIGTSTP,signal_callback_handler);



}



int main(int argc, char *argv[]) {
	
	char c;
	bool success;
	bool isClient =false;

	if (argc == 2 && strcmp(argv[1],"server")==0 ) {
		puts("Starting the 2048 server....");
		createGameServer();
	}
	if (argc == 2 && strcmp(argv[1],"client")==0 ){
		printf("Enter the username :");
		std::cin >> name;
		printf("Enter the server :");
		std::cin >> Gserver;
		registerUser(name);
		isClient = true;
	}

	printf("\033[?25l\033[2J\033[H");

	// register signal handler for when ctrl-c ,ctrl-z etc	
	setupSignalHandler();




	memset(board,0,sizeof(board));
	addRandom(board);
	addRandom(board);
	drawBoard(board);
	setBufferedInput(false);

	while (true) {
		c=getchar();
		switch(c) {
			case 97:	// 'a' key
			case 104:	// 'h' key
			case 68:	// left arrow
				success = moveLeft(board);  break;
			case 100:	// 'd' key
			case 108:	// 'l' key
			case 67:	// right arrow
				success = moveRight(board); break;
			case 119:	// 'w' key
			case 107:	// 'k' key
			case 65:	// up arrow
				success = moveUp(board);    break;
			case 115:	// 's' key
			case 106:	// 'j' key
			case 66:	// down arrow
				success = moveDown(board);  break;
			default: success = false;
		}
		if (success) {
			if( isClient ) uploadScore();
			drawBoard(board);
			//usleep(500);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) {
				uploadScore();
				printf("         GAME OVER          \n");
				break;
			}
		}
		if (c=='q') {
			printf("        QUIT? (y/n)         \n");
			while (true) {
			c=getchar();
				if (c=='y'){
					setBufferedInput(true);
					printf("\033[?25h");
					exit(0);
				}
				else {
					drawBoard(board);
					break;
				}
			}
		}
		if (c=='r') {
			printf("       RESTART? (y/n)       \n");
			while (true) {
			c=getchar();
				if (c=='y'){
					score = 0;
					uploadScore();
					memset(board,0,sizeof(board));
					addRandom(board);
					addRandom(board);
					drawBoard(board);
					break;
				}
				else {
					drawBoard(board);
					break;
				}
			}
		}
	}
	setBufferedInput(true);

	printf("\033[?25h");

	return EXIT_SUCCESS;
}
