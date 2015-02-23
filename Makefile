# $Id: Makefile,v 1.1 2002/12/06 14:25:18 marquet Exp $
##############################################################################

#multicoeur
#ROOTDIR=/home/m1/echallier/fac/m2/ASE++/ASE_TP5/lib_hd
# ROOTDIR=/lib_hd

#mahieddine
#ROOTDIR=/home/mahieddine/Documents/cours/M1S2/ASEa/libhardware
ROOTDIR=/home/mahieddine/Documents/cours/M1S2/ASEa/TP2/lib_hd

CC	= gcc
CFLAGS	= -Wall -ansi -pedantic
#nico
CFLAGS  += -g -m32
CFLAGS  += -std=iso9899:1999
CFLAGS += -pthread
# CFLAGS += -lpthread

LIBDIR  = -L$(ROOTDIR)/lib
INCDIR  = -I$(ROOTDIR)/include
LIBS    = -lhardware

###------------------------------
### Main targets 
###------------------------------------------------------------
BINARIES= multiCore
OBJECTS	= $(addsuffix .o,\
	hw sched)
# hw sched drive mbr vol super tools mount inode ifile file dir)



all: $(BINARIES) $(OBJECTS)

###------------------------------
### Binaries
###------------------------------------------------------------
if_cfile: if_cfile.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
if_nfile: if_nfile.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
if_dfile: if_dfile.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
if_pfile: if_pfile.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)

multiCore: multiCore.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)

mkhd: mkhd.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
dmps: dmps.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
frmt: frmt.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
vm: vm.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
super: super.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
mknfs: mknfs.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
dfs: dfs.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
test: test.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)
shell:shell.o  $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)


###------------------------------
### #include dependences 
###------------------------------------------------------------
# you may fillese lines with "make depend"
# if_cfile.o: if_cfile.c ifile.h inode.h tools.h mount.h
# if_dfile.o: if_dfile.c ifile.h inode.h tools.h mount.h
# if_nfile.o: if_nfile.c ifile.h inode.h tools.h mount.h
# if_pfile.o: if_pfile.c ifile.h inode.h tools.h mount.h

multiCore.o: multiCore.c hw.h sched.h
current.o: current.c current.h
dfs.o: dfs.c super.h mbr.h drive.h hw.h colors.h vol.h mount.h sched.h
dir.o: dir.c super.h mbr.h drive.h hw.h colors.h vol.h inode.h tools.h mount.h ifile.h dir.h sched.h
dmps.o: dmps.c drive.h hw.h
drive.o: drive.c drive.h hw.h tools.h sched.h

file.o: file.c mbr.h drive.h hw.h colors.h super.h vol.h ifile.h inode.h tools.h mount.h dir.h file.h sched.h
frmt.o: frmt.c drive.h hw.h colors.h sched.h
if_cfile.o: if_cfile.c ifile.h inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h sched.h
if_dfile.o: if_dfile.c ifile.h inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h sched.h
if_nfile.o: if_nfile.c ifile.h inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h sched.h

if_pfile.o: if_pfile.c ifile.h inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h sched.h
ifile.o: ifile.c inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h ifile.h sched.h
inode.o: inode.c inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h mount.h ifile.h sched.h
mbr.o: mbr.c mbr.h drive.h hw.h colors.h mbr.h sched.h
mkhd.o:mkhd.c

mknfs.o: mknfs.c super.h mbr.h drive.h hw.h colors.h vol.h mount.h tools.h sched.h
mkvol.o: mkvol.c mkvol.h mbr.h drive.h hw.h colors.h sched.h
# mount.o: mount.c config.h tools.h mount.h mbr.h drive.h hw.h colors.h super.h vol.h sched.h
shell.o: shell.c drive.h hw.h mbr.h colors.h mount.h file.h tools.h inode.h super.h vol.h dir.h ifile.h sched.h
super.o: super.c super.h mbr.h drive.h hw.h colors.h vol.h ifile.h sched.h

test.o: test.c mount.h inode.h tools.h hw.h super.h mbr.h drive.h colors.h vol.h sched.h
tools.o: tools.c tools.h sched.h
vm.o: vm.c drive.h hw.h mbr.h colors.h mount.h sched.h
vol.o: vol.c vol.h drive.h hw.h mbr.h colors.h sched.h
sched.o: sched.c sched.h hw.h
hw.o:hw.c hw.h


%.o: %.c
	$(CC) $(CFLAGS) -c $< $(INCDIR)

###------------------------------
### Misc.
###------------------------------------------------------------
.PHONY: clean realclean depend
clean:
	$(RM) *.o $(BINARIES)
realclean: clean 
	$(RM) vdiskA.bin vdiskB.bin
totalclean: realclean 
	$(RM) *.*~

depend : 
	$(CC) -MM $(INCDIR) *.c
