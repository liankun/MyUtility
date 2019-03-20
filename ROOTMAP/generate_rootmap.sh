#/bin/bash
#root map for MyUtility
#rlibmap -o libMyUtility.rootmap -l libMyUtility.so -d libCore.so libSpectrum.so libuspin.so libmpcex_base.so libmpcex_interface.so libmpcex_modules.so libmpcex_utils.so libMpcExReco.so -c ../MyUtilityLinkDef.h

#root map for mpcex
#rm -f libmpcex_interface.rootmap
#rlibmap -o libmpcex_interface.rootmap -l libmpcex_interface.so -d libMpcEx.so libmpcex_base.so libWrappers.so libgea.so libPgCal.so libphflag.so libfun4all.so libheader.so -c ../offline_src/mpcexcode/MpcExInterfaceLinkDef.h

#root map for mpc
#rlibmap -o libmpc.rootmap -l libmpc.so -d libpdbcalBase.so libnanoDST.so libphflag.so libphool.so libphtime.so libPISARoot.so libSubsysReco.so libtrigger.so libvtx.so libgea.so libbbc.so libPHPythia.so libPHHijing.so -c ../offline_src/mpc/mpcLinkDef.h

#root map for phwrappers
rlibmap -o libWrappers.rootmap -l libWrappers.so -d libCore.so libCint.so libRIO.so libNet.so libHist.so libGraf.so libGraf3d.so libGpad.so libTree.so libRint.so libPostscript.so libMatrix.so libPhysics.so libMathCore.so libThread.so -c ../offline_src/framework/phwrappers/PHTable_LinkDef.h
