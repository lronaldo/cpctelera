#CC=i386-pc-linux-gcc
CC=gcc

ifdef IPF_LIB_SUPPORT
    CFLAGS=-O3 -I . -I common/ -I common/includes -I common/ftdi_floppyemulator -DIPF_SUPPORT
    DISKLOADERPLUGINS_IPF= ipf_loader.o libcapsimage.so.2.0
else
    CFLAGS=-O3 -I . -I common/ -I common/includes -I common/ftdi_floppyemulator
    DISKLOADERPLUGINS_IPF=
endif

LDFLAGS=-lc -lm  -ldl  -lpthread
EXEC=hxcfloppyemulator_convert

ADFLIBOBJ= adf_hd.o adf_disk.o adf_raw.o adf_bitm.o adf_dump.o\
        adf_util.o adf_env.o adf_nativ.o adf_dir.o adf_file.o adf_cache.o \
        adf_link.o adf_salv.o

XDMSLIBOBJ= getbits.o crc_csum.o maketbl.o pfile.o getbits.o tables.o u_deep.o u_heavy.o u_init.o u_medium.o u_quick.o u_rle.o

ZLIBOBJ=adler32.o compress.o crc32.o deflate.o gzlib.o gzread.o gzwrite.o gzclose.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

MINIZIPOBJ=ioapi.o mztools.o unzip.o zip.o

FTDIFLOPPYEMU=usb_hxcfloppyemulator.o variablebitrate.o

DISKLOADERPLUGINS=copyqm_loader.o crctable.o adf_loader.o adz_loader.o amigadosfs_loader.o vfile.o cpcdsk_loader.o \
                  msa_loader.o msx_loader.o smc_loader.o st_loader.o stx_loader.o img_loader.o dms_loader.o mfm_loader.o\
                  oricdsk_loader.o d64_loader.o trd_loader.o scl_loader.o fat12floppy_loader.o fat12.o\
                  hfe_loader.o imd_loader.o afi_loader.o sap_loader.o libsap.o jv1_loader.o vtr_loader.o d88_loader.o hdm_loader.o\
                  ti99pc99_loader.o apridisk_loader.o ede_loader.o fd_loader.o vdk_loader.o dpx_loader.o mgt_loader.o emax_loader.o\
                  ensoniq_mirage_loader.o sad_loader.o jv3_loader.o stt_loader.o prophet_loader.o teledisk_loader.o td0_lzss.o\
                  emuii_raw_loader.o emuii_loader.o emui_raw_loader.o jvc_loader.o dim_loader.o ti99v9t9_loader.o d81_loader.o  acornadf_loader.o \
                  camputerslynxldf_loader.o vegasdsk_loader.o dmk_loader.o oldextadf_loader.o extadf_loader.o fdi_loader.o ssd_dsd_loader.o adl_loader.o \
                  krz_loader.o w30_loader.o fei_loader.o svd_loader.o gkh_loader.o imz_loader.o


all: $(EXEC)

hxcfloppyemulator_convert:      hxcfloppyemu_convert.o floppy_loader.o floppy_utils.o linux_api.o emuii_track.o crc.o track_generator.o gcr_track.o mfm_file_writer.o hfe_file_writer.o afi_file_writer.o cpcdsk_file_writer.o raw_file_writer.o sector_extractor.o  $(ADFLIBOBJ) $(DISKLOADERPLUGINS) $(DISKLOADERPLUGINS_IPF) $(XDMSLIBOBJ) $(ZLIBOBJ) $(MINIZIPOBJ)
	$(CC) -o $@    $^ $(LDFLAGS) -DFTDILIB

hxcfloppyemulator_d2xx: hxcfloppyemu.o floppy_loader.o floppy_utils.o linux_api.o ftdi_d2xxmode.o emuii_track.o crc.o track_generator.o gcr_track.o mfm_file_writer.o hfe_file_writer.o afi_file_writer.o cpcdsk_file_writer.o sector_extractor.o  $(ADFLIBOBJ) $(DISKLOADERPLUGINS) $(DISKLOADERPLUGINS_IPF) $(XDMSLIBOBJ) $(ZLIBOBJ) $(MINIZIPOBJ) $(FTDIFLOPPYEMU) libftd2xx.a.0.4.13
	$(CC) -o $@    $^ $(LDFLAGS)

