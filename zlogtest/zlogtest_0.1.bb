DESCRIPTION = "This package contains the simple PM750 test program."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=3f40d7994397109285ec7b81fdeb3b58 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r1"

SRC_URI = "file://zlog_test.c \
	   file://zlog.h \
	  "

S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} -o zlog_test zlog_test.c -lzlog
    
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 zlog_test ${D}${bindir}
}
