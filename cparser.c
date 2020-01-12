#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "cparser.h"

static char *cparsed[16];
static char buffer[512];
static char *s1;
static int cn=0,pn=0;

void cparser_set(char *f){
  strcpy(buffer,f);
	s1=buffer;
	cn=pn=0;
}
char **cparser_next(void){
  for(;;){
    int a=*s1;
    if(iscntrl(a)){
      if(cn>0){
        *s1=0;
        s1++;
        break;
      }
      else return NULL;
    }
    else if(a==';'){
			if(cn>0){
				*s1=0;
        s1++;
				break;
			}
		}
		else if(a==' '){
			if(pn==cn) continue;
			else{
				*s1=0;
				s1++;
				pn=cn;
			}
		}
		else{
			if(pn==cn) cparsed[cn++]=s1;
			s1++;
		}
	}
	for(int i=cn;i<16;i++){
		cparsed[i]=NULL;
	}
  cn=pn=0;
	return cparsed;
}
/*
int main(int argc,char **argv){
	for(int i=0;;i++){
		char **argv=cparser(stdin);
		if(argv==NULL) break;
		printf("%d) %s %s %s\n",i,argv[0],argv[1],argv[2]);
	}
	return 0;
}
*/