hxcfloppyemulator:      hxcfloppyemu.o floppy_loader.o floppy_utils.o linux_api.o ftdi_libftdimode.o emuii_track.o crc.o track_generator.o gcr_track.o mfm_file_writer.o hfe_file_writer.o afi_file_writer.o cpcdsk_file_writer.o sector_extractor.o  $(ADFLIBOBJ) $(DISKLOADERPLUGINS) $(DISKLOADERPLUGINS_IPF) $(XDMSLIBOBJ) $(ZLIBOBJ) $(MINIZIPOBJ) $(FTDIFLOPPYEMU) /usr/local/lib/libftdi.so.1
	$(CC) -o $@    $^ $(LDFLAGS) -DFTDILIB

hxcfloppyemu_convert.o: hxcfloppyemu_convert.c
	$(CC) -o $@ -c $< $(CFLAGS)

hxcfloppyemu.o: hxcfloppyemu.c
	$(CC) -o $@ -c $< $(CFLAGS)

floppy_loader.o: common/floppy_loader.c
	$(CC) -o $@ -c $< $(CFLAGS)

floppy_utils.o: common/floppy_utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

linux_api.o: linux_api.c
	$(CC) -o $@ -c $< $(CFLAGS)

usb_hxcfloppyemulator.o : common/usb_floppyemulator/usb_hxcfloppyemulator.c
	$(CC) -o $@ -c $< $(CFLAGS)

variablebitrate.o : common/usb_floppyemulator/variablebitrate.c
	$(CC) -o $@ -c $< $(CFLAGS)

mfm_file_writer.o : common/mfm_file_writer.c common/mfm_file_writer.h
	$(CC) -o $@ -c $< $(CFLAGS)

hfe_file_writer.o : common/hfe_file_writer.c common/hfe_file_writer.h
	$(CC) -o $@ -c $< $(CFLAGS)

raw_file_writer.o : common/raw_file_writer.c common/raw_file_writer.h
	$(CC) -o $@ -c $< $(CFLAGS)

afi_file_writer.o : common/afi_file_writer.c common/afi_file_writer.h
	$(CC) -o $@ -c $< $(CFLAGS)

cpcdsk_file_writer.o : common/cpcdsk_file_writer.c common/cpcdsk_file_writer.h
	$(CC) -o $@ -c $< $(CFLAGS)

sector_extractor.o : common/sector_extractor.c common/sector_extractor.h
	$(CC) -o $@ -c $< $(CFLAGS)

emuii_track.o: common/plugins/common/emuii_track.c common/plugins/common/emuii_track.h
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: common/plugins/common/crc.c common/plugins/common/crc.h
	$(CC) -o $@ -c $< $(CFLAGS)

track_generator.o : common/plugins/common/track_generator.c common/plugins/common/track_generator.h
	$(CC) -o $@ -c $< $(CFLAGS)

gcr_track.o : common/plugins/common/gcr_track.c common/plugins/common/gcr_track.h
	$(CC) -o $@ -c $< $(CFLAGS)

apridisk_loader.o : common/plugins/apridisk_loader/apridisk_loader.c common/plugins/apridisk_loader/apridisk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ede_loader.o : common/plugins/ede_loader/ede_loader.c common/plugins/ede_loader/ede_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

gkh_loader.o : common/plugins/gkh_loader/gkh_loader.c common/plugins/gkh_loader/gkh_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ti99pc99_loader.o : common/plugins/ti99pc99_loader/ti99pc99_loader.c common/plugins/ti99pc99_loader/ti99pc99_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ti99v9t9_loader.o : common/plugins/ti99v9t9_loader/ti99v9t9_loader.c common/plugins/ti99v9t9_loader/ti99v9t9_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

