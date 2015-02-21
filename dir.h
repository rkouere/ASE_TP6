/* ------------------------------
   $Id: dir.h 7085 2013-10-18 15:37:14Z marquet $
   ------------------------------------------------------------

   Directory manipulation.
   Philippe Marquet, november 2002

*/

#ifndef _DIR_H_
#define _DIR_H_

#define ENTRYMAXLENGTH 	16

/* return the inumber of an absolute pathname. 
   0 if not a valid pathname */
unsigned int inumber_of_path(const char *pathname);

/* pathname is an absolute pathname.
   return the inumber of the pathname dirname and
   return in basename the last entry of the pathname (basename will be
   a pointer to a char in pathname).
   return 0 if the directory does not exist. */
unsigned int dinumber_of_path(const char *pathname, const char **basename);

/* consider the directory of inumber idir. 
   search after an entry of name basename (which can not contain /). 
   return the inumber of the entry, 0 if no such entry or if idir is
   not a directory.
*/
unsigned int inumber_of_basename(unsigned int idir, const char *basename);

/* add/del an entry in the directory idir */
int add_entry(unsigned int idir, unsigned int inumber, const char *basename);
int del_entry(unsigned int idir, const char *basename);

/* An entry. A null inumber indicates a free entry.
   Export the structure definition on order to be able to read/write
   dir. Use at your own risk.*/
struct entry_s {
    unsigned int ent_inumber;
    char ent_basename[ENTRYMAXLENGTH]; 
}; 

#endif
