/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package jmonitor;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 *
 * @author john
 */
public class Client extends Thread {

    public Client(String server,int receiver,Audio audio,int limit) {
        this.receiver=receiver;
        this.audio=audio;
        this.limit=limit;
        if(server!=null) {
            this.server=server;
        }
        port=8000+receiver;
        connected=false;
        status="Server "+server+" is busy - please wait";
        try {
            socket=new Socket(this.server,port);
            inputStream=socket.getInputStream();
            outputStream=socket.getOutputStream();
            System.err.println("opened client socket on port "+Integer.toString(port));

            if(limit>0) {
                ConnectionLimit connectionLimit=new ConnectionLimit(limit);
                connectionLimit.start();
            }
        } catch (UnknownHostException e) {
            System.err.println("Client: UnknownHost: "+server);
            status="Unknown host "+server;
        } catch (IOException e) {
            System.err.println("Client: IOException: "+e.getMessage());
            status="Server "+server+" "+e.getMessage()+" - please try later.";
        }
    }

    private synchronized void sendCommand(String command) {
        byte[] buffer=new byte[64];
        byte[] commandBytes=command.getBytes();

        //System.err.println(command);
        
        for(int i=0;i<64;i++) {
            if(i<commandBytes.length) {
                buffer[i]=commandBytes[i];
            } else {
                buffer[i]=0;
            }
        }

        if(socket!=null) {
            try {
                outputStream.write(buffer);
                outputStream.flush();
            } catch (IOException e) {
                //System.err.println("sendCommand: IOException: "+e.getMessage());
                socket=null;
            }
        }
    }

    public void close() {
        running=false;
        if(socket!=null) {
            try {
                socket.close();
            } catch(IOException e) {
                // ignore error on close
            }
        }
    }