imd_loader.o : common/plugins/imd_loader/imd_loader.c common/plugins/imd_loader/imd_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

hfe_loader.o : common/plugins/hfe_loader/hfe_loader.c common/plugins/hfe_loader/hfe_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

copyqm_loader.o : common/plugins/copyqm_loader/copyqm_loader.c  common/plugins/copyqm_loader/crctable.c common/plugins/copyqm_loader/copyqm_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

crctable.o : common/plugins/copyqm_loader/crctable.c   common/plugins/copyqm_loader/crctable.h
	$(CC) -o $@ -c $< $(CFLAGS)

adf_loader.o : common/plugins/adf_loader/adf_loader.c common/plugins/adf_loader/adf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

extadf_loader.o : common/plugins/extadf_loader/extadf_loader.c common/plugins/extadf_loader/extadf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

oldextadf_loader.o : common/plugins/oldextadf_loader/oldextadf_loader.c common/plugins/oldextadf_loader/oldextadf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

acornadf_loader.o : common/plugins/acornadf_loader/acornadf_loader.c common/plugins/acornadf_loader/acornadf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

cpcdsk_loader.o : common/plugins/cpcdsk_loader/cpcdsk_loader.c common/plugins/cpcdsk_loader/cpcdsk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

msa_loader.o : common/plugins/msa_loader/msa_loader.c common/plugins/msa_loader/msa_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

msx_loader.o : common/plugins/msx_loader/msx_loader.c common/plugins/msx_loader/msx_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

st_loader.o : common/plugins/st_loader/st_loader.c common/plugins/st_loader/st_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

dim_loader.o : common/plugins/dim_loader/dim_loader.c common/plugins/dim_loader/dim_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

stt_loader.o : common/plugins/stt_loader/stt_loader.c common/plugins/stt_loader/stt_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

stx_loader.o : common/plugins/stx_loader/stx_loader.c common/plugins/stx_loader/stx_loader.h common/plugins/stx_loader/pasti_format.h
	$(CC) -o $@ -c $< $(CFLAGS)

img_loader.o : common/plugins/img_loader/img_loader.c common/plugins/img_loader/img_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

imz_loader.o : common/plugins/imz_loader/imz_loader.c common/plugins/imz_loader/imz_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

d64_loader.o : common/plugins/d64_loader/d64_loader.c common/plugins/d64_loader/d64_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

d81_loader.o : common/plugins/d81_loader/d81_loader.c common/plugins/d81_loader/d81_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

d88_loader.o : common/plugins/d88_loader/d88_loader.c common/plugins/d88_loader/d88_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

trd_loader.o : common/plugins/trd_loader/trd_loader.c common/plugins/trd_loader/trd_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

scl_loader.o : common/plugins/scl_loader/scl_loader.c common/plugins/scl_loader/scl_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

mfm_loader.o : common/plugins/mfm_loader/mfm_loader.c common/plugins/mfm_loader/mfm_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

svd_loader.o : common/plugins/svd_loader/svd_loader.c common/plugins/svd_loader/svd_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

smc_loader.o : common/plugins/smc_loader/snes_smc_loader.c common/plugins/smc_loader/snes_smc_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

krz_loader.o : common/plugins/krz_loader/krz_loader.c common/plugins/krz_loader/krz_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

camputerslynxldf_loader.o : common/plugins/camputerslynxldf_loader/camputerslynxldf_loader.c common/plugins/camputerslynxldf_loader/camputerslynxldf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

vegasdsk_loader.o : common/plugins/vegasdsk_loader/vegasdsk_loader.c common/plugins/vegasdsk_loader/vegasdsk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

jv1_loader.o : common/plugins/jv1_loader/jv1_loader.c common/plugins/jv1_loader/jv1_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

jv3_loader.o : common/plugins/jv3_loader/jv3_loader.c common/plugins/jv3_loader/jv3_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

dmk_loader.o : common/plugins/dmk_loader/dmk_loader.c common/plugins/dmk_loader/dmk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

sap_loader.o : common/plugins/sap_loader/sap_loader.c common/plugins/sap_loader/sap_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

