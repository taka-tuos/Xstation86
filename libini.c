#include "Xstation86.h"

char libini_in[256];

void libini_iniopen(char *name)
{
	strcpy(libini_in,name);
}

int libini_iniread_n(char *name)
{
	int n;
	char buf[256];
	FILE *fp;
	char *p;
	
	fp = fopen(libini_in,"rt");
	buf[0] = 0;
	for(;strncmp(buf,name,strlen(name)) != 0;)fgets(buf,256,fp);
	for(p = buf + strlen(name);*p - '0' > 9 || *p - '0' < 0; p++);
	n = strtol(p,&p,0);
	
	fclose(fp);
	
	return n;
}

char *libini_iniread_s(char *name)
{
	char buf[256];
	char *s;
	FILE *fp;
	char *p;
	int i;
	
	s = (char *)malloc(256);
	
	fp = fopen(libini_in,"rt");
	buf[0] = 0;
	for(;strncmp(buf,name,strlen(name)) != 0;)fgets(buf,256,fp);
	for(p = buf;*p != '\"'; p++);
	
	for(p++,i=0;*p != '\"';p++,i++) s[i] = *p;
	s[++i] = 0;
	
	fclose(fp);
	
	return s;
}