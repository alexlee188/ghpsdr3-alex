package org.g0orx;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.InetSocketAddress;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Connection extends Thread {
	public Connection(String server, int port, int width) {
		// Log.i("Connection",server+":"+port);
		
		SPECTRUM_BUFFER_SIZE=width;
		samples = new byte[SPECTRUM_BUFFER_SIZE];

		this.server = server;
		this.port = port;
		
		//connect();

		// 2.1
		// audioTrack=new
		// AudioTrack(AudioManager.STREAM_MUSIC,8000,AudioFormat.CHANNEL_OUT_MONO,AudioFormat.ENCODING_PCM_16BIT,BUFFER_SIZE*2,AudioTrack.MODE_STREAM);

		System.gc();
	}

	public void setServer(String server) {
		Log.i("Connection","setServer: "+server);
		this.server=server;
	}
	
	public void setFps(int fps){
		this.fps = fps;
	}
	
	public void connect() {
		Log.i("Connection","connect: "+server+":"+port);
		try {
			socket = new Socket();
			socket.connect(new InetSocketAddress(server,port),5000);
			inputStream = socket.getInputStream();
			outputStream = socket.getOutputStream();
		
		
		    audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000,
				AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT, AUDIO_BUFFER_SIZE * 2,
				AudioTrack.MODE_STREAM);
		
		    audioTrack.play();
		    
		    sendCommand("setClient aHPSDR(v1.1)");
		    
		} catch (Exception e) {
			Log.e("Connection", "Error creating socket for " + server + ":"
					+ port + "'" + e.getMessage() + "'");
			status=e.toString();
			socket=null;
		}
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
        }
    }

	public boolean isRunning() {
		return running;
	}
	
	public void run() {
		int bytes;
		int bytes_read=0;
		int buffer_type=0;
		int version=0;
		int subversion=0;
		int header_size=SPECTRUM_HEADER_SIZE_2_0;
		byte[] spectrumHeader = new byte[SPECTRUM_HEADER_SIZE_2_1]; // largest
		byte[] audioHeader= new byte[AUDIO_HEADER_SIZE];
		byte[] spectrumBuffer = new byte[SPECTRUM_BUFFER_SIZE];
		byte[] audioBuffer = new byte[AUDIO_BUFFER_SIZE];
        Log.i("Connection","run");
		if (socket != null) {
			running=true;
			while (running) {
				try {

			        buffer_type=inputStream.read();
			        version=inputStream.read();
			        subversion=inputStream.read();
					
					//Log.i("Connection","buffer_type="+buffer_type);
					
					if(buffer_type==SPECTRUM_BUFFER) {
						bytes = 0;
						switch(version) {
						    case 2:
						    	switch(subversion) {
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
						processSpectrumBuffer(version,subversion,spectrumHeader, spectrumBuffer);
						break;
					case AUDIO_BUFFER:
						bytes = 0;
						while (bytes != AUDIO_BUFFER_SIZE) {
							bytes += inputStream.read(audioBuffer, bytes,
									AUDIO_BUFFER_SIZE - bytes);
						}
						processAudioBuffer(audioHeader, audioBuffer);
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
	
	private void processSpectrumBuffer(int version,int subversion,byte[] header, byte[] buffer) {
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

        // rotate spectrum display if LO is not 0
        if(LO_offset==0) {
        	for (int i = 0; i < SPECTRUM_BUFFER_SIZE; i++) {
			    samples[i] = buffer[i];
		    }
        } else {
            float step=(float)sampleRate/(float)SPECTRUM_BUFFER_SIZE;
            offset=(int)((float)LO_offset/step);
            int j;
            for(int i=0;i<SPECTRUM_BUFFER_SIZE;i++) {
                j=i-offset;
                if(j<0) j+=SPECTRUM_BUFFER_SIZE;
                if(j>=SPECTRUM_BUFFER_SIZE) j%=SPECTRUM_BUFFER_SIZE;
                samples[i] = buffer[j];
            }
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
			decodedBuffer[i] = aLawDecode[buffer[i] & 0xFF];
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

	public void setFrequency(long frequency) {
		this.frequency = frequency;
		sendCommand("setFrequency " + frequency);
	}

	public long getFrequency() {
		return frequency;
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
		return modes[mode];
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

	public void setGain(int gain) {
		sendCommand("SetRXOutputGain " + gain);
	}

	public void getSpectrum() {
		sendCommand("getSpectrum " + SPECTRUM_BUFFER_SIZE);
	}
	
	public void getSpectrum_protocol3(int fps){
		sendCommand("setFPS " + SPECTRUM_BUFFER_SIZE + " " + fps);
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

	public String getStatus() {
		return status;
	}

	private SpectrumView spectrumView;

	private static final int BUFFER_TYPE_SIZE = 1;
	private static final int BUFFER_VERSION_SIZE = 2;
	private static final int AUDIO_HEADER_SIZE = 2;
	private static final int SPECTRUM_HEADER_SIZE_2_0 = 10;
	private static final int SPECTRUM_HEADER_SIZE_2_1 = 12;
	private int SPECTRUM_BUFFER_SIZE = 480;
	static final int AUDIO_BUFFER_SIZE = 2000;

	private static final int SPECTRUM_BUFFER = 0;
	private static final int AUDIO_BUFFER = 1;

	private String server;
	private int port;
	private Socket socket;
	private InputStream inputStream;
	private OutputStream outputStream;
	private boolean running = false;
	private boolean connected = false;
	

	private short LO_offset;
	
	private long frequency;
	private int filterLow;
	private int filterHigh;
	private int mode;
	private int sampleRate;
	private int band;
	private short meter;
	private int agc;
	private int fps;

	private int cwPitch = 600;

	private byte[] commandBuffer = new byte[64];

	private byte[] samples;
	private int audioSampleCount;
	private short[] decodedBuffer = new short[AUDIO_BUFFER_SIZE];

	private AudioTrack audioTrack;
	private String status = "";

	public static final int modeLSB = 0;
	public static final int modeUSB = 1;
	public static final int modeDSB = 2;
	public static final int modeCWL = 3;
	public static final int modeCWU = 4;
	public static final int modeFMN = 5;
	public static final int modeAM = 6;
	public static final int modeDIGU = 7;
	public static final int modeSPEC = 8;
	public static final int modeDIGL = 9;
	public static final int modeSAM = 10;
	public static final int modeDRM = 11;

	private static final String[] modes = { "LSB", "USB", "DSB", "CWL", "CWU", "FMN", "AM",
			"DIGU", "SPEC", "DIGL", "SAM", "DRM" };

	private static short[] aLawDecode = new short[256];

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

	}

}