fd_loader.o : common/plugins/fd_loader/fd_loader.c common/plugins/fd_loader/fd_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ipf_loader.o : common/plugins/ipf_loader/ipf_loader.c common/plugins/ipf_loader/ipf_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

fdi_loader.o : common/plugins/fdi_loader/fdi_loader.c common/plugins/fdi_loader/fdi_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

adl_loader.o : common/plugins/adl_loader/adl_loader.c common/plugins/adl_loader/adl_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ssd_dsd_loader.o : common/plugins/ssd_dsd_loader/ssd_dsd_loader.c common/plugins/ssd_dsd_loader/ssd_dsd_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

fat12floppy_loader.o : common/plugins/fat12floppy_loader/fat12floppy_loader.c common/plugins/fat12floppy_loader/fat12floppy_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)
fat12.o : common/plugins/fat12floppy_loader/fat12.c common/plugins/fat12floppy_loader/fat12.h
	$(CC) -o $@ -c $< $(CFLAGS)

oricdsk_loader.o : common/plugins/oricdsk_loader/oricdsk_loader.c common/plugins/oricdsk_loader/oricdsk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

afi_loader.o : common/plugins/afi_loader/afi_loader.c common/plugins/afi_loader/afi_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

amigadosfs_loader.o : common/plugins/amigadosfs_loader/amigadosfs_loader.c common/plugins/amigadosfs_loader/amigadosfs_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

dms_loader.o : common/plugins/dms_loader/dms_loader.c common/plugins/dms_loader/dms_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

mgt_loader.o : common/plugins/mgt_loader/mgt_loader.c common/plugins/mgt_loader/mgt_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

sad_loader.o : common/plugins/sad_loader/sad_loader.c common/plugins/sad_loader/sad_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

emax_loader.o : common/plugins/emax_loader/emax_loader.c common/plugins/emax_loader/emax_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

ensoniq_mirage_loader.o : common/plugins/ensoniq_mirage_loader/ensoniq_mirage_loader.c common/plugins/ensoniq_mirage_loader/ensoniq_mirage_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

w30_loader.o : common/plugins/w30_loader/w30_loader.c common/plugins/w30_loader/w30_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

prophet_loader.o : common/plugins/prophet_loader/prophet_loader.c common/plugins/prophet_loader/prophet_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

teledisk_loader.o : common/plugins/teledisk_loader/teledisk_loader.c common/plugins/teledisk_loader/teledisk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

td0_lzss.o : common/plugins/teledisk_loader/td0_lzss.c common/plugins/teledisk_loader/td0_lzss.h
	$(CC) -o $@ -c $< $(CFLAGS)

vtr_loader.o : common/plugins/vtr_loader/vtr_loader.c common/plugins/vtr_loader/vtr_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

vdk_loader.o : common/plugins/vdk_loader/vdk_loader.c common/plugins/vdk_loader/vdk_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

dpx_loader.o : common/plugins/dpx_loader/dpx_loader.c common/plugins/dpx_loader/dpx_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

jvc_loader.o : common/plugins/jvc_loader/jvc_loader.c common/plugins/jvc_loader/jvc_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

hdm_loader.o : common/plugins/hdm_loader/hdm_loader.c common/plugins/hdm_loader/hdm_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

emuii_loader.o : common/plugins/emuii_loader/emuii_loader.c common/plugins/emuii_loader/emuii_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

emuii_raw_loader.o : common/plugins/emuii_raw_loader/emuii_raw_loader.c common/plugins/emuii_raw_loader/emuii_raw_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

emui_raw_loader.o : common/plugins/emui_raw_loader/emui_raw_loader.c common/plugins/emui_raw_loader/emui_raw_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

fei_loader.o : common/plugins/fei_loader/fei_loader.c common/plugins/fei_loader/fei_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)


vfile.o : common/libs/xdms/vfile.c common/libs/xdms/vfile.h
	$(CC) -o $@ -c $< $(CFLAGS)

