/*
  mka encoder -- Encode wav + cue to single matroska audio file

  mkaenc.c

  Written by Nicolas "goldenear" Le Guen <goldenear@matroska.org>

  Distributed under the GPL
  see the file COPYING for details
  or visit http://www.gnu.org/copyleft/gpl.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

char *meversion = "1.0.0";
int debugflag = 0;
int quietflag = 0;
unsigned int i;
int *tmp;

int pause (){
  // Make a pause and wait for [Enter]
  int ch;
  printf ("Press [Enter] to continue\n");
  while ((ch = getchar()) != '\n' && ch != EOF);
  return 0;
}

int fexists (char *f) {
  // Verify if file exists
  FILE *checkfile;
  checkfile = fopen ( f,"r");
  if ( checkfile) {
    fclose ( checkfile);
    return 1;
  }
  else
    return 0;
}

int mkpath(char *path) {
  // Check/Create the path
  if (path[0] == '\\')
    chdir("\\");
  char *subdir;
  subdir = strtok( path,"\\");
  if (!quietflag)
   printf ("\nPreparing directory \"");
  while ( subdir) {
  if (!quietflag)
   printf ("%s\\", subdir);
    if (chdir( subdir)){
	  if (!mkdir( subdir)){
	    if (chdir( subdir)){
		  fprintf ( stderr,"\nError: Can't access directory \"%s\"", subdir);
		  pause();
		  return 1;
		}
	  }
	  else {
	    fprintf ( stderr,"\nError: Can't create directory \"%s\"", subdir);
		pause();
		return 1;
	  }
	}
    subdir = strtok( NULL,"\\");
  }
  if (!quietflag)
    printf ("\"\n");
  _getcwd (path, _MAX_DIR);
  strcat (path, "\\");
  return 0;
}

void mbinfo (){
  // Display informations about the software
  printf("\nmka encoder version %s - Encode wav + cue to single matroska audio file\n", meversion);
  printf("Copyright (C) 2004  Goldenear\n");
}
  
void helpmsg (){
  // Display a short help
  fprintf (stderr, "\n Usage: mkaenc -i <inputfile> [options]\n");
  fprintf (stderr, "\n To display all the options: mkaenc -h\n");
}

void gplmsg (){
  // Display licence information
  printf ("  \n");
  printf ("  This program is free software; you can redistribute it and/or modify\n");
  printf ("  it under the terms of the GNU General Public License as published by\n");
  printf ("  the Free Software Foundation; either version 2 of the License, or any\n");
  printf ("  later version.\n");
  printf ("  \n");
  printf ("  This program is distributed in the hope that it will be useful,\n");
  printf ("  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  printf ("  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  printf ("  GNU General Public License for more details.\n");
  printf ("  \n");
  printf ("  You should have received a copy of the GNU General Public License\n");
  printf ("  along with this program; if not, write to the Free Software\n");
  printf ("  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
  printf ("  \n");
}
  
int mbhelp (){
  // Display the full help
  printf ("\n Usage: mkaenc -i <inputfile> [options]\n");
  printf ("\n Optional parameters:\n");
  printf ("   -s <filename>  Source filename (%%o for EAC)\n");
  printf ("   -n <naming scheme>  By default \"/a - /t\"\n");
  printf ("   -p <parameters>  Optional mkvmerge parameters\n");
  printf ("   -r <command>  Run an external command after merging\n");
  printf ("   -e <encoder with full path>  By default \"C:\\Program Files\\TTA\\ttaenc.exe\"\n");
  printf ("   -o <encoder options>  By default \"-e\"\n");
  printf ("   -x <.extension>  Encoded file extension, by default \".tta\"\n");
  printf ("   \n");
  printf ("\n Tags parameters:\n");
  printf ("   -a <album artist>  Album artist\n");
  printf ("   -t <album title>  Album title\n");
  printf ("   -y <album date>  set DATE tag\n");
  printf ("   -g <album genre>  set GENRE tag\n");
  printf ("   -d <discID>  set DISCID tag\n");
  printf ("   -c <comment>  set COMMENT tag\n");
  printf ("   \n");
  printf ("\n General options:\n");
  printf ("   -m  Modify the CUE file (add tags to the CUE sheet)\n");
  printf ("   -f  Fix EAC CUE file extension\n");
  printf ("   -j  Join the CUE file as an attachment to the mka file\n");
  printf ("   -u  Unchange base filename (don't rename the files)\n");
  printf ("   -w  Don't delete WAV file after encoding\n");
  printf ("   -k  Don't delete encoded file after merging\n");
  printf ("   -b  Don't merge, only prepare the base files\n");
  printf ("   -q  Quiet mode\n");
  printf ("   -h  Display this help\n");
  printf ("   -v  Display version\n");
  printf ("   -l  Display licence informations\n");
  printf ("   \n");
  printf ("\n Please report bugs or comments to goldenear@matroska.org\n");
  printf ("   \n");
  return 0;
}


int main (int argc, char **argv) {

  // Initialize the main parameters
  int wavflag = 0;
  int keepflag = 0;
  int mergeflag = 1;
  int renflag = 1;
  int cueflag = 0;
  int eacflag = 0;
  int attachflag = 0;
  char *inputfile = NULL;
  char *basefile = NULL;
  char *encoder = "TTA";
  char *encoderopt = NULL;
  char *encoderext = NULL;
  char *newfilename = "/a - /t";
  char *mkvmergeopt = NULL;
  char *extcommand = NULL;
  char *albumartist = NULL;
  char *albumtitle = NULL;
  char *albumdate = NULL;
  char *albumgenre = NULL;
  char *albumdiscid = NULL;
  char *albumcomment = NULL;
  
  // Parse command line parameters
  if (argc < 2) {
    mbinfo();
    helpmsg();
	pause();
	return 1;
  }
  
  int mbopt;
  opterr = 0;

  while ((mbopt = getopt ( argc, argv, "hvlqwkbumfji:s:e:o:x:n:p:r:a:t:y:g:d:c:z")) != -1)
    switch ( mbopt) {
      case 'h':
        mbinfo();
		mbhelp();
		return 0;
        break;
      case 'l':
	    mbinfo();
        gplmsg();
		return 0;
        break;
		case 'v':
        printf ("\nmkaenc version %s\n", meversion);
		return 0;
        break;
      case 'q':
        quietflag = 1;
        break;
	  case 'w':
        wavflag = 1;
        break;
	  case 'k':
        keepflag = 1;
        break;
	  case 'b':
        mergeflag = 0;
        break;
	  case 'u':
        renflag = 0;
        break;
	  case 'f':
        eacflag = 1;
        break;
      case 'm':
        cueflag = 1;
        break;
	  case 'j':
        attachflag = 1;
        break;
	  case 'i':
        inputfile = optarg;
        break;
      case 's':
        basefile = optarg;
        break;
      case 'e':
        encoder = optarg;
        break;
      case 'o':
        encoderopt = optarg;
        break;
      case 'x':
        encoderext = optarg;
        break;
      case 'n':
        newfilename = optarg;
        break;
      case 'p':
        mkvmergeopt = optarg;
        break;
      case 'r':
        extcommand = optarg;
        break;
      case 'a':
        albumartist = optarg;
        break;
      case 't':
        albumtitle = optarg;
        break;
      case 'y':
        albumdate = optarg;
        break;
      case 'g':
        albumgenre = optarg;
        break;
      case 'd':
	    if (strlen ( optarg) > 1)
          albumdiscid = optarg;
        break;	
      case 'c':
        albumcomment = optarg;
        break;	
      case 'z':
	    debugflag = 1;
        break;	
      case '?':
        if (isprint ( optopt)){
          fprintf (stderr, "\nError: Invalid option `-%c'.\n", optopt);
		  helpmsg();
		}
        else {
          fprintf (stderr, "\nError: Invalid option character `\\x%x'.\n", optopt);
		  helpmsg();
		}
		pause();
        return 1;
      default:
	    return 1;
    }
    
  /* Start */

  if ( debugflag)
    quietflag = 0;
  if (!quietflag)
    mbinfo();
  
  /* Debug mode : display all the parameters */

  if (debugflag) {
    printf ("\nThe folowing parameters are set:\n");
	if (quietflag)
      printf (" quietflag is set\n");
	if (wavflag)
      printf (" wavflag is set\n");
	if (keepflag)
      printf (" keepflag is set\n");
	if (mergeflag)
      printf (" mergeflag is set\n");
	if (renflag)
      printf (" renflag is set\n");
	if (cueflag)
      printf (" cueflag is set\n");
	if (eacflag)
      printf (" eacflag is set\n");
  	if(inputfile)
      printf (" inputfile: %s\n", inputfile);
  	if(basefile)
      printf (" basefile: %s\n", basefile);
	if(encoder)
      printf (" encoder: %s\n", encoder);
  	if(encoderopt)
      printf (" encoderopt: %s\n", encoderopt);
  	if(encoderext)
      printf (" encoderext: %s\n", encoderext);
  	if(mkvmergeopt)
      printf (" mkvmergeopt: %s\n", mkvmergeopt);
    if(extcommand)
      printf (" extcommand: %s\n", extcommand);
  	if(albumartist)
      printf (" albumartist: %s\n", albumartist);
  	if(albumtitle)
      printf (" albumtitle: %s\n", albumtitle);
  	if(albumdate)
      printf (" albumdate: %s\n", albumdate);
  	if(albumgenre)
      printf (" albumgenre : %s\n", albumgenre);
  	if(albumdiscid)
      printf (" albumdiscid: %s\n", albumdiscid);
  	if(albumcomment)
      printf (" albumcomment: %s\n", albumcomment);
  }
	
  // Check if all basic options are set
 
  // Check inputfile
  if (!inputfile) {
    printf ("\nError: No input file specified!\n");
	helpmsg();
	pause();
	return 1;
  }
  
  // Set encoder
  int encflag = 1;
  if (!strcmp(encoder, "TTA")) {
	  encoder = "C:\\Program Files\\TTA\\ttaenc.exe";
      if (!encoderopt)
        encoderopt = "-e";
      if (!encoderext)
	    encoderext = ".tta";
  }
  else if (!strcmp(encoder, "FLAC")) {
      encoder = "C:\\Program Files\\FLAC\\flac.exe";
      if (!encoderopt)
	    encoderopt = "--best -V";
      if (!encoderext)
	    encoderext = ".flac";
  }
  else if (!strcmp(encoder, "WAV")) {
      encflag = 0;
	  encoder = "";
	  encoderopt = "";
	  encoderext = ".wav";
  }
  if (!encoderopt)
    encoderopt = "";
  if (!encoderext) {
  fprintf ( stderr,"\nError: Encoded file extension is not set!\n");
	pause();
	return 1;
  }
  
  // Generate additional strings
  
  // The path to the encoder
  int encoderpathlen;
  char *encoderpath;
  if (strrchr( encoder, '\\')){
    encoderpathlen = strrchr( encoder, '\\') - encoder + 1;
	if ((encoderpath = calloc( encoderpathlen, sizeof(char))))
	  strncat( encoderpath, encoder, encoderpathlen);
	else {
	  fprintf(stderr, "\nError: Out of memory for encoderpath!\n");
	  pause();
	  return 1;
	}
  }
  else {
  encoderpath = "";
  }
  if (debugflag)
    printf (" encoderpath: %s\n", encoderpath);

  // Find the encoder command name
  int encoderexelen = strlen( encoder) - strlen( encoderpath) + 1;
  char *encoderexe;
  if ((encoderexe = calloc( encoderexelen, sizeof(char)))){
    for (i = strlen( encoderpath); i < strlen( encoder); i++)
      encoderexe[i - strlen( encoderpath)] = encoder[i];
    if (debugflag)
      printf (" encoderexe: %s\n", encoderexe);
  }
  else {
	fprintf(stderr, "\nError: Out of memory for encoderexe!\n");
	pause();
	return 1;
  }
	
  // Path where the files are created
  int rippathlen;
  char *rippath;
  if (strrchr( inputfile, '\\')) {
    rippathlen = strrchr( inputfile, '\\') - inputfile + 1;
    if ((rippath = calloc( rippathlen, sizeof(char))))
	  strncat( rippath, inputfile, rippathlen);
	else {
	  fprintf(stderr, "\nError: Out of memory for rippath!\n");
	  pause();
	  return 1;
	}
  }
  else {
	rippath = "";
  }
  if (debugflag)
    printf (" rippath: %s\n", rippath);
	
  // The base file name
  if (!basefile) {
    int basefilelen = strlen( inputfile) - strlen( rippath) + 1;
    if ((basefile = calloc( basefilelen, sizeof(char)))){
      for (i = strlen( rippath); i <= strlen( inputfile); i++)
        basefile[i - strlen( rippath)] = inputfile[i];
    }
    else {
	  fprintf(stderr, "\nError: Out of memory for basefile!\n");
	  pause();
	  return 1;
    }
  }
  char *basefilename;
  basefilename = strtok( basefile, ".");
  
  // The CUE file name
  char cuefileext[10] = "";
  if (eacflag)
    strcat( cuefileext, ".mka.cue");
  else
    strcat( cuefileext, ".cue");
  char *cuefilename;
  if ((cuefilename = calloc( strlen( rippath) + strlen( basefilename) + strlen( cuefileext) + 1, sizeof(char)))){
    strcat( cuefilename, rippath);
    strcat( cuefilename, basefilename);
    strcat( cuefilename, cuefileext);
    if (debugflag)
      printf (" cuefilename: %s\n", cuefilename);
  }
  else {
	fprintf(stderr, "\nError: Out of memory for cuefilename!\n");
	pause();
	return 1;
  }
  
  // Set ARTIST and TITLE tags
  FILE *cuefile;
  cuefile = fopen( cuefilename,"r");
  if (!cuefile) {
    fprintf ( stderr,"\nUnable to open \" %s \".\n", cuefilename);
	pause();
    return 1;
  }
  char cueinfo[1024];
  if (!albumartist) {
    int q =0;
	fseek( cuefile, 0, SEEK_SET);
    while ((fgets( cueinfo,1024, cuefile) != NULL) && q != 1) {
      if (strstr( cueinfo,"PERFORMER \"")) {
        unsigned int startchr = strchr(cueinfo,'\"') - cueinfo + 1;
        unsigned int endchr = strrchr(cueinfo,'\"') - cueinfo;
	    if ((albumartist = calloc( endchr - startchr + 1, sizeof(char)))){
	      for ( i = startchr; i < endchr; i++)
	        albumartist[i - startchr] = cueinfo[i];
	    }
	    else {
	      fprintf(stderr, "\nError: Out of memory for albumartist0!\n");
		  pause();
	      return 1;
	    }
	    q = 1;
      }
	}
    printf (" albumartist: %s\n", albumartist);
  }
  if (!albumtitle) {
    int q =0;
	fseek( cuefile, 0, SEEK_SET);
    while ((fgets( cueinfo,1024, cuefile) != NULL) && q != 1) {
      if (strstr( cueinfo,"TITLE \"")) {
        unsigned int startchr = strchr(cueinfo,'\"') - cueinfo + 1;
        unsigned int endchr = strrchr(cueinfo,'\"') - cueinfo;
	    if ((albumtitle = calloc( endchr - startchr + 1, sizeof(char)))){
	      for ( i = startchr; i < endchr; i++)
	        albumtitle[i - startchr] = cueinfo[i];
	    }
	    else {
	      fprintf(stderr, "\nError: Out of memory for albumtitle!\n");
		  pause();
	      return 1;
	    }
	    q = 1;
      }
	}
    printf (" albumtitle: %s\n", albumtitle);
  }
  fclose( cuefile);
  
  if (!albumartist)
    albumartist = "";
  if (!albumtitle)
    albumtitle = "";
  
  if (!strcmp( albumartist, "various") || !strcmp( albumartist, "Various") || !strcmp( albumartist, "VARIOUS")) {
    if ((tmp = realloc( albumartist,16 * sizeof(char))) == NULL) {
      fprintf(stderr, "\nError: Out of memory for albumartist1!\n");
	  pause();
	  return 1;
	}
	else {
      albumartist = "Various Artists";
	}
  }
  
  // Reformat the base file name
  int newbasenamelen;
  char *newbasename;
  if (renflag) {
    newbasenamelen = 512;
	if ((newbasename = calloc( newbasenamelen, sizeof(char)))){
	  for (i = 0; i < strlen( newfilename); i++){
	    if (newfilename[i] != '/'){
		  if ((tmp = realloc( newbasename, (strlen( newbasename) + 2) * sizeof(char))) == NULL) {
           fprintf(stderr, "\nError: Out of memory for newbasename/albumartist0!\n");
			pause();
			return 1;
		  }
		  else {
            char tmpchr[] = {newfilename[i],'\0'};
            strcat ( newbasename, tmpchr);
		  }
        }
        else {
          i++;
          switch (newfilename[i]) {
		    case 'a':
		      if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumartist) + 1) * sizeof(char))) == NULL) {
                fprintf(stderr, "\nError: Out of memory for newbasename/albumartist1!\n");
				pause();
			    return 1;
			  }
			  else
			    strcat ( newbasename, albumartist);
	          break;
            case 't':
		      if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumtitle) + 1) * sizeof(char))) == NULL) {
			    fprintf(stderr, "\nError: Out of memory for newbasename/albumtitle!\n");
				pause();
			    return 1;
			  }
			  else
                strcat ( newbasename, albumtitle);
	          break;
            case 'y':
		      if (albumdate) {
			    if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumdate) + 1) * sizeof(char))) == NULL) {
			      fprintf(stderr, "\nError: Out of memory for newbasename/albumdate!\n");
				  pause();
				  return 1;
			    }
			    else
                  strcat ( newbasename, albumdate);
			  }
			  break;
            case 'g':
		      if (albumgenre) {
                if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumgenre) + 1) * sizeof(char))) == NULL) {
			      fprintf(stderr, "\nError: Out of memory for newbasename/albumgenre!\n");
				  pause();
				  return 1;
			    }
			    else
			      strcat ( newbasename, albumgenre);
			  }
			  break;
            case 'd':
		      if (albumdiscid) {
                if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumdiscid) + 1) * sizeof(char))) == NULL) {
			      fprintf(stderr, "\nError: Out of memory for newbasename/albumdiscid!\n");
				  pause();
				  return 1;
			    }
			    else
			      strcat ( newbasename, albumdiscid);
	          }
			  break;
            case 'c':
		      if (albumcomment) {
                if ((tmp = realloc( newbasename, (strlen( newbasename) + strlen( albumcomment) + 1) * sizeof(char))) == NULL) {
			      fprintf(stderr, "\nError: Out of memory for newbasename/albumcomment!\n");
			      pause();
				  return 1;
			    }
			    else
			      strcat ( newbasename, albumcomment);
			  }
	          break;
            default:
              strcat ( newbasename, "");
          }
        }
      }
	  if ((tmp = realloc( newbasename, (strlen( newbasename) + 1) * sizeof(char))) == NULL) {
	    fprintf(stderr, "\nError: Out of memory for newbasename0!\n");
		pause();
		return 1;
	  }
	  char badchr[] = "\"*/:<>\?|";
      for (i = 0; i < strlen( newbasename); i++){
        if (strchr( badchr, newbasename[i]))
	      if (i != 1 || newbasename[i] != ':')
            newbasename[i] = '_';
      }
      if (strlen( newbasename) < 1) {
	    if ((tmp = realloc( newbasename, (strlen( basefilename) + 1) * sizeof(char))) == NULL) {
		  fprintf(stderr, "\nError: Out of memory for newbasename0!\n");
		  pause();
		  return 1;
		}
		else
		  strcpy ( newbasename, basefilename);
	  }
	}
	else {
	  fprintf(stderr, "\nError: Out of memory for newbasename1!\n");
	  pause();
	  return 1;
	}
  }
  else {
    if (!(newbasename = calloc(strlen( basefilename) + 1, sizeof(char)))){
	  fprintf(stderr, "\nError: Out of memory for newbasename2!\n");
	  pause();
	  return 1;
	}
	else
	  strcpy ( newbasename, basefilename);
  }
  int mkapathlen;
  char *mkapath;
  if (strrchr( newbasename, '\\')) {
    mkapathlen = strrchr( newbasename, '\\') - newbasename + 1;
    if ((mkapath = calloc( mkapathlen, sizeof(char))))
	  strncat( mkapath, newbasename, mkapathlen);
	else {
	  fprintf(stderr, "\nError: Out of memory for mkapath!\n");
	  pause();
	  return 1;
	}
  }
  else {
	mkapath = "";
  }
  if (debugflag)
    printf (" mkapath: %s\n", mkapath);
  
  int newbasefilenamelen = strlen( newbasename) - strlen( mkapath) +1;
  char *newbasefilename;
  if ((newbasefilename = calloc( newbasefilenamelen, sizeof(char)))){
    for (i = strlen( mkapath); i <= strlen( newbasename); i++)
      newbasefilename[i - strlen( mkapath)] = newbasename[i];
  }
  else {
	fprintf(stderr, "\nError: Out of memory for newbasefilename!\n");
	pause();
	return 1;
  }
  
  free( newbasename);
  
  // Encoded file name
  int encodedfilenamelen = strlen( rippath) + strlen( newbasefilename) + strlen( encoderext) + 1;
  char *encodedfilename;
  if ((encodedfilename = calloc( encodedfilenamelen, sizeof(char)))){
    strcat( encodedfilename, rippath);
    strcat( encodedfilename, newbasefilename);
    strcat( encodedfilename, encoderext);
    if (debugflag)
      printf (" encodedfilename: %s\n", encodedfilename);
  }
  else {
	fprintf(stderr, "\nError: Out of memory for encodedfilename!\n");
	pause();
	return 1;
  }
  
  // Check if everythink is correctly set
  if (!fexists( inputfile)){
    fprintf ( stderr,"\nError: Unable to find inputfile: \"%s\".\n", inputfile);
	pause();
	return 1;
  }
  if (!fexists( cuefilename)){
    fprintf ( stderr,"\nError: Unable to find cuefile: \"%s\".\n", cuefilename);
	pause();
	return 1;
  }
  if (!fexists( encoder) && strlen( encoder) > 0){
    fprintf ( stderr,"\nError: Unable to find encoder: \"%s\".\n", encoder);
	pause();
	return 1;
  }
  if (!fexists( "mkvmerge.exe")){
    fprintf ( stderr,"\nError: Unable to find mkvmerge!\n");
	pause();
	return 1;
  }
  if (debugflag) {
    printf ("\nAll necessary files found: OK!\n");
	pause();
  }
  	
  // Create the destination path
  char runpath[_MAX_DIR]; 
  _getcwd (runpath, _MAX_DIR);
  if (strlen( mkapath) != 0)
    mkpath( mkapath);
  if (chdir( runpath)){
    fprintf ( stderr,"Error: Can't access to \"%s\"", runpath);
	pause();
	return 1;
  }
  
  // Process the CUE sheet
  if (debugflag) {
    printf ("\nThe CUE file \"%s\" will be processed ...\n", cuefilename);
	pause();
  }
  
  int newcuefilenamelen = strlen( rippath) + strlen( newbasefilename) + 5;
  char *newcuefilename;
  if ((newcuefilename = calloc( newcuefilenamelen, sizeof(char)))){
    strcat( newcuefilename, rippath);
    strcat( newcuefilename, newbasefilename);
    strcat( newcuefilename, ".cue");
  }
  else {
	fprintf(stderr, "\nError: Out of memory for newcuefilename!\n");
	pause();
	return 1;
  }
  
  // Open original CUE file and create the new one
  cuefile = fopen( cuefilename,"r");
  if (!cuefile) {
    fprintf ( stderr,"\nUnable to open \"%s\".\n", cuefilename);
	pause();
    return 1;
  }
  FILE *newcuefile;
  newcuefile = fopen( newcuefilename,"a+");
  if (!newcuefile) {
    fprintf ( stderr,"\nUnable to create \"%s\".\n", newcuefilename);
	pause();
    return 1;
  }
	
  // Write CUE informations to the new CUE file
  if (!quietflag)
    printf ("\nWriting modified CUE sheet: ");
  int q = 0;
  fseek( cuefile, 0, SEEK_SET);
  while ( fgets( cueinfo,1024, cuefile) != NULL ) {
    if (!strcmp( albumartist,"Various Artists") && strstr( cueinfo,"PERFORMER \"") && q !=1) {
	  printf ("PERFORMER \"Various Artists\"\n");
	  fprintf ( newcuefile,"PERFORMER \"Various Artists\"\n");
	  q = 1;
	}
    else if (strstr( cueinfo,"FILE \"")){
	  if (albumdate && cueflag)
        fprintf ( newcuefile,"REM DATE %s\n", albumdate);
	  if (albumgenre && cueflag)
        fprintf ( newcuefile,"REM GENRE %s\n", albumgenre);
	  if (albumdiscid && cueflag)
        fprintf ( newcuefile,"REM DISCID %s\n", albumdiscid);
	  if (albumcomment && cueflag)
        fprintf ( newcuefile,"REM COMMENT %s\n", albumcomment);
      fprintf ( newcuefile,"FILE \"%s - %s%s\" WAVE\n", albumartist, albumtitle, encoderext);
    }
    else
      fprintf ( newcuefile,"%s", cueinfo);
  }
  
  // Close the CUE files and delete the old one
  if (!fclose( cuefile) && !fclose( newcuefile)){
    if (!remove( cuefilename)){
	  if (!quietflag)
	    printf ("Done.\n");
    }
	else {
      fprintf ( stderr,"Error writing CUE!");
	  pause();
	  return 1;
    }
	free( cuefilename);
  }
  
  // Rename the wav file
  if (debugflag) {
    printf ("\nThe WAV file \"%s\" will be renamed ...\n", inputfile);
	pause();
  }
  if (!quietflag)
    printf ("\nRenaming wav file: ");
  
  int wavefilelen = strlen( rippath) + strlen( newbasefilename) + 5;
  char *wavfile;
  if ((wavfile = calloc( wavefilelen, sizeof(char)))){
    strcat( wavfile, rippath);
    strcat( wavfile, newbasefilename);
    strcat( wavfile, ".wav");
    if (rename( inputfile, wavfile)){
      fprintf ( stderr, "Error: Unable to rename the wav file!\n");
	  pause();
      return 1;
    }
    else
      if (!quietflag)
        printf ("Done, the WAV file is now \"%s\".\n", wavfile);
  }
  else {
	fprintf(stderr, "\nError: Out of memory for wavfile!\n");
	pause();
	return 1;
  }
  
  // Set the path
  if (encflag && strlen( encoderpath) > 0) {
    if (!quietflag)
      printf ("\nSetting the encoder path ...\n");
    char *envpath;
    envpath = getenv("Path");
    char *newpath;
    if ((newpath = calloc(strlen( envpath) + strlen (encoderpath) + 8, sizeof(char)))){
      strcat( newpath, "Path=");
      strcat( newpath, envpath);
      if (envpath[strlen( envpath) - 1] != ';')
        strcat( newpath, ";");
      strncat( newpath, encoderpath, strlen(encoderpath) - 1);
      free( encoderpath);
      putenv( newpath);
      if (debugflag)
        printf ("\n%s\n", newpath);
      free( newpath);
    }
    else {
	  fprintf(stderr, "\nError: Out of memory for newpath!\n");
	  pause();
	  return 1;
    }
  }
  
  // Encode using external encoder
  if (encflag) {
    if (chdir( rippath)){
      fprintf ( stderr,"Error: Can't access to \"%s\"", rippath);
      pause();
	  return 1;
    }
    
    if (!wavflag || mergeflag) {
      char *commandline;
	  if ((commandline = calloc( encoderexelen + strlen(encoderopt) + strlen( wavfile) + 5, sizeof(char)))){
        strcat( commandline, encoderexe);
        strcat( commandline, " ");
        strcat( commandline, encoderopt);
        strcat( commandline, " \"");
        strcat( commandline, wavfile);
        strcat( commandline, "\"");
        free( encoderexe);
        if (!quietflag)
          printf ("\nEncoding WAV file to \"%s%s\".\n", newbasefilename, encoderext);
        if (debugflag) {
          printf ("\nNext command-line to be executed:\n%s\n", commandline);
          pause();
        }
        if (!system ( commandline)) {
	      if (!quietflag)
            printf ("\nEncoding done.\n");
        }
	    else {
          fprintf ( stderr,"\nError: Unable to encode!\n");
	      pause();
          return 1;
	    }
        free( commandline);
      }
	  else {
	    fprintf(stderr, "\nError: Out of memory for commandline!\n");
		pause();
	    return 1;
	  }
    }
  
    if (chdir( runpath)) {
      fprintf ( stderr,"Error: Can't access to \"%s\"", runpath);
	  pause();
	  return 1;
    }
  }
  
  // Delete the WAV file
  if (!wavflag && encflag) {
    if (!quietflag)
      printf ("\nDeleting WAV file:\n");
	if (!quietflag)
      printf (" %s ", wavfile);
    if (!remove( wavfile)) {
	  if (!quietflag)
	    printf ("deleted.\n");
	}
	else
	  fprintf ( stderr,"\nWarning: Can't delete WAV file!\n");
  }
  free( wavfile);
  
  // Merge the files
  if (mergeflag) {
    if (!quietflag)
      printf ("\nStart merging to \"%s.mka\".\n\n", newbasefilename);
    int mergecmdlen;
    if (mkvmergeopt)
      mergecmdlen = strlen( rippath) + strlen( newbasefilename) + strlen( mkapath) + strlen( encodedfilename) + (strlen( newcuefilename) * 2) + strlen(mkvmergeopt) + 256;
    else
      mergecmdlen = strlen( rippath) + strlen( newbasefilename) + strlen( encodedfilename) + (strlen( newcuefilename) * 2) + 256;
    char *mergecmd;
	if ((mergecmd = calloc( mergecmdlen, sizeof(char)))){
      strcat( mergecmd, "mkvmerge -o \"");
      if (strlen( mkapath) == 0)
        strcat( mergecmd, rippath);
      else
        strcat( mergecmd, mkapath);
      strcat( mergecmd, newbasefilename);
      strcat( mergecmd,".mka\" --command-line-charset UTF-8 -D -S \"");
      strcat( mergecmd, encodedfilename);
	  if (!attachflag) {
        strcat( mergecmd,"\" --attachment-mime-type text/plain --attach-file \"");
        strcat( mergecmd, newcuefilename);
	  }
      strcat( mergecmd,"\" --chapters \"");
      strcat( mergecmd, newcuefilename);
      strcat( mergecmd,"\"");
      if (mkvmergeopt) {
        strcat( mergecmd," ");
        strcat( mergecmd, mkvmergeopt);
      }
      free( rippath);
      free( newbasefilename);
	  free( mkapath);
      if (debugflag) {
        printf ("\nNext command-line to be executed:\n%s\n", mergecmd);
        pause();
      }
      if (!system ( mergecmd))
        printf ("\nMerge done.\n");
      else {
        fprintf ( stderr,"\nError: Unable to merge!\n");
	    pause();
	    return 1;
      }
	  free( mergecmd);
    }
	else {
	  fprintf(stderr, "\nError: Out of memory for mergecmd!\n");
	  pause();
	  return 1;
	}
  }
  
  // Cleaning
  if (!quietflag)
    printf ("\nDeleting temporary files:\n");
	if (!keepflag || (!wavflag && !mergeflag)) {
	  if (!quietflag)
        printf (" %s ", encodedfilename);
      if (!remove( encodedfilename)) {
	    if (!quietflag)
	      printf ("deleted.\n");
	  }
	  else
	    fprintf ( stderr,"\nWarning: Can't delete encoded file!\n");
	}
    if (!keepflag || !mergeflag) {
	  if (!quietflag)
        printf (" %s ", newcuefilename);
	  if (!remove( newcuefilename)) {
	    if (!quietflag)
	      printf ("deleted.\n");
	  }
	  else
	    fprintf ( stderr,"\nWarning: Can't delete CUE file!\n");
    }
  free( encodedfilename);
  free( newcuefilename);
  
  // Run extarnal command
  if (extcommand) {
    if (!quietflag)
	  printf ("\nRunning external command ...\n");
	if (debugflag) {
	  printf ("%s\n", extcommand);
	  pause();
	}
	if (system ( extcommand)) {
      fprintf ( stderr,"\nExternal command error!\n");
	  pause();
	  return 1;
	}
  }
  
  // Terminating
  if (!quietflag)
    printf ("\nAll done!\n");
  if (debugflag)
    pause();
  return 0;
}
