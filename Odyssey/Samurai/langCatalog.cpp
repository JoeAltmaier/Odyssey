#include <stdio.h>
#include <string.h>
#include "SamuraiServer.h"


void	
SamuraiServer::build_lookup ()
{

	int    i=0, 
		   j=0;
	char   tmp[32];
	
	strcpy (tbl[0], "echo");
	strcpy (tbl[1], "repeat");
	strcpy (tbl[2], "createtbl");
	strcpy (tbl[3], "\0");

/*	while(strcmp(tbl[i], "\0") != 0)
	{
		while(tbl[i][j] != '\0')  
		{
			tmp[j] = tbl[i][j];
			j++;
		}
		tmp[j]='\0';
		j = 0;
		i++;
		memcpy(tmp,"\0",32);
	} */
}

int		
SamuraiServer::lookup (char * command)
{
	int   i = 0;

	build_lookup();		// build the language catalogue
	
	while (strcmp(tbl[i],"\0")!= 0) 
	{
		if (strcmp(tbl[i], command) == 0)
			return i;
		i ++;
	}
	return (-32);
}