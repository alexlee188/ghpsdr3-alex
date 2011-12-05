EXTRA_DIST = ozyfw-sdr1k.hex Ozy_Janus.rbf 90-ozy.rules
firmwaredir = $(bindir)/
firmware_DATA = ${EXTRA_DIST}

install-exec-hook:
	cp $(bindir)/90-ozy.rules /etc/udev/rules.d/ ; udevadm control --reload-rules