adz_loader.o : common/plugins/adz_loader/adz_loader.c common/plugins/adz_loader/adz_loader.h
	$(CC) -o $@ -c $< $(CFLAGS)

adf_hd.o : common/libs/adflib/Lib/adf_hd.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_disk.o : common/libs/adflib/Lib/adf_disk.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_raw.o : common/libs/adflib/Lib/adf_raw.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_bitm.o: common/libs/adflib/Lib/adf_bitm.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_dump.o: common/libs/adflib/Lib/adf_dump.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_util.o  : common/libs/adflib/Lib/adf_util.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_env.o :  common/libs/adflib/Lib/adf_env.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_nativ.o :common/libs/adflib/Lib/adf_nativ.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_dir.o : common/libs/adflib/Lib/adf_dir.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_file.o : common/libs/adflib/Lib/adf_file.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_cache.o : common/libs/adflib/Lib/adf_cache.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_link.o : common/libs/adflib/Lib/adf_link.c
	$(CC) -o $@ -c $< $(CFLAGS)
adf_salv.o : common/libs/adflib/Lib/adf_salv.c
	$(CC) -o $@ -c $< $(CFLAGS)


getbits.o : common/libs/xdms/xdms-1.3.2/src/getbits.c
	$(CC) -o $@ -c $< $(CFLAGS)
crc_csum.o : common/libs/xdms/xdms-1.3.2/src/crc_csum.c
	$(CC) -o $@ -c $< $(CFLAGS)
maketbl.o : common/libs/xdms/xdms-1.3.2/src/maketbl.c
	$(CC) -o $@ -c $< $(CFLAGS)
pfile.o : common/libs/xdms/xdms-1.3.2/src/pfile.c
	$(CC) -o $@ -c $< $(CFLAGS)
tables.o : common/libs/xdms/xdms-1.3.2/src/tables.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_deep.o : common/libs/xdms/xdms-1.3.2/src/u_deep.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_heavy.o : common/libs/xdms/xdms-1.3.2/src/u_heavy.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_init.o : common/libs/xdms/xdms-1.3.2/src/u_init.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_medium.o : common/libs/xdms/xdms-1.3.2/src/u_medium.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_quick.o : common/libs/xdms/xdms-1.3.2/src/u_quick.c
	$(CC) -o $@ -c $< $(CFLAGS)
u_rle.o : common/libs/xdms/xdms-1.3.2/src/u_rle.c
	$(CC) -o $@ -c $< $(CFLAGS)


adler32.o : common/libs/zlib/adler32.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
compress.o : common/libs/zlib/compress.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
crc32.o : common/libs/zlib/crc32.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
deflate.o : common/libs/zlib/deflate.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
gzlib.o : common/libs/zlib/gzlib.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
gzread.o : common/libs/zlib/gzread.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
gzwrite.o : common/libs/zlib/gzwrite.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
gzclose.o : common/libs/zlib/gzclose.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ

infback.o : common/libs/zlib/infback.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
inffast.o : common/libs/zlib/inffast.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
inflate.o : common/libs/zlib/inflate.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
inftrees.o : common/libs/zlib/inftrees.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
trees.o : common/libs/zlib/trees.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
uncompr.o : common/libs/zlib/uncompr.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ
zutil.o : common/libs/zlib/zutil.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNO_VIZ

ioapi.o : common/libs/zlib/contrib/minizip/ioapi.c
	$(CC) -o $@ -c $< $(CFLAGS) -DNOLONGFILESUPPORT
mztools.o : common/libs/zlib/contrib/minizip/mztools.c
	$(CC) -o $@ -c $< $(CFLAGS)
unzip.o : common/libs/zlib/contrib/minizip/unzip.c
	$(CC) -o $@ -c $< $(CFLAGS)
zip.o : common/libs/zlib/contrib/minizip/zip.c
	$(CC) -o $@ -c $< $(CFLAGS)



libsap.o : common/libs/libsap/libsap.c   common/libs/libsap/libsap.h
	$(CC) -o $@ -c $< $(CFLAGS)


ftdi_d2xxmode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi_libftdimode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS) -DFTDILIB


clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
