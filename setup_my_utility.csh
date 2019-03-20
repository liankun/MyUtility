if (! $?MYUTILITY) then
  echo "MYUTILITY not exsit !"
  setenv MYUTILITY /gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MyUtility/install
  setenv LD_LIBRARY_PATH $MYUTILITY/lib:${LD_LIBRARY_PATH}
  setenv PATH $MYUTILITY/bin:${PATH}
else echo "MYUTILITY already exsit !"
endif
