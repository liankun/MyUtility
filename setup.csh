source /opt/phenix/bin/phenix_setup.csh -n new

unsetenv MYINSTALL
unsetenv MYUTILITY

mkdir -p install
setenv MYINSTALL `pwd`/install
setenv LD_LIBRARY_PATH $MYINSTALL/lib:${LD_LIBRARY_PATH}
setenv PATH $MYINSTALL/bin:${PATH}

#setenv TSEARCHPATH $MYINSTALL

