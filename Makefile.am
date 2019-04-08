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
  ExHitList.h
 
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
  ExSimEventV2.cxx \
  ExHitList.cxx

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
  MyUtilityLinkDef.h 
	rootcint -f $@ -c $(DEFAULT_INCLUDES) $(INCLUDES) $^

testexternals_SOURCES = \
  testexternals.C

noinst_PROGRAMS = \
  testexternals

testexternals_LDADD = \
  libMyUtility.la

testexternals.C:
	@echo '//*** this is a generated file. Do not commit, do not edit' > $@
	@echo 'int main() { }' >> $@

clean-local:
	rm -f *Dict*
