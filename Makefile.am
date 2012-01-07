#
# we need to add "manually" to the distribution set the perseus configure.ac
# (and in the future the usrp/configure.ac as well) because the make dist
# command doesn't take in account the optional (user selected) subdirectories
#
EXTRA_DIST = $(srcdir)/trunk/src/perseus/configure.ac

if MAKE_PERSEUS
   PERSEUS_DIR = trunk/src/perseus
endif

if MAKE_USRP
   ###### uncomment the following line when usrp come in
   ###### USRP_DIR =  trunk/src/usrp
endif

SUBDIRS = trunk/src/softrock trunk/src/DttSP trunk/src/dspserver trunk/src/widget-server trunk/src/server trunk/bin trunk/src/QtRadio trunk/src/pylauncher trunk/src/hiqsdr $(PERSEUS_DIR) $(USRP_DIR) 


