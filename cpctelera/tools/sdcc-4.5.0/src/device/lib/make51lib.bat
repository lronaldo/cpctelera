setlocal
set AR=ar
set RANLIB=sdranlib

set REL_FLOAT=_atof.rel _schar2fs.rel _sint2fs.rel _slong2fs.rel _uchar2fs.rel _uint2fs.rel _ulong2fs.rel _fs2schar.rel _fs2sint.rel _fs2slong.rel _fs2uchar.rel _fs2uint.rel _fs2ulong.rel _fsadd.rel _fsdiv.rel _fsmul.rel _fssub.rel _fseq.rel _fsgt.rel _fslt.rel _fsneq.rel fabsf.rel frexpf.rel ldexpf.rel expf.rel powf.rel sincosf.rel sinf.rel cosf.rel logf.rel log10f.rel sqrtf.rel tancotf.rel tanf.rel cotf.rel asincosf.rel asinf.rel acosf.rel atanf.rel atan2f.rel sincoshf.rel sinhf.rel coshf.rel tanhf.rel floorf.rel ceilf.rel modff.rel errno.rel
set REL_LONG=_divslong.rel _modslong.rel _modulong.rel
set REL_SDCC=atoi.rel atol.rel atoll.rel abs.rel labs.rel rand.rel _iscntrl.rel _isdigit.rel _isgraph.rel _isprint.rel _ispunct.rel _isspace.rel _islower.rel _isupper.rel _isxdigit.rel _strcat.rel _strchr.rel _strcmp.rel _strcpy.rel _strcspn.rel _strncat.rel _strncmp.rel _strncpy.rel _strpbrk.rel _strrchr.rel _strspn.rel _strstr.rel _strtok.rel _memchr.rel _memcmp.rel _memcpy.rel _memset.rel _calloc.rel _malloc.rel _realloc.rel _free.rel printf_large.rel puts.rel gets.rel assert.rel time.rel

set REL_FLOAT=%REL_FLOAT% _fscmp.rel _fsget1arg.rel _fsget2args.rel _fsnormalize.rel _fsreturnval.rel _fsrshift.rel _fsswapargs.rel _logexpf.rel isnan.c isinf.c
set REL_INT=%REL_INT% _divsint.rel _divuint.rel _modsint.rel _moduint.rel _mulint.rel
set REL_LONG=%REL_LONG% _divulong.rel _mullong.rel
set REL_SDCC=%REL_SDCC% _autobaud.rel _bp.rel _decdptr.rel _gptrget.rel _gptrgetc.rel _gptrput.rel _ser.rel _setjmp.rel serial.rel _itoa.rel _ltoa.rel _spx.rel _startup.rel _strlen.rel _memmove.rel _heap.rel sprintf.rel vprintf.rel printf_fast.rel printf_fast_f.rel printf_tiny.rel printfl.rel bpx.rel

set MODELS=small medium large

for %%M in (%MODELS%) do (
  pushd %%M
  del *.lib
  set SDCCLIB_CC=sdcc --use-stdout --model-%%M -c
  %AR% -S -cq libfloat.lib %REL_FLOAT%
  %RANLIB% libfloat.lib
  %AR% -S -cq libint.lib %REL_INT%
  %RANLIB% libint.lib
  %AR% -S -cq liblong.lib %REL_LONG%
  %RANLIB% liblong.lib
  %AR% -S -cq libsdcc.lib %REL_SDCC%
  %RANLIB% libsdcc.lib
  cd ..\mcs51
  for %%I in (*.asm) do sdas8051 -plosgff %%I
  for %%I in (*.rel) do %AR% -S -cq ..\%%M\mcs51.lib %%I
  %RANLIB% ..\%%M\mcs51.lib
  popd
)
endlocal
