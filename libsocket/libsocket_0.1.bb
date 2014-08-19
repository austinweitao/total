DESCRIPTION = "This package contains the simple PM750 test program."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=3f40d7994397109285ec7b81fdeb3b58 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r1"

SRC_URI = "file://libsocket.c \
	   file://sbslog.c  \
	   file://sbslog.h  \
	   file://unsock.c  \
	   file://unsock.h  \
	   file://libsocket.h  \
          "

S = "${WORKDIR}"

do_compile() {
  ${CC} ${CFLAGS} --shared -o libsocket.so libsocket.c unsock.c sbslog.c -lpthread
}

do_install() {
  install -d ${D}/${libdir}
  install -m 0755 libsocket.so ${D}/${libdir}
}
FILES_${PN} = "${libdir}/libsocket.so"
