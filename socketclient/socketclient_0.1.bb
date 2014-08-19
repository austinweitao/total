DESCRIPTION = "This package contains the simple PM750 test program."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=3f40d7994397109285ec7b81fdeb3b58 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r1"

SRC_URI = "file://libsocket.c \ 
	   file://libsocket.h \
	   file://sbslog.c \
	   file://sbslog.h \
	   file://socket_client.c \
	   file://unsock.h \
	   file://unsock.c \
	   "

S = "${WORKDIR}"

do_compile() {
  ${CC} ${CFLAGS} -o socket_client socket_client.c sbslog.c unsock.c libsocket.c -lpthread
}

do_install() {
  install -d ${D}${bindir}
  install -m 0755 socket_client ${D}${bindir}
}