    public void run() {
        int bytes;
        int buffer_type = 0;
	byte[] buffer_type_array = new byte[1];
	byte[] version = new byte[2];
        byte[] spectrum_header=new byte[SPECTRUM_HEADER_SIZE];
        byte[] audio_header=new byte[AUDIO_HEADER_SIZE];
        byte[] spectrum_buffer=new byte[SPECTRUM_BUFFER_SIZE];
        byte[] audio_buffer=new byte[AUDIO_BUFFER_SIZE];
	byte[] answerBuffer = new byte[101];
        int j;
        if(socket!=null) {

            // flush
//            try {
//                while(inputStream.available()>buffer.length) {
//                    inputStream.skip(buffer.length);
//                }
//            } catch (IOException e) {
//                System.err.println("Client.run: flush: "+e.getMessage());
//            }
            status=null;
            while(running) {
                try {
		    inputStream.read(buffer_type_array, 0, 1);
		    inputStream.read(version, 0, 2);
                    buffer_type= buffer_type_array[0];
                    if(buffer_type==SPECTRUM_BUFFER) {
                        bytes=0;
                        while(bytes<SPECTRUM_HEADER_SIZE) {
                            bytes+=inputStream.read(spectrum_header,bytes,SPECTRUM_HEADER_SIZE-bytes);
                        }
                        bytes=0;
                        while(bytes<SPECTRUM_BUFFER_SIZE) {
                            bytes+=inputStream.read(spectrum_buffer,bytes,SPECTRUM_BUFFER_SIZE-bytes);
                        }
                        processSpectrumBuffer(spectrum_header,spectrum_buffer);
                    } else if(buffer_type==AUDIO_BUFFER) {
                        bytes=0;
                        while(bytes<AUDIO_HEADER_SIZE) {
                            bytes+=inputStream.read(audio_header,bytes,AUDIO_HEADER_SIZE-bytes);
                        }
                        bytes=0;
                        while(bytes<AUDIO_BUFFER_SIZE) {
                            bytes+=inputStream.read(audio_buffer,bytes,AUDIO_BUFFER_SIZE-bytes);
                        }
                        processAudioBuffer(audio_header,audio_buffer);
		    } else if(buffer_type==ANSWER_BUFFER) {
			String length_str = new String(version);
			answer_length = Integer.valueOf(length_str);
			bytes = 0;
			while (bytes != answer_length) {
				bytes += inputStream.read(answerBuffer, bytes,
						answer_length - bytes);
			}
			//processAnswerBuffer(answer_length, answerBuffer); // remove terminating null
                    } else {
                        System.err.println("Client: invalid buffer_type "+buffer_type);
                    }
                    if(connected==false) {
                        sendCommand("startAudioStream "+AUDIO_BUFFER_SIZE);
			//sendCommand("setFPS " + SPECTRUM_BUFFER_SIZE + " " + 10);	// 10 is the fps
                        connected=true;
                    }
                } catch (IOException e) {
                    if(running) {
                        status="Exceptiion: "+e.getMessage();
                        System.err.println("Client.run exception reading input stream "+e.getMessage());
                        running=false;
                    }
                }

            }
            //System.err.println("Client.run: status: "+status);
            listener.updateStatus();
        }
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

    private void processSpectrumBuffer(byte[] header,byte[] buffer) {
        int j;

/*
        sampleRate = ((header[8] & 0xFF) << 24) + ((header[9] & 0xFF) << 16) + ((header[10] & 0xFF) << 8) + (header[11] & 0xFF);
        meter=(short)(((header[4]&0xFF)<<8)+(header[5]&0xFF));
*/
	meter=getShort(header,2);
        sampleRate=getInt(header,6);

        for (int i = 0; i < SAMPLES; i++) {
            samples[i] = -(buffer[i] & 0xFF);
        }

        listener.updateSamples(this.getSamples(),this.getFilterLow(),this.getFilterHigh(),this.getSampleRate());

    }

    private void processAudioBuffer(byte[] header,byte[] buffer) {
        audio.playAudioBuffer(buffer);
    }

    public void getSpectrum(MonitorUpdateListener listener) {
        this.listener=listener;
        sendCommand("getSpectrum "+SAMPLES);
    }


    public float[] getSamples() {
        return samples;
    }

    public long getFrequency() {
        return frequency;
    }

    public int getMode() {
        return mode;
    }

    public String getStringMode() {
        return modes[mode];
    }

    public int getFilterLow() {
        return filterLow;
    }

    public int getFilterHigh() {
        return filterHigh;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public void setFrequency(long frequency) {
        this.frequency=frequency;
        sendCommand("setFrequency "+frequency);
    }

    public void setFilter(int filterLow,int filterHigh) {
        this.filterLow=filterLow;
        this.filterHigh=filterHigh;
        sendCommand("setFilter "+filterLow+" "+filterHigh);
        //System.err.println("setFilter "+filterLow+" "+filterHigh);
    }

    public void setMode(int mode) {
        this.mode=mode;
        sendCommand("setMode "+mode);
        //System.err.println("setMode " + mode);
    }

    public int getMeter() {
        return meter;
    }

    public int getCWPitch() {
        return cwPitch;
    }

    public void setAGC(int agc) {
        this.agc=agc;
        sendCommand("setAGC  "+agc);
    }

    public void setNR(boolean state) {
        sendCommand("setNR "+state);
    }

    public void setANF(boolean state) {
        sendCommand("setANF "+state);
    }

    public void setNB(boolean state) {
        sendCommand("setNB "+state);
    }

    public void setGain(int gain) {
        sendCommand("SetRXOutputGain "+gain);
    }
    
    public String getStatus() {
        return status;
    }

    public boolean isConnected() {
        return connected;
    }

    public String getServer() {
        return server;
    }

    public int getReceiver() {
        return receiver;
    }

    private static final int SPECTRUM_BUFFER=0;
    private static final int AUDIO_BUFFER=1;
    private static final byte ANSWER_BUFFER = '4';
    private int answer_length = 0;
    
    private static final int SPECTRUM_HEADER_SIZE=12;
    private static final int AUDIO_HEADER_SIZE=2;

    private static final int SPECTRUM_BUFFER_SIZE=480;
    
    public static final int AUDIO_BUFFER_SIZE=2000;
    
    private Audio audio;
    private MonitorUpdateListener listener;

    private boolean running=true;

    private int limit;

    private int receiver=0;
    private int port=8000;
    private String server="192.168.1.9";
    private Socket socket;

    private InputStream inputStream;
    private OutputStream outputStream;

    private static final int SAMPLES=480;
    private float[] samples=new float[SAMPLES];

    private long frequency;
    private int filterLow;
    private int filterHigh;
    private int mode;
    //private String sampleRate;
    private int sampleRate;
    private int band;
    private short meter;
    private int agc;

    private int cwPitch=600;

    public static final int modeLSB=0;
    public static final int modeUSB=1;
    public static final int modeDSB=2;
    public static final int modeCWL=3;
    public static final int modeCWU=4;
    public static final int modeFMN=5;
    public static final int modeAM=6;
    public static final int modeDIGU=7;
    public static final int modeSPEC=8;
    public static final int modeDIGL=9;
    public static final int modeSAM=10;
    public static final int modeDRM=11;

    private String[] modes={"LSB","USB","DSB","CWL","CWU","FMN","AM","DIGU","SPEC","DIGL","SAM","DRM"};

    private String status;

    private boolean connected;

    class ConnectionLimit extends Thread {

        ConnectionLimit(int timeout) {
            this.timeout=timeout;
        }

        public void run() {
            //System.err.println("ConnectionLimit sleeping for "+limit+" seconds");
            try {
                sleep(timeout*1000);
            } catch (InterruptedException e) {
                System.err.println("ConnectionLimit: InterruptedException: "+e.getMessage());
            }
            //System.err.println("ConnectionLimit timedout");
            status="Time limit exceeded!";
            close();
        }

        private int timeout;
    }
}
