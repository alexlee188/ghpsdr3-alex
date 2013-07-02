/*
 * Client.java
 * Created by John Melton G0ORX
 * 
 */

/*
 * This code has been and reviewed modified.
 * John Tucker, N8MDP
 */

/*
 * Revsion History
 * 1/20/13: Changes:
 *  1. public void run()
 *      A. Added byte[] answerBuffer = new byte [101] to hande the 
 *          ghpsdr3-alex protocol
 *      B. Added code to start a timer that determines whether the tuner is 
 *          Master or Slave. The timer task is part of the process that updates
 *          the spectrum, waterfall , and frequency in slave mode. Timer is (1)
 *          one second intervals.
 *      C. Code in the "while (running) section was pulled from glSDR for 
 *          consistency.
 *  2. private void processSpectrumBuffer
 *      A. Updated this code for the ghpsdr3-alex protocols. Pulled from glSDR.
 *      Code in this section gets the LO_offset Local Oscillator offset from 
 *      dspserver. It also computes 'step' for step size and the 'offset' 
 *      adjustment factor (LO_offset/step).
 *  3. private void processAnswerBuffer
 *      A. This code was pulled from glSDR. Identification of Master or Slave 
 *          mode is performed here. Note that if Master-Mode, the timer is 
 *          cancelled.
 *  4. Added the following new functions to support updates:
 *      A. public boolean getIsSlave()
 *      B. public int getOffset()
 *      C. public String getmsMode()
 *      5. Added/Changed new declarations to support updates:
 *          a. ADDED private static final byte ANSWER_BUFFER = '4';
 *          b. ADDED private int answer_length = 0;
 *          c. ADDED private static final int SPECTRUM_HEADER_SIZE_2_0=10;    
 *          d. ADDED private static final int SPECTRUM_HEADER_SIZE_2_1=12; 
 *          e. Changed AUDIO_BUFFER_SIZE from 480 to 2000
 *          f. Changed public boolean isSlave = false; from private
 *          g. ADDED private Timer timer; function
 *          h. ADDED private short LO_offset;
 *          i. ADDED public int freq_offset;
 *          j. ADDED public String msMode="Master-Mode";
 *          k. ADDED class answerTask extends TimerTask
 *
 */

package jmonitor;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

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
        int bytes_read=0;       
        byte[] buffer_type_array = new byte[1];
        byte[] version = new byte[2];        

        int header_size=SPECTRUM_HEADER_SIZE_2_0;
        byte[] spectrum_header=new byte[SPECTRUM_HEADER_SIZE];
        byte[] audio_header=new byte[AUDIO_HEADER_SIZE];
        byte[] spectrum_buffer=new byte[SPECTRUM_BUFFER_SIZE];
        byte[] audio_buffer=new byte[AUDIO_BUFFER_SIZE];
        
        // This line of code is needed to handle the protocols of
        // ghpsdr3-alex  01-12-13.        
        byte[] answerBuffer = new byte[101];
        
        // Start the timer??        
	timer = new Timer();
	timer.schedule(new answerTask(), 1000, 1000);
        
        if(socket!=null) {
            status=null;
//            running = true;
 
	    while (running) {
	    try {
		inputStream.read(buffer_type_array, 0, 1);
	        inputStream.read(version, 0, 2);
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
                                    break;
				}
                            break;
			default:
			// invalid version
                            break;
                        }
                    while (bytes != header_size) {
                        bytes_read = inputStream.read(spectrum_header, bytes, header_size - bytes);
                        if(bytes_read==-1) {
                            break;
                        }
                        bytes+=bytes_read;
                        }
                } else if(buffer_type==AUDIO_BUFFER) {
                    bytes = 0;
                    while (bytes != AUDIO_HEADER_SIZE) {
                        bytes_read = inputStream.read(audio_header, bytes, AUDIO_HEADER_SIZE - bytes);
			if(bytes_read==-1) {
                            break;
                        }
			bytes+=bytes_read;
			}
		} else if(buffer_type==ANSWER_BUFFER) {
                    String length_str = new String(version);
                    answer_length = Integer.valueOf(length_str);
                    bytes_read = 0;
                    } else {
                        status="invalid buffer type";
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
                                bytes += inputStream.read(spectrum_buffer, bytes,SPECTRUM_BUFFER_SIZE - bytes);
                            }
                            processSpectrumBuffer(version[0],version[1],spectrum_header,spectrum_buffer);
                            //processSpectrumBuffer(spectrum_header,spectrum_buffer);
                            break;
			case AUDIO_BUFFER:
                            bytes = 0;
                            while (bytes != AUDIO_BUFFER_SIZE) {
                                bytes += inputStream.read(audio_buffer, bytes, AUDIO_BUFFER_SIZE - bytes);
                            }
                            processAudioBuffer(audio_header,audio_buffer);
                            break;
			case ANSWER_BUFFER:
                            bytes = 0;
                            while (bytes != answer_length) {
                                bytes += inputStream.read(answerBuffer, bytes, answer_length - bytes);
                            }
                            processAnswerBuffer(answer_length,answerBuffer); // remove terminating null
                            break;
			default:
                            System.err.println("Client: invalid buffer_type "+ buffer_type);
                            break;
			}

                    } catch (IOException e) {
                        if(running) {
                            status="Exceptiion: "+e.getMessage();
                            System.err.println("Client.run exception reading input stream "+e.getMessage());
                            running=false;
                        }
                    }
			
            }
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

    private void processSpectrumBuffer(byte version, byte subversion, byte[] header,byte[] buffer) {
        int j;
/*
// This is the orginal John Melton calculations...
        sampleRate = ((header[8] & 0xFF) << 24) + ((header[9] & 0xFF) << 16) + ((header[10] & 0xFF) << 8) + (header[11] & 0xFF);
        meter=(short)(((header[4]&0xFF)<<8)+(header[5]&0xFF));
*/
        
        // These two lines replace the above code
	meter=getShort(header,2);
        sampleRate=getInt(header,6);

 // Added this code from glSDR to capture the LO offset
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

        int step = sampleRate / SPECTRUM_BUFFER_SIZE;
        int offset = (LO_offset / step);
        int localOscOffset = LO_offset;
        freq_offset = LO_offset;
        
        for (int i = 0; i < SAMPLES; i++) {
            samples[i] = -(buffer[i] & 0xFF);
        }
// Added "offset" in the function
        listener.updateSamples(this.getSamples(),this.getFilterLow(),this.getFilterHigh(),this.getSampleRate(),offset,localOscOffset);

    }

    private void processAudioBuffer(byte[] header,byte[] buffer) {
        audio.playAudioBuffer(buffer);
    }

