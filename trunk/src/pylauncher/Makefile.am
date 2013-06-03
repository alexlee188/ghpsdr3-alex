##pythondir = $(prefix)/bin

dist_bin_SCRIPTS = ghpsdr3-mgr.py exit24.png  fav24.png  hpsdr24.png hiqsdr.png p24.png  reset24.png  sr24.png  start24.png  stop24.png

#
# auxiliary services, git string, icons
#


desktop_in_files = ghpsdr3launcher.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)



EXTRA_DIST = icons $(desktop_in_files) $(desktop_DATA)

pixmapdir = $(datadir)/icons
pixmap_DATA = $(srcdir)/icons/ghpsdr3launcher.png


UPDATE_DESKTOP = \
	if which update-desktop-database>/dev/null 2>&1 ; then \
		update-desktop-database; \
	fi

install-data-hook:
	xdg-desktop-menu install --mode user --novendor $(srcdir)/ghpsdr3launcher.desktop

uninstall-hook:
	xdg-desktop-menu uninstall --mode user --novendor ghpsdr3launcher.desktop

