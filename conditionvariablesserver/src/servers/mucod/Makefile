# Makefile for MUCOD server
PROG=   mucod
SRCS=   main.c 

DPADD+= ${LIBSYS}
LDADD+= -lsys -lminlib -ltimers -lcompat_minix

MAN=

BINDIR?= /usr/sbin
FILES=mucod.conf
FILESNAME=mucod
FILESDIR= /etc/system.conf.d

.include <minix.service.mk>
