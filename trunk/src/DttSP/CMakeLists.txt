project(DttSP)


set(CMAKE_VERBOSE_MAKEFILE ON)
include_directories(.)

ADD_DEFINITIONS(  -g -O3 -msse -ffast-math -Wall -Wno-unused-function)


ADD_LIBRARY ( DttSP STATIC
	am_demod.c
	banal.c
	bufvec.c
	correctIQ.c
	cwtones.c
	cxops.c
	dcblock.c
	dttspagc.c
	fastrig.c
	filter.c
	fm_demod.c
	graphiceq.c
	hilbert.c
	isoband.c
	keyer.c
	keyerio.c
	lmadf.c
	malloc16.c
	meter.c
	metermon.c
	noiseblanker.c
	oscillator.c
	ovsv.c
	resample.c
	resampleF.c
	ringb.c
	sdr.c
	sdrexport.c
	spectrum.c
	speechproc.c
	splitfields.c
	spottone.c
	update.c
	window.c
	winmain.c
	wscompand.c
)