// This Function is for the test case from glSDR
	private void processAnswerBuffer(int length, byte[] buffer){
		byte[] answer_buff = new byte[length];                
		for (int i = 0; i < length; i++) {
                    answer_buff[i] = buffer[i];
                }
		String full_string = new String(answer_buff);
                
//                System.err.println(full_string);
                
		if (full_string.indexOf("q-master") == 0){
			answer = full_string.substring(9);
			if (answer.indexOf("slave") != -1) {
				isSlave = true;
				hasBeenSlave = true;
                                msMode="Slave-Mode";
			}
                        else {
                            isSlave = false;
                            msMode="Master-Mode";                            
                            timer.cancel();  // added this to cancel timer if q-master
                        }
		}
		else if (full_string.indexOf("q-info") == 0 && isSlave){
			int freq_pos = full_string.indexOf(";f;");
			int mode_pos = full_string.indexOf(";m;");
			int mode_end_pos = full_string.indexOf(";", mode_pos+3);
			String freq_string = full_string.substring(freq_pos+3, mode_pos);
			this.frequency = Integer.valueOf(freq_string);
			String mode_string = full_string.substring(mode_pos+3, mode_end_pos);
			this.mode = Integer.valueOf(mode_string);
			switch (mode) {
				case 0: //LSB
					setFilter(-3450, -150);
					break;
				case 1: //USB
					setFilter(150, 3450);
					break;
				case 2: //DSB
					setFilter(-3300, 3300);
					break;
				case 3: //CWL
					setFilter(-cwPitch - 250,-cwPitch + 250);
					break;
				case 4: //CWU
					setFilter(cwPitch - 250,cwPitch + 250);
					break;
				case 5: //FMN
					setFilter(-3300, 3300);
					break;                                    
				case 6: //AM
					setFilter(-3300, 3300);
                                        break;
                                case 7: //DIGU
					setFilter(150, 3450);  
                                        break;
				case 9: //DIGL
					setFilter(-3450, -150);  
                                        break;                                    
				case 10: //SAM
					setFilter(-4000, 4000);
					break;
			}
                }
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
    
    public boolean getIsSlave() {
        return isSlave;
    } 
    
    public int getOffset() {
        return freq_offset;
    }
    
    public String getmsMode() {
        return msMode;
    }

    private static final int SPECTRUM_BUFFER=0;
    private static final int AUDIO_BUFFER=1;
    
    // Needed to add the following declarations for the new code 01-12-13
    private static final byte ANSWER_BUFFER = '4';
    private int answer_length = 0;
    
    private static final int SPECTRUM_HEADER_SIZE=12;
// Special cases
    private static final int SPECTRUM_HEADER_SIZE_2_0=10;    
    private static final int SPECTRUM_HEADER_SIZE_2_1=12;     
    private static final int AUDIO_HEADER_SIZE=2; 

    private static final int SPECTRUM_BUFFER_SIZE=480;
    
//    public static final int AUDIO_BUFFER_SIZE=480;
    public static final int AUDIO_BUFFER_SIZE=2000; //Changed from 480
 
// Added the following parameters from glSDR  01-13-13
    private String answer = "unknown";
    public boolean isSlave = false;  // Changed this to Public from private
    private boolean hasBeenSlave = false;

// Added the following Timer declaration
    private Timer timer;
    
    private Audio audio;
    private MonitorUpdateListener listener;

    private boolean running = true;

    private int limit;

    private int receiver=0;
    private int port=8000;
    // The server string below will need to by updated for the proper server
    // when deployed...
    private String server="n8mdp.dyndns.org";
//    private String server="192.168.254.53";
    private Socket socket;

    private InputStream inputStream;
    private OutputStream outputStream;

    private static final int SAMPLES=480;
    private float[] samples=new float[SAMPLES];
    
// Added this declaration for the LO offset from glSDR
    private short LO_offset;
    public int freq_offset;
    
// Added this declaration for the Master/Slave Mode identification
    public String msMode="Master-Mode";

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

class answerTask extends TimerTask {
@Override
    public void run() {
    sendCommand("q-master");
    if (getIsSlave() == true){
        sendCommand("Q-info");
    }
}
}
    
    
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