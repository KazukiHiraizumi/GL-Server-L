#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>

#include "cparser.h"

using namespace std;
static void error_callback(int error, const char* description){
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
//	if (key==GLFW_KEY_ESCAPE && action==GLFW_PRESS) glfwSetWindowShouldClose(window,GL_TRUE);
}

//Global parameters
int width,height;
float aspect;
float apixel;
float px2gl(int px){
	return 2*(float)px/width-1;
}
float a2f(char *s){
	if(s==NULL) return 1;
	else return atof(s);
}
void drawWav(float lamda,float phase,float g0,float g180,int bin){
  fprintf(stderr,"Wav %f %f %f %f %d\n",lamda,phase,g0,g180,bin);
	for(int i=0;i<width;i++){
		float lx=px2gl(i);
		float ip=(1+cos(2*M_PI*(lx/lamda-phase)))/2;
		float grey=ip>0.5? g0:g180;
		if(!bin) grey=g0*ip+g180*(1-ip);
		glBegin(GL_LINES);
		glColor3f(grey,grey,grey);
		glVertex2f(lx,-aspect);
		glVertex2f(lx, aspect);
		glEnd();
	}
}
void drawCircle(float x,float y,float rad,float g){
//	fprintf(stderr,"rad %f\n",apixel);
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(g,0.f,0.f);
	glVertex2f(x,y);
	for(int i=0;i<=15;i++){
		glVertex2f(x+rad*sin(2*M_PI/15*i),y+rad*cos(2*M_PI/15*i));
	}
	glEnd();
	glPointSize(1);
	glBegin(GL_POINTS);
	glColor3f(g,0.f,0.f);
	glVertex2f(x,y);
	glEnd();
}
void drawBand(float wid,float g){
	float y=wid;
	glBegin(GL_QUADS);
	glColor3f(g,g,g);
	glVertex2f(1,y);
	glVertex2f(-1,y);
	glVertex2f(-1,-y);
	glVertex2f(1,-y);
	glEnd();
}

int main(void){
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwSetErrorCallback(error_callback);

//	glfwWindowHint(GLFW_ALPHA_BITS, 0);
//	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
#ifdef KIOSK
	GLFWwindow *window=glfwCreateWindow(mode->width,mode->height,"OKNGL",monitor,NULL);
#else
	GLFWwindow *window=glfwCreateWindow((int)(mode->width*0.7),(int)(mode->width*0.7*9/16),"OKNGL",NULL,NULL);
#endif
	if (!window){
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
//	glfwSetKeyCallback(window,key_callback);
	glfwMakeContextCurrent(window);
  glfwGetFramebufferSize(window,&width,&height);
	aspect=(float)height/width;
	apixel=px2gl(1)-px2gl(0);
	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(-1.f,1.f,-aspect,aspect,1.f,-1.f);
//	glLoadIdentity();
	gluLookAt(0.0,0.0,1.0, 0.0,0.0,0.0, 0.0,1.0,0.0);

	static GLfloat lightPosition[]={-10.0,10.0,10.0,1.0};
	glLightfv(GL_LIGHT0,GL_POSITION,lightPosition);
	static GLfloat lightAmbient[]={0.4,0.4,0.4,1.0};
	glLightfv(GL_LIGHT0,GL_AMBIENT,lightAmbient);
//	static GLfloat matFrontCol[] = {0.7,1.0,1.0,1.0};
//	glMaterialfv(GL_FRONT,GL_DIFFUSE,matFrontCol);
//Depth Setting
//	glDepthRange(0,255);
//	glDepthFunc(GL_LEQUAL);
//	glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);

  /* First call to socket() function */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
      perror("ERROR opening socket");
      exit(1);
  }
  /* Initialize socket structure */
  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  int portno = 8888;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }
  struct sockaddr_in cli_addr; 
  listen(sockfd,5);
  socklen_t clilen = sizeof(cli_addr);

	double tsch=(float)glfwGetTime();
  int loop=0;
  static char buffer[20000];
  int rdcount;
  int newsockfd;
  char **arg=NULL;
  double gltm;
  int seq=0;
LOOP:
  if(glfwWindowShouldClose(window)){
    glfwDestroyWindow(window);
  	glfwTerminate();
    exit(EXIT_SUCCESS);
  }
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if(newsockfd<0){
    perror("ERROR on accept");
    goto LOOP;
  }
READ:
  fprintf(stderr,"READ\n");
  memset(buffer,0,20000);
  rdcount=read(newsockfd,buffer,20000);
  if(rdcount<=0){
    perror("read socket error");
    goto LOOP;
  }
  cparser_set(buffer);
  arg=NULL;
PARSE:
	arg=cparser_next();
	if(arg==NULL) goto READ;
	if(loop==0){
	  glClearColor(0.f,0.f,0.f,1.f);
	  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	  glDisable(GL_LIGHTING);
		glLoadIdentity();
		glRotatef(0.f,1.f,0.f,0.f);
	}
  fprintf(stderr,"CMD %c\n",arg[0][0]);
	switch(arg[0][0]){
	case 'W':
	  drawWav(a2f(arg[1]),a2f(arg[2]),a2f(arg[3]),a2f(arg[4]),strcmp(arg[5],"false"));
	  break;
	case 'C':
	  drawCircle(a2f(arg[1]),a2f(arg[2]),a2f(arg[3]),a2f(arg[4]));
	  break;
	case 'B':
	  drawBand(a2f(arg[1]),a2f(arg[2]));
    break;
	case 'G':
	  goto RESPONSE;
	  break;
	}
  goto PARSE;
RESPONSE:
	fprintf(stderr,"Glfw swap buffer\n");
	glfwSwapBuffers(window);
	gltm=glfwGetTime();
	sprintf(buffer,"%f\n",(gltm-tsch)*1000);
  rdcount=write(newsockfd,buffer,strlen(buffer));
	fprintf(stderr,"%d %s\n",seq++,buffer);
	tsch=gltm;
  if(rdcount<=0){
    perror("write socket error");
    goto LOOP;
  }
//  glfwPollEvents();
  goto PARSE;
QUIT:
  if(sockfd>0) close(sockfd);
}
