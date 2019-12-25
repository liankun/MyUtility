AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -Wall -Werror
INCLUDES = -I$(includedir) -I$(OFFLINE_MAIN)/include -I`root-config --incdir`

SUBDIRS = 

lib_LTLIBRARIES = \
  libMyUtility.la 

noinst_HEADERS = \
  MyUtilityLinkDef.h 

dist_pkgdata_DATA = \
  minipad_mip_correction.txt \
  minipads_scale_smear_db.txt \
  neighbor_minipads_can_calibrate_low_gain.txt \
  minipads_scale_smear_include_low_gain_db.txt \
  minipads_scale_smear_include_low_gain_db_v2.txt \
  minipads_scale_smear_include_low_gain_db_v3.txt \
  minipads_scale_smear_include_low_gain_db_v4.txt

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -lTOAD

include_HEADERS = \
  ExEvent.h \
  ExEventV1.h \
  ExSimEvent.h \
  ExSimEventV1.h \
  ExPrimParticle.h \
  ExEmbedEvent.h \
  ExShowerV1.h \
  ExShowerV2.h \
  STFinder.h \
  STFinderV2.h \
  ExGeomToKey.h \
  mMpcExMyLoadCalibrations.h \
  mMpcExMyApplyCalibrations.h \
  ExAcpt.h \
  mMpcExMyEmbed.h \
  mMpcExMyDigitizeHits.h \
  ExSimEventV2.h \
  ExSimEventV3.h \
  mMpcExShowerV2.h \
  ExHitList.h \
  ExMiniClusters.h \
  MiniClusterV1.h \
  JohnCalibrateShowerEnergy.h \
  MTensor.h \
  MLayer.h \
  MConv.h \
  MPooling.h
 
libMyUtility_la_SOURCES = \
  ExEvent.cxx \
  ExEventV1.cxx \
  ExSimEvent.cxx \
  ExSimEventV1.cxx \
  ExPrimParticle.cxx \
  ExEmbedEvent.cxx \
  ExShowerV1.cxx \
  ExShowerV2.cxx \
  STFinder.cxx \
  STFinderV2.cxx \
  ExGeomToKey.cxx \
  mMpcExMyLoadCalibrations.cxx \
  mMpcExMyApplyCalibrations.cxx \
  ExAcpt.cxx \
  mMpcExMyEmbed.cxx \
  mMpcExMyDigitizeHits.cxx \
  ExHitList.cxx \
  ExSimEventV2.cxx \
  ExSimEventV3.cxx \
  mMpcExShowerV2.C \
  ExMiniClusters.cxx \
  MiniClusterV1.cxx \
  JohnCalibrateShowerEnergy.cxx \
  MTensor.cxx \
  MLayer.cxx \
  MConv.cxx \
  MPooling.cxx

libMyUtility_la_LIBADD = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(ROOTSYS)/lib \
  -lSpectrum \
  -luspin \
  -lmpcex_base \
  -lmpcex_interface \
  -lmpcex_modules \
  -lmpcex_utils \
  -lMpcExReco

nodist_libMyUtility_la_SOURCES = \
  MyUtility_Dict.C

# Rule for generating CINT dictionaries from class headers.
MyUtility_Dict.C: \
  ExEvent.h \
  ExEventV1.h \
  ExSimEvent.h \
  ExSimEventV1.h \
  ExPrimParticle.h \
  ExEmbedEvent.h \
  ExShowerV1.h \
  ExShowerV2.h \
  STFinder.h \
  STFinderV2.h \
  ExGeomToKey.h \
  mMpcExMyLoadCalibrations.h \
  mMpcExMyApplyCalibrations.h \
  mMpcExMyEmbed.h \
  ExAcpt.h \
  mMpcExMyDigitizeHits.h \
  ExSimEventV2.h \
  ExHitList.h \
  ExSimEventV3.h \
  mMpcExShowerV2.h \
  ExMiniClusters.h \
  MiniClusterV1.h \
  JohnCalibrateShowerEnergy.h \
  MTensor.h \
  MLayer.h \
  MConv.h \
  MPooling.h \
  MyUtilityLinkDef.h 
	rootcint -f $@ -c $(DEFAULT_INCLUDES) $(INCLUDES) $^

testexternals_SOURCES = \
  testexternals.C

BUILT_SOURCES = \
  ml_training_data_bdtg_arm0.h \
  ml_training_data_bdtg_arm0_inner.h \
  ml_training_data_bdtg_arm0_outer.h \
  ml_training_data_bdtg_arm0_E33_0.h \
  ml_training_data_bdtg_arm1.h \
  ml_training_data_bdtg_arm1_inner.h \
  ml_training_data_bdtg_arm1_outer.h \
  ml_training_data_bdtg_arm1_E33_0.h \
  ml_training_data_mlp_arm0.h \
  ml_training_data_mlp_arm0_inner.h \
  ml_training_data_mlp_arm0_outer.h \
  ml_training_data_mlp_arm0_E33_0.h \
  ml_training_data_mlp_arm1.h \
  ml_training_data_mlp_arm1_inner.h \
  ml_training_data_mlp_arm1_outer.h \
  ml_training_data_mlp_arm1_E33_0.h \
  ml_training_data_bdtg_mpcex_only.h \
  ml_training_data_bdtg_photon_frac.h \
  ml_training_data_bdtg_pi0_frac.h

