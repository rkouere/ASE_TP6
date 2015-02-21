/* ------------------------------
   $Id: dir.c 7085 2013-10-18 15:37:14Z marquet $
   ------------------------------------------------------------

   Directories manipulation.
   Based on the ifile library.    
   Philippe Marquet, october 2002

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "super.h"
#include "inode.h"
#include "tools.h"
#include "ifile.h"
#include "dir.h"
unsigned int min(int a, int b) {
  if(a > b)
    return b;
  else
    return a;
}

/* on est en gcc 99 et cette fonction n'éxiste pas dans 99... on a donc la notre !!! */
char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}
/*------------------------------
  Private entry manipulation.
  ------------------------------------------------------------*/

/* return a new entry index.
   the file descriptor must the one of an open directory */
static unsigned int
new_entry(file_desc_t *fd)
{
    struct entry_s entry;
    unsigned int ientry = 0; /* the new entry index */

    /* seek to begin of dir */
    seek2_ifile(fd, 0);

    /* look after a null inumber in an entry */
    while (read_ifile (fd, &entry, sizeof(struct entry_s)) != READ_EOF) {
	if (! entry.ent_inumber)
	    return ientry;
	ientry++;
    }

    /* need to append an entry in the dir */
    return ientry;
}

/* return the entry index or RETURN_FAILURE.
   the file descriptor must the one of an open directory */
static int
find_entry(file_desc_t *fd, const char *basename)
{
    struct entry_s entry;
    unsigned int ientry = 0; /* the entry index */

    /* seek to begin of dir */
    seek2_ifile(fd, 0);
    
    /* look after the right entry */
    while (read_ifile (fd, &entry, sizeof(struct entry_s)) != READ_EOF) {
	if (entry.ent_inumber
	    && ! strcmp(entry.ent_basename, basename))
	    return ientry;
	ientry++;
    }

    /* not found */
    return RETURN_FAILURE;
}

/*------------------------------
  Create and delete entry
  ------------------------------------------------------------*/
int
add_entry(unsigned int idir, unsigned int inumber, const char *basename)
{
    struct inode_s inode; 
    file_desc_t _fd, *fd = &_fd;
    struct entry_s entry; 
    unsigned int ientry = 0;
    int nbyte; 
    /* int status; */
    
    /* a directory inode? */
    if(DEBUG) {
      printDebug(FNNAME, "read inode");
    }
    read_inode(idir, &inode); 
    if (inode.in_type !=  FILE_DIRECTORY) 
	return RETURN_FAILURE;
    
    /* open the dir */
    if(DEBUG) {
      printDebug(FNNAME, "open ifile");
    }
    open_ifile(fd, idir);

    /* the new entry position in the file */
    if(DEBUG) {
      printDebug(FNNAME, "new entry");
    }
    ientry = new_entry(fd);
    if(DEBUG) {
      printDebug(FNNAME, "");
      printf("ientry = %d\n", ientry);
    }
    /* built the entry */
    entry.ent_inumber = inumber;
    strncpy(entry.ent_basename, basename, ENTRYMAXLENGTH);
    entry.ent_basename[ENTRYMAXLENGTH - 1] = 0;

    /* seek to the right position */
    seek2_ifile(fd, ientry * sizeof(struct entry_s));
    
    /* write the entry */
    nbyte = write_ifile(fd, &entry, sizeof(struct entry_s));
    
    /* done */
    close_ifile(fd); /* even in case of write failure */

    if (nbyte == sizeof(struct entry_s))
	return RETURN_SUCCESS;
    else
	return RETURN_FAILURE;
}

