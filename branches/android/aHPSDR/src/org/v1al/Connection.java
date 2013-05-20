package org.v1al;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.net.InetSocketAddress;

import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord.OnRecordPositionUpdateListener;
import android.media.AudioTrack;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Build;
import android.util.Log;

@TargetApi(Build.VERSION_CODES.CUPCAKE)
public class Connection extends Thread {
	public Connection(String server, int port, int width) {
		// Log.i("Connection",server+":"+port);
		
		SPECTRUM_BUFFER_SIZE=width;
		samples = new byte[SPECTRUM_BUFFER_SIZE];
		for (int i = 0; i < SPECTRUM_BUFFER_SIZE; i++) samples[i] = 120;
		this.server = server;
		this.port = port;
		System.gc();
	}

	public void setServer(String server) {
		Log.i("Connection","setServer: "+server);
		this.server=server;
	}
	
	public void setFps(int fps){
		this.fps = fps;
	}
	
	public boolean connect() {
		boolean result = true;
		Log.i("Connection","connect: "+server+":"+port);
		try {
			socket = new Socket();
			socket.connect(new InetSocketAddress(server,port),5000);
			inputStream = socket.getInputStream();
			outputStream = socket.getOutputStream();
		
		
		    audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000,
				AudioFormat.CHANNEL_OUT_MONO,
				AudioFormat.ENCODING_PCM_16BIT, AUDIO_BUFFER_SIZE * 2,
				AudioTrack.MODE_STREAM);
		
		    audioTrack.play();
		    
		    int N = 10 * AudioRecord.getMinBufferSize(8000,AudioFormat.CHANNEL_IN_MONO,AudioFormat.ENCODING_PCM_16BIT);
		    if (N < micBufferSize * 40) N = micBufferSize * 40; // 40 * 58 = 2320
		    
		    recorder = new AudioRecord(AudioSource.MIC, 8000,
		    				AudioFormat.CHANNEL_IN_MONO,AudioFormat.ENCODING_PCM_16BIT, N);
		    
		    recorder.setPositionNotificationPeriod(micBufferSize * nMicBuffers);
		    final int finalMicBufferSize = micBufferSize;
		    
		    OnRecordPositionUpdateListener positionUpdater = new OnRecordPositionUpdateListener () {
		    	public void onPeriodicNotification(AudioRecord recorder){
		    		for (int j = 0; j < nMicBuffers; j++){
			    		short[] micData = new short[micBufferSize];
			    		recorder.read(micData, 0, finalMicBufferSize);
			    		if (allowTx & MOX){
				    		byte[] micEncodedData = new byte[micBufferSize];
				    		for (int i = 0; i < micBufferSize; i++){
				    			micEncodedData[i] = aLawEncode[(micData[i] << micGain) & 0xFFFF];
				    		}
				    		sendAudio(micEncodedData);
				    	}
		    		}
		    	}
		    	
		    	public void onMarkerReached(AudioRecord recorder){
		    	}
		    	
		    };
		    recorder.setRecordPositionUpdateListener(positionUpdater);
		    recorder.startRecording();
		    short[] buffer = new short[micBufferSize*nMicBuffers];
		    recorder.read(buffer, 0, micBufferSize*nMicBuffers);  // initiate the first read
		    
		    
		} catch (Exception e) {
			Log.e("Connection", "Error creating socket for " + server + ":"
					+ port + "'" + e.getMessage() + "'");
			status=e.toString();
			socket=null;
			result = false;
		}
		return result;
	}
	
	public void close() {
        Log.i("Connection","close");
        running=false;
        if(socket!=null) {
                try {
                        socket.close();
                } catch (Exception e) {

                }
                socket=null;
        }
        if(audioTrack!=null) {
            audioTrack.stop();
            audioTrack.release();
            audioTrack = null;
        }
        if (recorder != null){
        	recorder.stop();
        	recorder.release();
        	recorder = null;
        }
    }

	public boolean isRunning() {
		return running;
	}
	
	public void run() {
		int bytes;
		int bytes_read=0;
		byte buffer_type = 0;
		byte[] buffer_type_array = new byte[1];
		byte[] version = new byte[2];
		int header_size=SPECTRUM_HEADER_SIZE_2_0;
		byte[] spectrumHeader = new byte[SPECTRUM_HEADER_SIZE_2_1]; // largest
		byte[] audioHeader= new byte[AUDIO_HEADER_SIZE];
		byte[] spectrumBuffer = new byte[SPECTRUM_BUFFER_SIZE];
		byte[] audioBuffer = new byte[AUDIO_BUFFER_SIZE];
		byte[] answerBuffer = new byte[101];
        Log.i("Connection","run");
		if (socket != null) {
			running=true;
			while (running) {
				try {
					inputStream.read(buffer_type_array, 0, 1);
			        inputStream.read(version, 0, 2);
					//Log.i("Connection","buffer_type="+buffer_type);
					buffer_type = buffer_type_array[0];
					if(buffer_type==SPECTRUM_BUFFER) {
						bytes = 0;
						switch(version[0]) {
						    case 2:
						    	switch(version[1]) {
						    	    case 0:
						    	    	header_size=SPECTRUM_HEADER_SIZE_2_0;
						    	    	break;
						    	    case 1:
						    	    	header_size=SPECTRUM_HEADER_SIZE_2_1;
						    	    	break;
						    	    default:
						    	    	// invalid subversion
						    	}
						    	break;
							default:
								// invalid version
								break;
						}
						while (bytes != header_size) {
							bytes_read = inputStream.read(spectrumHeader, bytes, header_size
									- bytes);
							if(bytes_read==-1) break;
							bytes+=bytes_read;
						}
						//Log.i("Connection","SPECTRUM_HEADER length="+bytes);
					} else if(buffer_type==AUDIO_BUFFER) {
						bytes = 0;
						while (bytes != AUDIO_HEADER_SIZE) {
							bytes_read = inputStream.read(audioHeader, bytes, AUDIO_HEADER_SIZE
									- bytes);
							if(bytes_read==-1) break;
							bytes+=bytes_read;
						}
						//Log.i("Connection","AUDIO_HEADER length="+bytes);
					} else if(buffer_type==ANSWER_BUFFER) {
						String length_str = new String(version);
						answer_length = Integer.valueOf(length_str);
						bytes_read = 0;
					} else {
						status="invalid buffer type";
						/*
						if(socket!=null) {
							socket.close();
							socket=null;
						}
						status="remote connection terminated";
						connected=false;
						break;
						*/
					}

					if(bytes_read==-1) {
						if(socket!=null) {
							socket.close();
							socket=null;
						}
						status="remote connection terminated";
						connected=false;
						break;
					}
					
					// start the audio once we are connected
					if (!connected) {
						sendCommand("startAudioStream " + AUDIO_BUFFER_SIZE);
						connected = true;
					}

					switch (buffer_type) {
					case SPECTRUM_BUFFER:
						bytes = 0;
						while (bytes != SPECTRUM_BUFFER_SIZE) {
							bytes += inputStream.read(spectrumBuffer, bytes,
									SPECTRUM_BUFFER_SIZE - bytes);
						}
						processSpectrumBuffer(version[0],version[1],spectrumHeader, spectrumBuffer);
						break;
					case AUDIO_BUFFER:
						bytes = 0;
						while (bytes != AUDIO_BUFFER_SIZE) {
							bytes += inputStream.read(audioBuffer, bytes,
									AUDIO_BUFFER_SIZE - bytes);
						}
						processAudioBuffer(audioHeader, audioBuffer);
						break;
					case ANSWER_BUFFER:
						bytes = 0;
						while (bytes != answer_length) {
							bytes += inputStream.read(answerBuffer, bytes,
									answer_length - bytes);
						}
						processAnswerBuffer(answer_length, answerBuffer); // remove terminating null
						break;
					default:
						Log.e("Buffer", "Invalid type " + buffer_type);
						break;
					}

				} catch (Exception e) {
					Log.e("Connection","run: Exception reading socket: "
									+ e.toString());
					e.printStackTrace();
					status=e.toString();
					running=false;
					connected=false;
				}
			}
			
			running=false;
		}
        Log.i("Connection","run: exit");
	}

	private short getShort(byte[] buffer,int offset) {
		short result;
		result=(short)(((buffer[offset]&0xFF)<<8)+(buffer[offset+1]&0xFF));
		return result;
	}
	
	private int getInt(byte[] buffer,int offset) {
		int result;
		result=((buffer[offset]&0xFF)<<24)|((buffer[offset+1]&0xFF)<<16)|((buffer[offset+2]&0xFF)<<8)|(buffer[offset+3]&0xFF);
		return result;
	}
	
	private void processSpectrumBuffer(byte version,byte subversion,byte[] header, byte[] buffer) {
		int offset=0;
		
		meter=getShort(header,2);
        sampleRate=getInt(header,6); 
        
        switch(version) {
            case 2:
            	switch(subversion) {
            	    case 0:
            	    	LO_offset=0;
            	    	break;
            	    case 1:
            	    	LO_offset=getShort(header,10);
            	    	break;
            	    default:
            	    	// invalid subversion
            	    	break;
            	}
        	    break;
        	default:
        		// invalid version
        		break;
        }

        //Log.i("processSpectrumBuffer","sampeRate="+sampleRate+" meter="+meter+" buffer="+buffer.length);

        float step = (float)sampleRate/(float)SPECTRUM_BUFFER_SIZE;
        offset = (int)((float)LO_offset/step);
        
        // rotate spectrum display is done in dspserver now.  Do not rotate.
        	for (int i = 0; i < SPECTRUM_BUFFER_SIZE; i++) {
			    samples[i] = (byte)((((float)samples[i]*(float)spectrumAverage) 
			    		+ (float)buffer[i])/(float)(spectrumAverage+1));
        	}

		if (spectrumView != null) {
			spectrumView.plotSpectrum(samples, filterLow, filterHigh,
					sampleRate, offset);
		}
	}

	private void processAudioBuffer(byte[] header, byte[] buffer) {

		//Log.i("processAudioBuffer","buffer="+buffer.length);
		
		// decode 8 bit aLaw to 16 bit linear
		for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
			if (MOX) decodedBuffer[i] = 0;
			else decodedBuffer[i] = aLawDecode[buffer[i] & 0xFF];
		}

		int waitingToSend = audioSampleCount
				- audioTrack.getPlaybackHeadPosition();
		// Log.d("AudioTrack","waitingToSend="+waitingToSend);
		if (waitingToSend < AUDIO_BUFFER_SIZE) {
			audioSampleCount += audioTrack.write(decodedBuffer, 0,
					AUDIO_BUFFER_SIZE);
		} else {
			Log.d("AudioTrack", "dropping buffer");
		}
	}
	
	private void processAnswerBuffer(int length, byte[] buffer){
		byte[] answer_buff = new byte[length];
		for (int i = 0; i < length; i++) answer_buff[i] = buffer[i];
		String full_string = new String(answer_buff);
		if (full_string.indexOf("q-master") == 0){
			answer = full_string.substring(9);
			if (answer.indexOf("slave") != -1) {
				isSlave = true;
				hasBeenSlave = true;
			}
			else isSlave = false;
		}
		else if (full_string.indexOf("q-info") == 0 && isSlave){
			int freq_pos = full_string.indexOf(";f;");
			int mode_pos = full_string.indexOf(";m;");
			int mode_end_pos = full_string.indexOf(";", mode_pos+3);
			String freq_string = full_string.substring(freq_pos+3, mode_pos);
			this.frequency = Integer.valueOf(freq_string);
			setBand(frequency);
			String mode_string = full_string.substring(mode_pos+3, mode_end_pos);
			this.mode = Integer.valueOf(mode_string);
			switch (mode) {
				case AHPSDRActivity.MODE_LSB:
					setFilter(-3050, -150);
					break;
				case AHPSDRActivity.MODE_USB:
					setFilter(150, 3050);
					break;
				case AHPSDRActivity.MODE_DSB:
					setFilter(-2900, 2900);
					break;
				case AHPSDRActivity.MODE_CWL:
					setFilter(-cwPitch - 250,
							-cwPitch + 250);
					break;
				case AHPSDRActivity.MODE_CWU:
					setFilter(cwPitch - 250,
							cwPitch + 250);
					break;
				case AHPSDRActivity.MODE_FMN:
				case AHPSDRActivity.MODE_AM:
				case AHPSDRActivity.MODE_DIGU:
				case AHPSDRActivity.MODE_DIGL:
				case AHPSDRActivity.MODE_SAM:
					setFilter(-3300, 3300);
					break;
				case AHPSDRActivity.MODE_SPEC:
					break;
				case AHPSDRActivity.MODE_DRM:
					break;
			}
			int zoom_pos = full_string.indexOf(";z;");
			if (zoom_pos != -1){
				int zoom_end_pos = full_string.indexOf(";", zoom_pos+3);
				String zoom_string = full_string.substring(zoom_pos+3, zoom_end_pos);
				zoom = Integer.valueOf(zoom_string);
				if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
				if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
			}
			int low_pos = full_string.indexOf(";l;");
			if (low_pos != -1){
				int low_end_pos = full_string.indexOf(";", low_pos+3);
				String low_string = full_string.substring(low_pos+3, low_end_pos);
				filterLow = Integer.valueOf(low_string);
			}
			int high_pos = full_string.indexOf(";r;");
			if (high_pos != -1){
				int high_end_pos = full_string.indexOf(";", high_pos+3);
				String high_string = full_string.substring(high_pos+3, high_end_pos);
				filterHigh = Integer.valueOf(high_string);
			}
		}
	}

	public synchronized void sendCommand(String command) {

		// Log.i("sendCommand",command);
		byte[] commandBytes = command.getBytes();
		for (int i = 0; i < 64; i++) {
			if (i < commandBytes.length) {
				commandBuffer[i] = commandBytes[i];
			} else {
				commandBuffer[i] = 0;
			}
		}

		if (socket != null) {
			try {
				outputStream.write(commandBuffer);
				outputStream.flush();
			} catch (IOException e) {
				Log.e("Connection","sendCommand: IOException: "
						+ e.getMessage());
				status=e.toString();
				connected=false;
			}
		}
	}

	public synchronized void sendAudio(byte [] micBuffer){
		byte[] commandBuffer = new byte[64];
		commandBuffer[0] = 'm';
		commandBuffer[1] = 'i';
		commandBuffer[2] = 'c';
		commandBuffer[3] = ' ';
		for (int i = 0; i < micBufferSize; i++){
				commandBuffer[i + 4] = MOX ? micBuffer[i] : (byte)213;
			}
		commandBuffer[63] = 0;
		
		if (socket != null) {
			try {
				outputStream.write(commandBuffer);
				outputStream.flush();
			} catch (IOException e) {
				Log.e("Connection","sendCommand: IOException: "
						+ e.getMessage());
				status=e.toString();
				connected=false;
			}
		}
	}
	
	public void setFrequency(long frequency) {
		this.frequency = frequency;
		sendCommand("setFrequency " + frequency);
	}

	public long getFrequency() {
		return frequency;
	}
	
	public void setBand(int band){
		this.band = band;
	}
	
	private void setBand(long frequency){
		if (frequency < 2000000) band = AHPSDRActivity.BAND_160;
		else if (frequency < 3900000) band = AHPSDRActivity.BAND_80;
		else if (frequency < 5500000) band = AHPSDRActivity.BAND_60;
		else if (frequency < 7350000) band = AHPSDRActivity.BAND_40;
		else if (frequency < 10150000) band = AHPSDRActivity.BAND_30;
		else if (frequency < 14350000) band = AHPSDRActivity.BAND_20;
		else if (frequency < 18168000) band = AHPSDRActivity.BAND_17;
		else if (frequency < 21450000) band = AHPSDRActivity.BAND_15;
		else if (frequency < 24990000) band = AHPSDRActivity.BAND_12;
		else if (frequency < 30000000) band = AHPSDRActivity.BAND_10;
		else if (frequency < 50500000) band = AHPSDRActivity.BAND_6;
		else band = AHPSDRActivity.BAND_20;
	}
	
	public int getBand(){
		return band;
	}

	public void setFilter(int filterLow, int filterHigh) {
		this.filterLow = filterLow;
		this.filterHigh = filterHigh;
		sendCommand("setFilter " + filterLow + " " + filterHigh);
	}

	public int getFilterLow() {
		return filterLow;
	}
	
	public int getFilterHigh() {
		return filterHigh;
	}
	
	public void setMode(int mode) {
		this.mode = mode;
		sendCommand("setMode " + mode);
	}

	public int getMode() {
		return mode;
	}

	public String getStringMode() {
		return (String) AHPSDRActivity.modes[mode];
	}

	public void setAGC(int agc) {
		this.agc = agc;
		sendCommand("setAGC  " + agc);
	}

	public void setNR(boolean state) {
		sendCommand("setNR " + state);
	}

	public void setANF(boolean state) {
		sendCommand("setANF " + state);
	}

	public void setNB(boolean state) {
		sendCommand("setNB " + state);
	}

	public void setIQCorrection(boolean state) {
		sendCommand("RxIQmuVal 0.1");
		sendCommand("setIQEnable " + (state ? "true" : "false"));
	}
	
	public void setGain(int gain) {
		sendCommand("SetRXOutputGain " + gain);
	}

	public void getSpectrum() {
		sendCommand("getSpectrum " + SPECTRUM_BUFFER_SIZE);
	}
	
	public void getSpectrum_protocol3(int fps){
		sendCommand("setFPS " + SPECTRUM_BUFFER_SIZE + " " + fps);
	}
	
	public void setMOX(boolean state){
		if (state) {
			sendCommand("Mox on " + txUser + " " + txPass);
			MOX = true;
		}
		else {
			sendCommand("Mox off " + txUser + " " + txPass);
			MOX = false;
		}
	}
	
	public int getScaleFactor(){
		return zoom+1;
	}
	
	public void setScaleFactor(float scale){
		zoom = (int) scale - 1;
		sendCommand("zoom " + zoom);
	}
	
	public boolean getMOX(){
		return MOX;
	}
	
	public void setTxUser(String User){
		txUser = User;
	}
	
	public void setTxPass(String Pass){
		txPass = Pass;
	}

	public byte[] getSamples() {
		return samples;
	}

	public int getMeter() {
		return meter;
	}

	public int getSampleRate() {
		return sampleRate;
	}

	public boolean isConnected() {
		return connected;
	}

	public void setSpectrumView(SpectrumView spectrumView) {
		this.spectrumView = spectrumView;
	}

	public void setStatus(String message) {
		status = message;
	}
	
	public void setMaster(){
		sendCommand("setMaster " + txUser + " " + txPass);
	}

	public String getStatus() {
		return status;
	}
	
	public short getLO_offset(){
		return LO_offset;
	}
	
	public void setAllowTx(boolean state){
		allowTx = state;
	}
	
	public boolean getAllowTx(){
		return allowTx;
	}
	
	public int getMicGain(){
		return micGain;
	}
	
	public void setMicGain(int gain){
		micGain = gain;
	}
	
	public void setSpectrumAverage(int value){
		spectrumAverage = value;
	}
	
	public String getAnswer(){
		return answer;
	}
	
	public boolean getIsSlave(){
		return isSlave;
	}
	
	public boolean getHasBeenSlave(){
		return hasBeenSlave;
	}
	
	public void setHasBeenSlave(boolean state){
		hasBeenSlave = state;
	}

	private SpectrumView spectrumView;

	private static final int BUFFER_TYPE_SIZE = 1;
	private static final int BUFFER_VERSION_SIZE = 2;
	private static final int AUDIO_HEADER_SIZE = 2;
	private static final int SPECTRUM_HEADER_SIZE_2_0 = 10;
	private static final int SPECTRUM_HEADER_SIZE_2_1 = 12;
	private int SPECTRUM_BUFFER_SIZE = 480;
	static final int AUDIO_BUFFER_SIZE = 2000;

	private static final byte SPECTRUM_BUFFER = 0;
	private static final byte AUDIO_BUFFER = 1;
	private static final byte ANSWER_BUFFER = '4';
	
	private int answer_length = 0;
	private String answer = "unknown";
	private boolean isSlave = false;
	private boolean hasBeenSlave = false;
	private String server;
	private int port;
	private Socket socket;
	private InputStream inputStream;
	private OutputStream outputStream;
	private boolean running = false;
	private boolean connected = false;
	
	private String txUser = "";
	private String txPass = "";
	

	private short LO_offset;
	
	private long frequency;
	private int filterLow;
	private int filterHigh;
	private int mode;
	private int band = AHPSDRActivity.BAND_20;
	private int fps;
	private int agc;
	private int sampleRate;
	private short meter;
	private int spectrumAverage = 0;

	private int cwPitch = 600;
	private byte[] commandBuffer = new byte[64];
	private byte[] samples;
	private int audioSampleCount;
	private short[] decodedBuffer = new short[AUDIO_BUFFER_SIZE];
	private AudioTrack audioTrack;
	private AudioRecord recorder;
	private boolean MOX = false;
	private String status = "";
	
	private int zoom = 0;
	private static int MIN_ZOOM = 0;
	private static int MAX_ZOOM = 99;

	public final int micBufferSize = 58;
	private final int nMicBuffers = 2;
	private int micGain = 0;
	private boolean allowTx = false;
	
	private static short[] aLawDecode = new short[256];
	private static byte[] aLawEncode = new byte[65536];
	
    static final int[] _s2a = {

        213,212,215,214,209,208,211,210,221,220,223,222,217,216,219,218,
        197,196,199,198,193,192,195,194,205,204,207,206,201,200,203,202,
        245,245,244,244,247,247,246,246,241,241,240,240,243,243,242,242,
        253,253,252,252,255,255,254,254,249,249,248,248,251,251,250,250,
        229,229,229,229,228,228,228,228,231,231,231,231,230,230,230,230,
        225,225,225,225,224,224,224,224,227,227,227,227,226,226,226,226,
        237,237,237,237,236,236,236,236,239,239,239,239,238,238,238,238,
        233,233,233,233,232,232,232,232,235,235,235,235,234,234,234,234,
        149,149,149,149,149,149,149,149,148,148,148,148,148,148,148,148,
        151,151,151,151,151,151,151,151,150,150,150,150,150,150,150,150,
        145,145,145,145,145,145,145,145,144,144,144,144,144,144,144,144,
        147,147,147,147,147,147,147,147,146,146,146,146,146,146,146,146,
        157,157,157,157,157,157,157,157,156,156,156,156,156,156,156,156,
        159,159,159,159,159,159,159,159,158,158,158,158,158,158,158,158,
        153,153,153,153,153,153,153,153,152,152,152,152,152,152,152,152,
        155,155,155,155,155,155,155,155,154,154,154,154,154,154,154,154,
        133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,
        132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,
        135,135,135,135,135,135,135,135,135,135,135,135,135,135,135,135,
        134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,
        129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
        128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
        131,131,131,131,131,131,131,131,131,131,131,131,131,131,131,131,
        130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,
        141,141,141,141,141,141,141,141,141,141,141,141,141,141,141,141,
        140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,
        143,143,143,143,143,143,143,143,143,143,143,143,143,143,143,143,
        142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,
        137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,
        136,136,136,136,136,136,136,136,136,136,136,136,136,136,136,136,
        139,139,139,139,139,139,139,139,139,139,139,139,139,139,139,139,
        138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,
        181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
        181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
        180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
        180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
        183,183,183,183,183,183,183,183,183,183,183,183,183,183,183,183,
        183,183,183,183,183,183,183,183,183,183,183,183,183,183,183,183,
        182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,
        182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,
        177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,
        177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,
        176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
        176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
        179,179,179,179,179,179,179,179,179,179,179,179,179,179,179,179,
        179,179,179,179,179,179,179,179,179,179,179,179,179,179,179,179,
        178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,
        178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,
        189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,
        189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,
        188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
        188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
        191,191,191,191,191,191,191,191,191,191,191,191,191,191,191,191,
        191,191,191,191,191,191,191,191,191,191,191,191,191,191,191,191,
        190,190,190,190,190,190,190,190,190,190,190,190,190,190,190,190,
        190,190,190,190,190,190,190,190,190,190,190,190,190,190,190,190,
        185,185,185,185,185,185,185,185,185,185,185,185,185,185,185,185,
        185,185,185,185,185,185,185,185,185,185,185,185,185,185,185,185,
        184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,
        184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,
        187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,
        187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,
        186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,
        186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,
        165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
        165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
        165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
        165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
        164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
        164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
        164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
        164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
        167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,
        167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,
        167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,
        167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,
        166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,
        166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,
        166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,
        166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,
        161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
        161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
        161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
        161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
        160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
        160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
        160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
        160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
        163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,
        163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,
        163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,
        163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,163,
        162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,
        162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,
        162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,
        162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,
        173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
        173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
        173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
        173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
        172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,
        172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,
        172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,
        172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,
        175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,
        175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,
        175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,
        175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,
        174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,
        174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,
        174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,
        174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,
        169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,
        169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,
        169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,
        169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,169,
        168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,
        168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,
        168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,
        168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,
        171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,
        171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,
        171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,
        171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,171,
        170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
        170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
        170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
        170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
         42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
         42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
         42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
         42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
         43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
         43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
         43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
         43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
         41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
         41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
         41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
         46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
         46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
         46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
         46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
         47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
         47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
         47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
         47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
         44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
         44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
         44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
         44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
         45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
         45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
         45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
         45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
         34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
         34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
         34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
         34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
         35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
         35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
         35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
         35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
         32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
         32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
         32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
         32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
         33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
         33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
         33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
         33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
         38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
         38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
         38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
         38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
         36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
         36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
         36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
         37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
         37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
         37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
         37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
         58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
         58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
         59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
         59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
         56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
         56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
         57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57,
         57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57,
         62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
         62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
         63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
         63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
         60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
         60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
         61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
         61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
         50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
         50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
         51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
         51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
         48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
         48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
         49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
         49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
         54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
         54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
         55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
         55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
         52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
         52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
         53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
         53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
         10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
         11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
          8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
          9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
         14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
         15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
         12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
          2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
          3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
          1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
          6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
          7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
          4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
          5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
         26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27,
         24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25,
         30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31,
         28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29,
         18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19,
         16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17,
         22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
         20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21,
        106,106,106,106,107,107,107,107,104,104,104,104,105,105,105,105,
        110,110,110,110,111,111,111,111,108,108,108,108,109,109,109,109,
         98, 98, 98, 98, 99, 99, 99, 99, 96, 96, 96, 96, 97, 97, 97, 97,
        102,102,102,102,103,103,103,103,100,100,100,100,101,101,101,101,
        122,122,123,123,120,120,121,121,126,126,127,127,124,124,125,125,
        114,114,115,115,112,112,113,113,118,118,119,119,116,116,117,117,
         74, 75, 72, 73, 78, 79, 76, 77, 66, 67, 64, 65, 70, 71, 68, 69,
         90, 91, 88, 89, 94, 95, 92, 93, 82, 83, 80, 81, 86, 87, 84, 85
};


	static {

		for (int i = 0; i < 256; i++) {
			int input = i ^ 85;
			int mantissa = (input & 15) << 4;
			int segment = (input & 112) >> 4;
			int value = mantissa + 8;
			if (segment >= 1) {
				value += 256;
			}
			if (segment > 1) {
				value <<= (segment - 1);
			}
			if ((input & 128) == 0) {
				value = -value;
			}
			aLawDecode[i] = (short) value;
		}
		
	    for(int i=0;i<65536;i++) {
	       aLawEncode[i]=(byte)(_s2a[i>>4]);
	    }
	}

}
