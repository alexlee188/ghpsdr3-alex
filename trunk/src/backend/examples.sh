#!/bin/sh

case $1 in
    rtlsdr-local)    
	# example radio-backend invocation
	# runs the rtlsdr-server with default parameters
	# runs the server in ~/rtlsdr-backend with a logs directory
	# creates the dspserver pkey/cert files
	# runs dspserver with no local oscillator offset
	radio-backend \
	    --key \
	    --hw "rtlsdr-server" \
	    --dsp "dspserver --lo 0 --debug" \
	    --ui "QtRadio -s localhost" \
	    --in ~/rtlsdr-backend \
	    --log logs/
	;;
    sdrplay-remote)
	radio-backend \
	    --at rradio \
	    --key \
	    --hw "sdrplay-server --samplerate 96000" \
	    --dsp "dspserver --lo 10000" \
	    --ui "QtRadio -s rradio" \
	    --in ~/sdrplay-backend \
	    --log logs/
	;;
    sdrplay-local)
	radio-backend \
	    --key \
	    --hw "sdrplay-server --samplerate 192000" \
	    --dsp "dspserver --lo 10000" \
	    --ui "QtRadio -s localhost" \
	    --in ~/sdrplay-backend \
	    --log logs/
	;;
    rtlsdr-local)    
	# example radio-backend invocation
	# runs the rtlsdr-server with default parameters
	# runs the server in ~/rtlsdr-backend with a logs directory
	# creates the dspserver pkey/cert files
	# runs dspserver with no local oscillator offset
	radio-backend \
	    --key \
	    --hw "rtlsdr-server" \
	    --dsp "dspserver --lo 0 --debug" \
	    --ui "QtRadio -s localhost" \
	    --in ~/rtlsdr-backend \
	    --log logs/
	;;
esac
