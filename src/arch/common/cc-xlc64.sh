COMMENT="Enable 64-bit mode (-q64)"
CMK_CC='xlc_r -q64 '
CMK_CXX='xlC_r -q64 -qstaticinline '
CMK_C_OPTIMIZE='-O3 -qstrict -Q!  '
CMK_CXX_OPTIMIZE='-O3 -qstrict -Q! '
CMK_CC64=true
CMK_PIC='-qpic=small'

CMK_LD="$CMK_CC "
CMK_LDXX="$CMK_CXX "
if test "$isAIX" = true
then
  CMK_LD="$CMK_LD -brtl"
  CMK_LDXX="$CMK_LDXX -brtl"
fi

CMK_SEQ_CC='xlc_r -q64'
CMK_SEQ_LD='xlc_r -q64'
CMK_SEQ_CXX='xlC_r -qstaticinline -q64'
CMK_SEQ_LDXX='xlC_r -q64'

CMK_NATIVE_CC='xlc_r -q64'
CMK_NATIVE_LD='xlc_r -q64'
CMK_NATIVE_CXX='xlC_r -qstaticinline -q64'
CMK_NATIVE_LDXX='xlC_r -q64'

CMK_CF77='xlf_r -q64 '
CMK_CF90='xlf90_r -q64 -qsuffix=f=f90' 
CMK_CF90_FIXED='xlf90_r -q64 ' 

if test "$isAIX" = "true"
then
  AR_OPTS="-X 64"
fi
CMK_AR="ar $AR_OPTS cq"
CMK_NM="nm $AR_OPTS"