noinst_PROGRAMS = \
  testexternals

testexternals_LDADD = \
  libMyUtility.la

testexternals.C:
	@echo '//*** this is a generated file. Do not commit, do not edit' > $@
	@echo 'int main() { }' >> $@

ml_training_data_bdtg_arm0.h:\
	TMVARegression_BDTG.weights.arm0.xml 
	cp $< .
	xxd -i $(<F) > temp1
	head -n -1 temp1 > $@
	rm $(<F)
	rm temp1

ml_training_data_bdtg_arm0_inner.h:\
	TMVARegression_BDTG.weights.arm0_inner.xml 
	cp $< .
	xxd -i $(<F) > temp2 
	head -n -1 temp2 > $@
	rm $(<F)
	rm temp2

ml_training_data_bdtg_arm0_outer.h:\
	TMVARegression_BDTG.weights.arm0_outer.xml 
	cp $< .
	xxd -i $(<F) > temp3 
	head -n -1 temp3 > $@
	rm $(<F)
	rm temp3

ml_training_data_bdtg_arm0_E33_0.h:\
	TMVARegression_BDTG.weights.arm0_E33_0.xml 
	cp $< .
	xxd -i $(<F) > temp4 
	head -n -1 temp4 > $@
	rm $(<F)
	rm temp4

ml_training_data_bdtg_arm1.h:\
	TMVARegression_BDTG.weights.arm1.xml 
	cp $< .
	xxd -i $(<F) > temp5 
	head -n -1 temp5 > $@
	rm $(<F)
	rm temp5

ml_training_data_bdtg_arm1_inner.h:\
	TMVARegression_BDTG.weights.arm1_inner.xml 
	cp $< .
	xxd -i $(<F) > temp6 
	head -n -1 temp6 > $@
	rm $(<F)
	rm temp6

ml_training_data_bdtg_arm1_outer.h:\
	TMVARegression_BDTG.weights.arm1_outer.xml 
	cp $< .
	xxd -i $(<F) > temp7 
	head -n -1 temp7 > $@
	rm $(<F)
	rm temp7

ml_training_data_bdtg_arm1_E33_0.h:\
	TMVARegression_BDTG.weights.arm1_E33_0.xml 
	cp $< .
	xxd -i $(<F) > temp8 
	head -n -1 temp8 > $@
	rm $(<F)
	rm temp8

ml_training_data_mlp_arm0.h:\
	TMVARegression_MLP.weights.arm0.xml 
	cp $< .
	xxd -i $(<F) > temp9 
	head -n -1 temp9 > $@
	rm $(<F)
	rm temp9

ml_training_data_mlp_arm0_inner.h:\
	TMVARegression_MLP.weights.arm0_inner.xml 
	cp $< .
	xxd -i $(<F) > temp10 
	head -n -1 temp10 > $@
	rm $(<F)
	rm temp10

ml_training_data_mlp_arm0_outer.h:\
	TMVARegression_MLP.weights.arm0_outer.xml 
	cp $< .
	xxd -i $(<F) > temp11 
	head -n -1 temp11 > $@
	rm $(<F)
	rm temp11

ml_training_data_mlp_arm0_E33_0.h:\
	TMVARegression_MLP.weights.arm0_E33_0.xml 
	cp $< .
	xxd -i $(<F) > temp12 
	head -n -1 temp12 > $@
	rm $(<F)
	rm temp12

ml_training_data_mlp_arm1.h:\
	TMVARegression_MLP.weights.arm1.xml 
	cp $< .
	xxd -i $(<F) > temp13 
	head -n -1 temp13 > $@
	rm $(<F)
	rm temp13

ml_training_data_mlp_arm1_inner.h:\
	TMVARegression_MLP.weights.arm1_inner.xml 
	cp $< .
	xxd -i $(<F) > temp14 
	head -n -1 temp14 > $@
	rm $(<F)
	rm temp14

ml_training_data_mlp_arm1_outer.h:\
	TMVARegression_MLP.weights.arm1_outer.xml 
	cp $< .
	xxd -i $(<F) > temp15 
	head -n -1 temp15 > $@
	rm $(<F)
	rm temp15

ml_training_data_mlp_arm1_E33_0.h:\
	TMVARegression_MLP.weights.arm1_E33_0.xml 
	cp $< .
	xxd -i $(<F) > temp16 
	head -n -1 temp16 > $@
	rm $(<F)
	rm temp16

ml_training_data_bdtg_mpcex_only.h:\
	TMVARegression_MpcExOnly_BDTG.weights.xml
	cp $< .
	xxd -i $(<F) > temp17 
	head -n -1 temp17 > $@
	rm $(<F)
	rm temp17

ml_training_data_bdtg_photon_frac.h:\
	TMVARegression_BDTG_photon_frac.weights.xml
	cp $< .
	xxd -i $(<F) > temp18
	head -n -1 temp18 > $@
	rm $(<F)
	rm temp18

ml_training_data_bdtg_pi0_frac.h:\
	TMVARegression_BDTG_pi0_frac.weights.xml
	cp $< .
	xxd -i $(<F) > temp19
	head -n -1 temp19 > $@
	rm $(<F)
	rm temp19

clean-local:
	rm -f *Dict*
