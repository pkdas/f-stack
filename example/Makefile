TOPDIR=..

ifeq ($(FF_PATH),)
	FF_PATH=${TOPDIR}
endif

ifeq ($(FF_DPDK),)
	FF_DPDK=${TOPDIR}/dpdk/x86_64-native-linuxapp-gcc
endif

LIBS+= -L${FF_PATH}/lib -Wl,--whole-archive,-lfstack,--no-whole-archive
LIBS+= -L${FF_DPDK}/lib -Wl,--whole-archive,-ldpdk,--no-whole-archive
LIBS+= -Wl,--no-whole-archive -lrt -lm -ldl -lcrypto -pthread -lnuma

TARGET="helloworld"
all:
	cc -O0 -gdwarf-2  -g3 -Wno-format-truncation -I../lib -o ${TARGET} main.c ${LIBS}
	cc -O0 -gdwarf-2 -g3 -Wno-format-truncation  -I../lib -I../freebsd/netinet -o ${TARGET}_epoll main_epoll.c ${LIBS}
	cc -O0 -gdwarf-2 -g3 -Wno-format-truncation  -I../lib -o udp_epoll udp_epoll.c ${LIBS}

.PHONY: clean
clean:
	rm -f *.o ${TARGET} ${TARGET}_epoll