int
del_entry(unsigned int idir, const char *basename)
{
    struct inode_s inode; 
    file_desc_t _fd, *fd = &_fd;
    struct entry_s entry; 
    unsigned int ientry;
    int status;
    
    /* a directory inode? */
    read_inode(idir, &inode); 
    if (inode.in_type !=  FILE_DIRECTORY) 
	return RETURN_FAILURE;
    
    /* open the dir */
    open_ifile(fd, idir);

    /* the entry position in the file */
    status = find_entry(fd, basename);
    if (status == RETURN_FAILURE) {
	close_ifile(fd);
	return RETURN_FAILURE;
    }
    ientry = status; 

    /* built a null entry */
    memset(&entry, 0, sizeof(struct entry_s));
    
    /* seek to that entry */
    seek2_ifile(fd, ientry * sizeof(struct entry_s));

    /* delete the entry = write the null entry */
    status = write_ifile(fd, &entry, sizeof(struct entry_s));

    /* close, and report status */
    close_ifile(fd);
    return status;
}

/*------------------------------
  Looking after entries
  ------------------------------------------------------------*/

/* consider the directory of inumber idir. 
   search after an entry of name basename (which can not contain /). 
   return the inumber of the entry, 0 if no such entry or if idir is
   not a directory.
*/
unsigned int
inumber_of_basename(unsigned int idir, const char *basename)
{
    struct inode_s inode;
    file_desc_t _fd, *fd = &_fd;
    unsigned int ientry;
    struct entry_s entry;
    int status;
    
    /* a directory inode? */
    read_inode(idir, &inode);
    if (inode.in_type !=  FILE_DIRECTORY) 
	return 0;

    /* open the dir */
    open_ifile(fd, idir);

    /* the entry position in the file */
    status = find_entry(fd, basename);
    if (status == RETURN_FAILURE) 
	return 0;
    ientry = status; 

    /* seek to the right position */
    seek2_ifile(fd, ientry * sizeof(struct entry_s));
    
    /* read the entry */
    status = read_ifile(fd, &entry, sizeof(struct entry_s));
    close_ifile(fd); 

    /* the result */
    return entry.ent_inumber;
}

unsigned int
inumber_of_path(const char *pathname)
{
    unsigned int icurrent; 	/* the inumber of the current dir */
    
    /* an *absolute* pathname */
    if (*pathname != '/')
	return 0;

    /* start at root */
    icurrent = current_super.super_root;
    
    while (*pathname) {
	if (*pathname != '/') {
	    char basename[ENTRYMAXLENGTH];
	    char *pos;		/* the first / position */
	    int lg; 		/* the length of the first basename */
	    
	    /* length of the leading basename */
	    pos = strchr(pathname, '/');
	    lg = pos ? pos - pathname : strlen (pathname);

	    /* copy this leading basename to basename */
	    strncpy (basename, pathname, min(ENTRYMAXLENGTH, lg));
	    basename[min(ENTRYMAXLENGTH, lg)] = 0;

	    /* look after this basename in the directory.
	       this entry inumber is the new current */
	    icurrent = inumber_of_basename(icurrent, basename); 
	    if (! icurrent) 
		return 0;

	    /* skip the basename in pathname */
	    pathname += lg;

	    /* may end here */ 
	    if (! *pathname) break;
	}
	pathname++ ;
    }
    return icurrent ;
}

unsigned int
dinumber_of_path(const char *pathname, const char **basename)
{
    char *dirname = strdup(pathname);
    unsigned int idirname = 0; 
    struct inode_s inode; 

    /* an *absolute* pathname */
    if (*pathname != '/') 
	goto free;
    
    /* the last basename (there is at least a '/') */
    *basename = strrchr (pathname, '/');
    (*basename)++;
    
    /* the dirname stops at the last '/'. ugly isn't it! */
    *(dirname + ((*basename) - pathname)) = 0;
    
    /* the dirname inumber */
    idirname = inumber_of_path(dirname);
    if (! idirname)
	goto free; 

    /* is dirname a directory? */
    read_inode(idirname, &inode); 
    if (inode.in_type !=  FILE_DIRECTORY)
	idirname = 0; 

 free:
    /* free dirname strdup() */
    free(dirname); 
    
    return idirname;
}
