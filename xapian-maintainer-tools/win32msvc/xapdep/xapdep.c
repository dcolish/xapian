/** @file xapdep.c
 * @brief File converter for Win32 MSVC dependency information
 */
/* Copyright (C) 2010 Lemur Consulting Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */
/* Replacement for makedepend, which had several bugs. We read in a file 'deps.d', 
made by the MSVC compiler using the -showIncludes switch, reformat it and add its
 contents to the end of 'Makefile'  - we do a nasty in-place replacement */

#pragma warning(disable:4996)
#include <io.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define DEPFILE "deps.d"
#define MAKEFILE "Makefile"
#define BACKUP_MAKEFILE "Makefile.bak"
#define BUFSIZE	1024
#define ENDMAKSTRING "# DO NOT DELETE THIS LINE"
#define DISCARDSTRING "Generating Code..."
#define DISCARDSTRING2 "Compiling..."
/* no, of characters after 'Note: including file:' in dep file */
#define INSET 22 
/*  lines that don't match the above will have a colon at this position*/
#define CHECKPOS 4 

/* Ugly rename-in-place hack taken from Makedepend sources */
int myrename(char *from, char *to)
{
  int r=1;
  char buf[BUFSIZE];
  FILE *in, *out;
  in=fopen(from, "rb");
  if(in==0)
    return -1;
  out=fopen(to, "wb");
  if(out==0)
    return -1;
  while(!feof(in) && r>0)
    {
      r=fread(buf, 1, sizeof(buf), in);
      if(r>0)
		fwrite(buf, 1, r, out);
    }
  fclose(out);
  fclose(in);
  if(unlink(from) < 0)
	  fprintf(stderr, "\nXAPDEP could not delete %s : %s\n", from, strerror(errno));

  return 0;
}

int main(int argc, char *argv[])
{
	FILE *indep,*inmak,*outmak;
	char buf[BUFSIZE], depbuf[BUFSIZE];
	int ch, endch, wch;

    /* Open the files we'll need, renaming the old Makefile to a backup */
	indep=fopen(DEPFILE,"rb");
	if(indep==0)
	{
		fprintf(stderr, "\nXAPDEP could not read deps.d\n");
		return -1;
	}
	if(myrename(MAKEFILE,BACKUP_MAKEFILE)!=0)
		return -1;
	inmak=fopen(BACKUP_MAKEFILE,"rb");
	outmak=fopen(MAKEFILE,"wb");
	if((inmak==0)||(outmak==0))
	{
		fprintf(stderr, "\nXAPDEP could not use Makefile\n");
		return -1;
	}

    /* Move through the backup makefile, reading each string and writing it unmodified to the new makefile */
	while(!feof(inmak))
	{
		fgets(buf, sizeof(buf), inmak);
		fputs(buf,outmak);
		if((strncmp(buf,ENDMAKSTRING,strlen(ENDMAKSTRING))==0) && !feof(indep))
		{
            /* We've got to the marker string at the end of the makefile, start reading dependencies */
			fgets(buf, sizeof(buf), indep);
			fputs("\r\n\r\n#Automatically generated dependencies from XAPDEP follow:\r\n\r\n",outmak);
			while(!feof(indep))
			{
                /* check for other random strings the compiler emits sometimes */
                while ( (strcmp(buf, DISCARDSTRING)==0) || (strcmp(buf, DISCARDSTRING2)==0) )    
                {
                      if(feof(indep)) break;
                      else /* skip line */
                        fgets(buf, sizeof(buf), indep);
                }

				/* first line should be a .cc file, use this to generate the .obj file */
				ch=0; 
				while((buf[ch]!='.') && (ch!=strlen(buf)))
					putc(buf[ch++], outmak);
				fputs(".obj : ", outmak);
				while(!feof(indep))
				{
                    /* get all the dependencies */
					fgets(buf, sizeof(buf), indep);
                    
                    /* check for the end of the deps list */
					if(buf[CHECKPOS]!=':') 
						break;
                        
                    /* check for other random strings the compiler emits sometimes */
                    if ( (strcmp(buf, DISCARDSTRING)==0) || (strcmp(buf, DISCARDSTRING2)==0) )  
                        break;
                        
                    /* clean up the dependencies and write them to the makefile */
					ch=INSET;
					endch=strlen(buf);
                    wch=0;
                    /* skip space */
					while((ch < endch) && (buf[ch]!='\r') && (buf[ch]==' '))
						ch++;
					while((ch < endch) && (buf[ch]!='\r'))
						depbuf[wch++]=buf[ch++];
					depbuf[wch]=0;
					fprintf(outmak,"%s\r\n",depbuf);
				}
			}
		}
	}
	fclose(outmak);
	fclose(inmak);
	fclose(indep);
	return 0;
}
