package org.v1al;

import android.app.Activity;



import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.Display;
import android.view.ViewGroup.LayoutParams;
import android.graphics.PixelFormat;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.FrameLayout;
import android.widget.Toast;
import android.media.AudioManager;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import org.apache.http.util.ByteArrayBuffer;
import org.v1al.SpectrumView.JogTask;

import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;
import android.util.DisplayMetrics;

public class AHPSDRActivity extends Activity implements SensorEventListener {
	/** Called when the activity is first created. */
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setTitle("glSDR: ");
		
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		height = metrics.heightPixels;
		width = metrics.widthPixels;
		// Create a new GLSurfaceView - this holds the GL Renderer
		mGLSurfaceView = new Waterfall(this, width, height);	
		// detect if OpenGL ES 2.0 support exists - if it doesn't, exit.
		if (detectOpenGLES20()) {
			// Tell the surface view we want to create an OpenGL ES 2.0-compatible
			// context, and set an OpenGL ES 2.0-compatible renderer.
			mGLSurfaceView.setEGLContextClientVersion(2);
			mGLSurfaceView.setEGLConfigChooser(true);
			mGLSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
			mGLSurfaceView.getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
			//mGLSurfaceView.setZOrderOnTop(true);
			renderer = new Renderer(this);
			mGLSurfaceView.setRenderer(renderer);
			mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		} 
		else { // quit if no support - get a better phone! :P
			this.finish();
		}
		
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        SharedPreferences prefs = getSharedPreferences("aHPSDR", 0);
        band=prefs.getInt("Band", BAND_20);
		filter=prefs.getInt("Filter",FILTER_5);
		mode=prefs.getInt("Mode",MODE_USB);
    	band_160_freq = prefs.getLong("band_160_freq", 1850000L);
    	band_80_freq = prefs.getLong("band_80_freq", 3850000L);
    	band_60_freq = prefs.getLong("band_60_freq", 5371500L);
    	band_40_freq = prefs.getLong("band_40_freq", 7050000L);
    	band_30_freq = prefs.getLong("band_30_freq", 10135000L);
    	band_20_freq = prefs.getLong("band_20_freq", 14200000L);
    	band_17_freq = prefs.getLong("band_17_freq", 18130000L);
    	band_15_freq = prefs.getLong("band_15_freq", 21270000L);
    	band_12_freq = prefs.getLong("band_12_freq", 24910000L);
    	band_10_freq = prefs.getLong("band_10_freq", 28500000L);
    	band_6_freq = prefs.getLong("band_6_freq", 50200000L);
    	band_gen_freq = prefs.getLong("band_gen_freq", 15310000L);
    	band_wwv_freq = prefs.getLong("band_wwv_freq", 10000000L);
    	long band_default_frequency = 14200000L;
    	switch (band){
    	case BAND_160:
    		band_default_frequency = prefs.getLong("band_160_freq", 1850000L);
    		break;
    	case BAND_80:
        	band_default_frequency = prefs.getLong("band_80_freq", 3850000L);
        	break;
    	case BAND_60:
        	band_default_frequency = prefs.getLong("band_60_freq", 5371500L);
        	break;
    	case BAND_40:
        	band_default_frequency = prefs.getLong("band_40_freq", 7050000L);
        	break;
    	case BAND_30:
        	band_default_frequency = prefs.getLong("band_30_freq", 10135000L);
        	break;
    	case BAND_20:
        	band_default_frequency = prefs.getLong("band_20_freq", 14200000L);
        	break;
    	case BAND_17:
        	band_default_frequency = prefs.getLong("band_17_freq", 18130000L);
        	break;
    	case BAND_15:
        	band_default_frequency = prefs.getLong("band_15_freq", 21270000L);
        	break;
    	case BAND_12:
        	band_default_frequency = prefs.getLong("band_12_freq", 24910000L);
        	break;
    	case BAND_10:
        	band_default_frequency = prefs.getLong("band_10_freq", 28500000L);
        	break;
    	case BAND_6:
        	band_default_frequency = prefs.getLong("band_6_freq", 50200000L);
        	break;
    	case BAND_GEN:
        	band_default_frequency = prefs.getLong("band_gen_freq", 15310000L);
        	break;
    	case BAND_WWV:
        	band_default_frequency = prefs.getLong("band_wwv_freq", 10000000L);
        	break;
    	}
    	frequency = prefs.getLong("Frequency", band_default_frequency);
		filterLow=prefs.getInt("FilterLow",150);
		filterHigh=prefs.getInt("FilterHigh", 2850);
		gain=prefs.getInt("Gain", 5);
		micgain=prefs.getInt("Micgain", 0);
		agc=prefs.getInt("AGC", AGC_LONG);
		fps=prefs.getInt("Fps", FPS_10);
		spectrumAverage=prefs.getInt("SpectrumAverage", 0);
		server=prefs.getString("Server", "qtradio.napan.ca");
		receiver=prefs.getInt("Receiver", 0);
		txUser=prefs.getString("txUser", "");
		txPass=prefs.getString("txPass", "");
		tx_state[0]=prefs.getBoolean("txAllow", false);
		dsp_state[3]=prefs.getBoolean("IQ", false);

		connection=null;

		spectrumView = new SpectrumView(this, width, (int)((float)height/2.3f));
		spectrumView.setRenderer(renderer);
		spectrumView.setGLSurfaceView(mGLSurfaceView);
		spectrumView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 
				ViewGroup.LayoutParams.MATCH_PARENT, 1.0f));
		
		mGLSurfaceView.setSpectrumView(spectrumView);
		mGLSurfaceView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 
				ViewGroup.LayoutParams.MATCH_PARENT, 1.0f));
			
		LinearLayout ll = new LinearLayout(this);
		ll.setOrientation(LinearLayout.VERTICAL);
		ll.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 
						ViewGroup.LayoutParams.MATCH_PARENT));
		ll.addView(spectrumView);
		ll.addView(mGLSurfaceView);
		setContentView(ll);
		
		//filterAdapter = new ArrayAdapter<String>(this, android.R.layout.select_dialog_singlechoice);
		//filterAdapter = new ArrayAdapter<String>(this, R.layout.row, R.id.filter);
		filterAdapter = new CustomAdapter(this, R.layout.row, R.id.selection);
		//serverAdapter = new ArrayAdapter<String>(this, android.R.layout.select_dialog_singlechoice);
		serverAdapter = new CustomAdapter(this, R.layout.row, R.id.selection);
	}

	@Override
    protected void onStop(){
        Log.i("AHPSDRActivity","onStop");
        super.onStop();
        boolean isSlave = connection.getHasBeenSlave();
        connection.close();

        SharedPreferences prefs = getSharedPreferences("aHPSDR", 0);
        SharedPreferences.Editor editor = prefs.edit();
        if (!isSlave){
	        editor.putInt("Band", band);
			editor.putLong("Frequency", connection.getFrequency());
	        editor.putInt("Filter", filter);
			editor.putInt("Mode", connection.getMode());
			editor.putInt("FilterLow", connection.getFilterLow());
			editor.putInt("FilterHigh", connection.getFilterHigh());
        }
    	editor.putLong("band_160_freq", band_160_freq);
    	editor.putLong("band_80_freq", band_80_freq);
    	editor.putLong("band_60_freq", band_60_freq);
    	editor.putLong("band_40_freq", band_40_freq);
    	editor.putLong("band_30_freq", band_30_freq);
    	editor.putLong("band_20_freq", band_20_freq);
    	editor.putLong("band_17_freq", band_17_freq);
    	editor.putLong("band_15_freq", band_15_freq);
    	editor.putLong("band_12_freq", band_12_freq);
    	editor.putLong("band_10_freq", band_10_freq);
    	editor.putLong("band_6_freq", band_6_freq);
    	editor.putLong("band_gen_freq", band_gen_freq);
    	editor.putLong("band_wwv_freq", band_wwv_freq);
		editor.putInt("Gain", gain);
		editor.putInt("Micgain", micgain);
		editor.putInt("AGC", agc);
		editor.putInt("Fps", fps);
		editor.putInt("SpectrumAverage", spectrumAverage);
		editor.putString("Server", server);
		editor.putInt("Receiver", receiver);
		editor.putString("txUser", txUser);
		editor.putString("txPass", txPass);
		editor.putBoolean("txAllow", tx_state[0]);
		editor.putBoolean("IQ", dsp_state[3]);
		editor.commit();
    }

	public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    public void onSensorChanged(SensorEvent event) {
    	spectrumView.setSensors(event.values[0],event.values[1],event.values[2]);
    }

	public boolean onTrackballEvent(MotionEvent event) {
		switch (event.getAction()) {
		case MotionEvent.ACTION_DOWN:
			// Log.i("onTouch","ACTION_DOWN");
			spectrumView.setVfoLock();
			mGLSurfaceView.setVfoLock();
			break;
		case MotionEvent.ACTION_MOVE:
			// Log.i("onTrackballEvent","ACTION_MOVE");
			spectrumView.scroll(-(int) (event.getX() * 6.0));
			break;
		}
		return true;
	}

	public void onStart() {
		super.onStart();
		Log.i("AHPSDR", "onStart");
		spectrumView.setAverage(-100);
	}

	public void onResume() {
		super.onResume();
		mGLSurfaceView.onResume();
		Log.i("AHPSDR", "onResume");
		//mSensorManager.registerListener(this, mGravity, SensorManager.SENSOR_DELAY_NORMAL);
		connection = new Connection(server, BASE_PORT+receiver, width);
		setConnectionDefaults();
		mySetTitle();
		spectrumView.setAverage(-100);
	}

	public void onPause() {
		super.onPause();
		mGLSurfaceView.onPause();
		Log.i("AHPSDR", "onPause");
	}

	public void onDestroy() {
		super.onDestroy();
		Log.i("AHPSDR", "onDestroy");
		//update.close();
		connection.close();
	}

	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(0, MENU_SERVERS, 0, "Servers"); //kb3omm reordered the menu to my likings
		menu.add(0, MENU_BAND, 0, "Band");
		menu.add(0, MENU_FREQUENCY, 0, "Frequency");
		menu.add(0, MENU_MODE, 0, "Mode");
		menu.add(0, MENU_FILTER, 0, "Filter");
		menu.add(0, MENU_CONNECTION,0, "Connection");
		menu.add(0, MENU_RECEIVER,0, "Receiver");
		menu.add(0, MENU_AGC, 0, "AGC");
		menu.add(0, MENU_DSP, 0, "DSP");
		menu.add(0, MENU_GAIN, 0, "GAIN");
		menu.add(0, MENU_FPS, 0, "FPS");
		menu.add(0, MENU_SPECTRUM_AVERAGE, 0, "Spectrum Average");
		menu.add(0, MENU_TX, 0, "ALLOW TX");
		menu.add(0, MENU_TX_USER, 0, "TX User Password");
		menu.add(0, MENU_MIC_GAIN, 0, "MIC GAIN");
		menu.add(0, MENU_MASTER, 0, "MASTER");
		menu.add(0, MENU_ABOUT, 0, "About");
		menu.add(0, MENU_QUIT, 0, "Quit");
		return true;
	}

	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case MENU_QUIT:
			this.finish();
			break;
	    default:
			showDialog(item.getItemId());
			break;
		}
		return true;
	}
	
	protected void onPrepareDialog(final int id, final Dialog dialog){
		switch (id){
			case MENU_SERVERS:
				try { 
		            URL updateURL = new URL("http://qtradio.napan.ca/qtradio/qtradio.pl"); 
		            URLConnection conn = updateURL.openConnection(); 
		            conn.setUseCaches(false);
		            InputStream is = conn.getInputStream(); 
		            BufferedInputStream bis = new BufferedInputStream(is); 
		            ByteArrayBuffer baf = new ByteArrayBuffer(50); 
		            
		            int current = 0; 
		            while((current = bis.read()) != -1){ 
		                baf.append((byte)current); 
		            } 
		
		            bis.close();
		  
		            String html = new String(baf.toByteArray()); 
		            
		            // need to extract out the servers addresses
		            // look for <tr><td>
		            Vector<String>temp = new Vector<String>();
		            String ip;
		            String call;
		            String clients;
		            int n=0;
		            int i=0;
		            int j;
		            serverAdapter.clear();
		            while((i=html.indexOf("<tr><td>",i))!=-1) {
		            	i+=8;
		            	j=html.indexOf("</td>",i);
		            	if(j != -1) {
		            		ip=html.substring(i,j);
		            		temp.add(ip);  
		            		i=html.indexOf("<td>",j);
		            		i+=4;
		            		j=html.indexOf("</td>",i);
		            		call=html.substring(i,j);
		            		i=j+9;
		            		i=html.indexOf("</td>",i);
		            		i+=9;
		            		i=html.indexOf("</td>",i);
		            		i+=9;
		            		i=html.indexOf("</td>",i);
		            		i+=9;
		            		i=html.indexOf("</td>",i);
		            		i+=9;
		            		j=html.indexOf("lient",i);
		            		j--;
		            		clients = html.substring(i,j);
		                    serverAdapter.add(ip+" ("+call+")"+" "+clients+"client(s)");
		            		i=j; 
		            		n++;
		            	}
		            }           
		            Log.i("servers",html);
		            servers=new CharSequence[n];
		            serverAdapter.setSelection(0);
		            for(i=0;i<n;i++) {
		            	servers[i]=temp.elementAt(i);
		            	if (servers[i].toString().equals(server)) serverAdapter.setSelection(i);
		            }
		        } catch (Exception e) {  	
		        }
				break;
			case MENU_FILTER:
				filters = null;
				switch (connection.getMode()) {
				case MODE_LSB:
				case MODE_USB:
				case MODE_DSB:
					filters = ssbFilters;
					break;
				case MODE_CWL:
				case MODE_CWU:
					filters = cwFilters;
					break;
				case MODE_FMN:
					filters = fmFilters;
					break;
				case MODE_AM:
				case MODE_DIGU:
				case MODE_DIGL:
				case MODE_SAM:
					filters = amFilters;
					break;
				case MODE_SPEC:
				case MODE_DRM:
					filters = null;
					break;
				}
				filterAdapter.clear();
				if (filters != null){
					for (int i = 0; i < 10; i++) filterAdapter.add(filters[i].toString());
					filterAdapter.setSelection(filter);
				}
				break;
			case MENU_BAND:
				if (!connection.getHasBeenSlave()){		// update band specific default freq
			    	switch (connection.getBand()){
			    	case BAND_160:
			    		band_160_freq = connection.getFrequency();
			    		break;
			    	case BAND_80:
			        	band_80_freq = connection.getFrequency();
			        	break;
			    	case BAND_60:
			        	band_60_freq = connection.getFrequency();
			        	break;
			    	case BAND_40:
			        	band_40_freq = connection.getFrequency();
			        	break;
			    	case BAND_30:
			        	band_30_freq = connection.getFrequency();
			        	break;
			    	case BAND_20:
			        	band_20_freq = connection.getFrequency();
			        	break;
			    	case BAND_17:
			        	band_17_freq = connection.getFrequency();
			        	break;
			    	case BAND_15:
			        	band_15_freq = connection.getFrequency();
			        	break;
			    	case BAND_12:
			        	band_12_freq = connection.getFrequency();
			        	break;
			    	case BAND_10:
			        	band_10_freq = connection.getFrequency();
			        	break;
			    	case BAND_6:
			        	band_6_freq = connection.getFrequency();
			        	break;
			    	case BAND_GEN:
			        	band_gen_freq = connection.getFrequency();
			        	break;
			    	case BAND_WWV:
			        	band_wwv_freq = connection.getFrequency();
			        	break;
			    	}
				}
				break;	
		}
		spectrumView.setAverage(-100);	
	}

	protected Dialog onCreateDialog(final int id) {
		Dialog dialog;
		AlertDialog.Builder builder;

		dialog = null;
		switch (id) {
		case MENU_CONNECTION:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Enter server name or ip address.");
			final EditText input = new EditText(this);
			input.setText(server);
			builder.setView(input);
			builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					String value = input.getText().toString().trim();
					Log.i("Server",value);
					mode=connection.getMode();
					frequency=connection.getFrequency();
					band = connection.getBand();
					filterLow=connection.getFilterLow();
					filterHigh=connection.getFilterHigh();
					connection.close();
					server=value;	
					connection = new Connection(server, BASE_PORT + receiver,width);
					setConnectionDefaults();
					mySetTitle();
					dialog.dismiss();
				}
			});
			dialog = builder.create();
			break;
		case MENU_SERVERS:
            builder = new AlertDialog.Builder(this);
			builder.setTitle("Select a Server");
			builder.setAdapter(serverAdapter,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							Log.i("selected",servers[item].toString());
							mode=connection.getMode();
							frequency=connection.getFrequency();
							band=connection.getBand();
							filterLow=connection.getFilterLow();
							filterHigh=connection.getFilterHigh();
							connection.close();
							server=servers[item].toString();	
							connection = new Connection(server, BASE_PORT + receiver,width);
							setConnectionDefaults();
							mySetTitle();
							dialog.dismiss();
						}
			});       
			dialog = builder.create();
			break;
		case MENU_RECEIVER:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select Reveiver");
			builder.setSingleChoiceItems(receivers, receiver,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							Log.i("Receiver",Integer.toString(item));
							mode=connection.getMode();
							frequency=connection.getFrequency();
							filterLow=connection.getFilterLow();
							filterHigh=connection.getFilterHigh();
							connection.close();
							receiver=item;	
							connection = new Connection(server, BASE_PORT + receiver,width);
							setConnectionDefaults();
							mySetTitle();
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;

        case MENU_FREQUENCY:
            builder = new AlertDialog.Builder(this);
            builder.setTitle("Enter frequency (in Hz):");
            final EditText freq = new EditText(this);
            builder.setView(freq);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                            String value = freq.getText().toString().trim();
                            Log.i("Frequency",value);
                            connection.setFrequency(Long.parseLong(value));
                            dialog.dismiss();
                    }
            });
            dialog = builder.create();
            break;
        case MENU_BAND:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select a Band");
			builder.setSingleChoiceItems(bands, band,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							//
							if (item != BAND_RESET) {
								band = item;
							} else {  // BAND_RESET
								band_160_freq = 1850000L;
								band_80_freq = 3850000L;
								band_60_freq = 5371500L;
								band_40_freq = 7050000L;
								band_30_freq = 10135000L;
								band_20_freq = 14200000L;
								band_17_freq = 18130000L;
								band_15_freq = 21270000L;
								band_12_freq = 24910000L;
								band_10_freq = 28500000L;
								band_6_freq = 50200000L;
								band_gen_freq = 15310000L;
								band_wwv_freq = 10000000L;
							}
							switch (band) {
							case BAND_160:
								connection.setMode(MODE_LSB);
								connection.setFilter(-2850, -150);
								connection.setFrequency(band_160_freq);
								break;
							case BAND_80:
								connection.setMode(MODE_LSB);
								connection.setFilter(-2850, -150);
								connection.setFrequency(band_80_freq);
								break;
							case BAND_60:
								connection.setMode(MODE_LSB);
								connection.setFilter(-2850, -150);
								connection.setFrequency(band_60_freq);
								break;
							case BAND_40:
								connection.setMode(MODE_LSB);
								connection.setFilter(-2850, -150);
								connection.setFrequency(band_40_freq);
								break;
							case BAND_30:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_30_freq);
								break;
							case BAND_20:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_20_freq);
								break;
							case BAND_17:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_17_freq);
								break;
							case BAND_15:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_15_freq);
								break;
							case BAND_12:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_12_freq);
								break;
							case BAND_10:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_10_freq);
								break;
							case BAND_6:
								connection.setMode(MODE_USB);
								connection.setFilter(150, 2850);
								connection.setFrequency(band_6_freq);
								break;
							case BAND_GEN:
								connection.setMode(MODE_SAM);
								connection.setFilter(-4000, 4000);
								connection.setFrequency(band_gen_freq);
								break;
							case BAND_WWV:
								connection.setMode(MODE_AM);
								connection.setFilter(-4000, 4000);
								connection.setFrequency(band_wwv_freq);
								break;
							}
							connection.setBand(band);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_MODE:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select a Mode");
			builder.setSingleChoiceItems(modes, connection.getMode(),
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							mode=item;
							connection.setMode(mode);
							filter = FILTER_5;
							switch (item) {
							case MODE_LSB:
								connection.setFilter(-2850, -150);
								break;
							case MODE_USB:
								connection.setFilter(150, 2850);
								break;
							case MODE_DSB:
								connection.setFilter(-2600, 2600);
								break;
							case MODE_CWL:
								connection.setFilter(-800, -400);
								break;
							case MODE_CWU:
								connection.setFilter(400, 800);
								break;
							case MODE_FMN:
								connection.setFilter(-2600, 2600);
								break;
							case MODE_AM:
								connection.setFilter(-4000, 4000);
								break;
							case MODE_DIGU:
								connection.setFilter(150, 3450);
								break;
							case MODE_SPEC:
								connection.setFilter(-6000, 6000);
								break;
							case MODE_DIGL:
								connection.setFilter(-3450, -150);
								break;
							case MODE_SAM:
								connection.setFilter(-4000, 4000);
								break;
							case MODE_DRM:
								connection.setFilter(-6000, 6000);
								break;
							}
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_FILTER:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select Filter");
			filters = null;
			switch (connection.getMode()) {
			case MODE_LSB:
			case MODE_USB:
			case MODE_DSB:
				filters = ssbFilters;
				break;
			case MODE_CWL:
			case MODE_CWU:
				filters = cwFilters;
				break;
			case MODE_FMN:
				filters = fmFilters;
				break;
			case MODE_AM:
			case MODE_DIGU:
			case MODE_DIGL:
			case MODE_SAM:
				filters = amFilters;
				break;
			case MODE_SPEC:
			case MODE_DRM:
				filters = null;
				break;
			}
			if (filters != null) {
				filterAdapter.clear();
				for (int i = 0; i < 10; i++)
					filterAdapter.add(filters[i].toString());
				builder.setAdapter(filterAdapter,
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int item) {
								filter=item;
								switch (filter) {
								case FILTER_0:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-5150, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 5150);
										break;
									case MODE_DSB:
										connection.setFilter(5000, 5000);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 500,
												-cwPitch + 500);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 500,
												cwPitch + 500);
										break;
									case MODE_FMN:
										connection.setFilter(-40000, 40000);
										break;
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-8000, 8000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_1:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-4550, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 4550);
										break;
									case MODE_DSB:
										connection.setFilter(-4400, 4400);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 400,
												-cwPitch + 400);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 400,
												cwPitch + 400);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-6000, 6000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_2:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-3950, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 3950);
										break;
									case MODE_DSB:
										connection.setFilter(-3800, 3800);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 375,
												-cwPitch + 375);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 375,
												cwPitch + 375);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-5000, 5000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_3:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-3450, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 3450);
										break;
									case MODE_DSB:
										connection.setFilter(-3300, 3300);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 300,
												-cwPitch + 300);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 300,
												cwPitch + 300);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-4000, 4000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_4:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-3050, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 3050);
										break;
									case MODE_DSB:
										connection.setFilter(-2900, 2900);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 250,
												-cwPitch + 250);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 250,
												cwPitch + 250);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-3300, 3300);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_5:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-2850, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 2850);
										break;
									case MODE_DSB:
										connection.setFilter(-2700, 2700);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 200,
												-cwPitch + 200);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 200,
												cwPitch + 200);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-2600, 2600);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_6:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-2550, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 2550);
										break;
									case MODE_DSB:
										connection.setFilter(-2400, 2400);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 125,
												-cwPitch + 125);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 125,
												cwPitch + 125);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-2000, 2000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_7:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-2250, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 2250);
										break;
									case MODE_DSB:
										connection.setFilter(-2100, 2100);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 50,
												-cwPitch + 50);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 50,
												cwPitch + 50);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-1550, 1550);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_8:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-1950, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 1950);
										break;
									case MODE_DSB:
										connection.setFilter(-1800, 1800);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 25,
												-cwPitch + 25);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 25,
												cwPitch + 25);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-1450, 1450);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								case FILTER_9:
									switch (connection.getMode()) {
									case MODE_LSB:
										connection.setFilter(-1150, -150);
										break;
									case MODE_USB:
										connection.setFilter(150, 1150);
										break;
									case MODE_DSB:
										connection.setFilter(-1000, 1000);
										break;
									case MODE_CWL:
										connection.setFilter(-cwPitch - 12,
												-cwPitch + 12);
										break;
									case MODE_CWU:
										connection.setFilter(cwPitch - 12,
												cwPitch + 12);
										break;
									case MODE_FMN:
									case MODE_AM:
									case MODE_DIGU:
									case MODE_DIGL:
									case MODE_SAM:
										connection.setFilter(-1000, 1000);
										break;
									case MODE_SPEC:
										break;
									case MODE_DRM:
										break;
									}
									break;
								}
								dialog.dismiss();
							}
						});
				dialog = builder.create();
			}
			break;
		case MENU_AGC:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select AGC");
			builder.setSingleChoiceItems(agcs, agc,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							//
							agc=item;
							connection.setAGC(agc);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_DSP:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select DSP");
			builder.setMultiChoiceItems(dsps, dsp_state,
					new DialogInterface.OnMultiChoiceClickListener() {
						public void onClick(DialogInterface dialog, int item,
								boolean state) {
							//
							switch (item) {
							case DSP_NR:
								connection.setNR(state);
								break;
							case DSP_ANF:
								connection.setANF(state);
								break;
							case DSP_NB:
								connection.setNB(state);
								break;
							case DSP_IQ:
								connection.setIQCorrection(state);
								break;
							}

							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_TX:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Configure Tx");
			builder.setMultiChoiceItems(txs, tx_state,
					new DialogInterface.OnMultiChoiceClickListener() {
						public void onClick(DialogInterface dialog, int item,
								boolean state) {
							//
							switch (item) {
							case TX_ALLOW:
								connection.setAllowTx(state);
								break;
							}

							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
        case MENU_TX_USER:
            builder = new AlertDialog.Builder(this);
            builder.setTitle("Enter TX User and Password:");
            LinearLayout ll = new LinearLayout(this);
            ll.setOrientation(1); // vertical
            final EditText user = new EditText(this);
            final EditText pass = new EditText(this);
            user.setText(txUser);
            pass.setText(txPass);
            ll.addView(user);
            ll.addView(pass);
            builder.setView(ll);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                            txUser = user.getText().toString().trim();
                            txPass = pass.getText().toString().trim();
                            connection.setTxUser(txUser);
                            connection.setTxPass(txPass);
                            dialog.dismiss();
                    }
            });
            dialog = builder.create();
			spectrumView.setAverage(-100);
            break;
        case MENU_MASTER:
        	connection.setMaster();
        	break;
		case MENU_GAIN:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select Gain");
			builder.setSingleChoiceItems(gains, gain,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							gain=item;
							connection.setGain(gain*10);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_MIC_GAIN:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select Mic Gain");
			builder.setSingleChoiceItems(micgains, micgain,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							micgain=item;
							connection.setMicGain(micgain);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_FPS:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Select FPS");
			builder.setSingleChoiceItems(fpss, fps,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							fps=item;
							connection.getSpectrum_protocol3(fps+1);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_SPECTRUM_AVERAGE:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Set Spectrum Averaging");
			builder.setSingleChoiceItems(spectrumAverages, spectrumAverage,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int item) {
							spectrumAverage=item;
							connection.setSpectrumAverage(item);
							dialog.dismiss();
						}
					});
			dialog = builder.create();
			break;
		case MENU_ABOUT:
			AboutDialog about = new AboutDialog(this);
			about.setTitle("About glSDR");
			about.show();
			break;
		default:
			dialog = null;
			break;
		}
		return dialog;
	}
	
	/**
	 * Detects if OpenGL ES 2.0 exists
	 * @return true if it does
	 */
	private boolean detectOpenGLES20() {
		ActivityManager am =
			(ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
		ConfigurationInfo info = am.getDeviceConfigurationInfo();
		Log.d("OpenGL Ver:", info.getGlEsVersion());
		return (info.reqGlEsVersion >= 0x20000);
	}
	
	private void setConnectionDefaults(){
		boolean result;
		if (timer != null) timer.cancel();
		connection.setSpectrumView(spectrumView);
		result = connection.connect();
		if (!result){	
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
			// set title
			alertDialogBuilder.setTitle("Server Unavailable");
			// set dialog message
			alertDialogBuilder
				.setMessage("The selected Server is unavailable.  Please use MENU (looks like 3 dots at bottom of your device) to select another Server.")
				.setCancelable(false)
				.setPositiveButton("OK",new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog,int id) {
						// if this button is clicked, close
						// current activity
						if (connection != null) connection.close();
					}
				  });
				// create alert dialog
				AlertDialog alertDialog = alertDialogBuilder.create();
				// show it
				alertDialog.show();
		};
		connection.start();
		connection.sendCommand("q-master");
	    connection.sendCommand("setClient glSDR(33)");
		connection.setFrequency(frequency);
		connection.setMode(mode);
		connection.setBand(band);
		connection.setFilter(filterLow, filterHigh);
		connection.setGain(gain*10);
		connection.setMicGain(micgain);
		connection.setAGC(agc);
	    connection.setAllowTx(tx_state[0]);
	    connection.setTxUser(txUser);
	    connection.setTxPass(txPass);
	    connection.setIQCorrection(dsp_state[3]);					
		spectrumView.setConnection(connection);
		mGLSurfaceView.setConnection(connection);
		spectrumView.setAverage(-100);
		connection.setFps(fps);
		connection.setSpectrumAverage(spectrumAverage);
		connection.getSpectrum_protocol3(fps+1);
		connection.setScaleFactor(1f);
		connection.setHasBeenSlave(false);
		timer = new Timer();
		timer.schedule(new answerTask(), 1000, 1000);
	}
	
	private void mySetTitle(){
		setTitle("glSDR: "+server+" (rx"+receiver+") "+qAnswer);
		mHandler.removeCallbacks(updateTitle);
		mHandler.postDelayed(updateTitle, 500);
	}
	
	private Runnable updateTitle = new Runnable() {
		public void run(){
			setTitle("glSDR: "+server+" (rx"+receiver+") "+qAnswer);
		}
	};
	

	class answerTask extends TimerTask {
	    public void run() {
			if (connection != null){
				qAnswer = connection.getAnswer();
				connection.sendCommand("q-master");
				if (connection.getIsSlave() == true){
					connection.sendCommand("q-info");
				}
				mHandler.removeCallbacks(updateTitle);
				mHandler.postDelayed(updateTitle, 500);
			}
	    }
	}
	
	private Timer timer;
	private Handler mHandler = new Handler();
	private int width;
	private int height;

	private SensorManager mSensorManager;
    private Sensor mGravity;

	private Connection connection;
	private SpectrumView spectrumView;

	public static final CharSequence[] receivers = { "0", "1", "2", "3" };
	
    public int receiver = RX_0;
    
	public static final int RX_0 = 0;
	public static final int RX_1 = 1;
	public static final int RX_2 = 2;
	public static final int RX_3 = 3;
	
	public static final int MENU_QUIT = 0;
	public static final int MENU_BAND = 1;
	public static final int MENU_MODE = 2;
	public static final int MENU_FILTER = 3;
	public static final int MENU_AGC = 4;
	public static final int MENU_DSP = 5;
	public static final int MENU_GAIN = 6;
	public static final int MENU_FPS = 7;
	public static final int MENU_CONNECTION = 8;
	public static final int MENU_RECEIVER = 9;
	public static final int MENU_FREQUENCY = 10;
	public static final int MENU_SERVERS = 11;
	public static final int MENU_TX = 12;
	public static final int MENU_TX_USER = 13;
	public static final int MENU_MASTER = 14;
	public static final int MENU_MIC_GAIN = 15;
	public static final int MENU_SPECTRUM_AVERAGE = 16;
	public static final int MENU_ABOUT = 17;

	public static final CharSequence[] bands = { "160", "80", "60", "40", "30",
			"20", "17", "15", "12", "10", "6", "GEN", "WWV", "Reset" };

	private int band = BAND_20;
	private long frequency=14200000L;
	
	private long band_160_freq = 1850000L;
	private long band_80_freq = 3850000L;
	private long band_60_freq = 5371500L;
	private long band_40_freq = 7050000L;
	private long band_30_freq = 10135000L;
	private long band_20_freq = 14200000L;
	private long band_17_freq = 18130000L;
	private long band_15_freq = 21270000L;
	private long band_12_freq = 24910000L;
	private long band_10_freq = 28500000L;
	private long band_6_freq = 50200000L;
	private long band_gen_freq = 15310000L;
	private long band_wwv_freq = 10000000L;

	public static final int BAND_160 = 0;
	public static final int BAND_80 = 1;
	public static final int BAND_60 = 2;
	public static final int BAND_40 = 3;
	public static final int BAND_30 = 4;
	public static final int BAND_20 = 5;
	public static final int BAND_17 = 6;
	public static final int BAND_15 = 7;
	public static final int BAND_12 = 8;
	public static final int BAND_10 = 9;
	public static final int BAND_6 = 10;
	public static final int BAND_GEN = 11;
	public static final int BAND_WWV = 12;
	public static final int BAND_RESET = 13;

	private int mode = MODE_USB;
	
	public static final CharSequence[] modes = { "LSB", "USB", "DSB", "CWL",
			"CWU", "FMN", "AM", "DIGU", "SPEC", "DIGL", "SAM", "DRM" };
	public static final int MODE_LSB = 0;
	public static final int MODE_USB = 1;
	public static final int MODE_DSB = 2;
	public static final int MODE_CWL = 3;
	public static final int MODE_CWU = 4;
	public static final int MODE_FMN = 5;
	public static final int MODE_AM = 6;
	public static final int MODE_DIGU = 7;
	public static final int MODE_SPEC = 8;
	public static final int MODE_DIGL = 9;
	public static final int MODE_SAM = 10;
	public static final int MODE_DRM = 11;

	public static final CharSequence[] agcs = { "OFF", "LONG", "SLOW",
			"MEDIUM", "FAST" };
	private int agc = AGC_LONG;

	public static final int AGC_OFF = 0;
	public static final int AGC_LONG = 1;
	public static final int AGC_SLOW = 2;
	public static final int AGC_MEDIUM = 3;
	public static final int AGC_FAST = 4;

	public static final CharSequence[] dsps = { "NR", "ANF", "NB", "IQ CORRECTION" };

	public static final int DSP_NR = 0;
	public static final int DSP_ANF = 1;
	public static final int DSP_NB = 2;
	public static final int DSP_IQ = 3;

	private boolean[] dsp_state = { false, false, false, false };
	
	public static final CharSequence[] txs = { "Allow Tx" };
	public static final int TX_ALLOW = 0;
	public boolean[] tx_state = { false };

	public static final CharSequence[] gains = { "0", "10", "20", "30", "40",
			"50", "60", "70", "80", "90", "100" };

	public int gain = 5;
	
	public static final CharSequence[] micgains = { "default", "x2", "x4", "x8", "x16", "x32", "x64" };

	public int micgain = 0;
	
    public static final CharSequence[] fpss = { "1", "2", "3", "4", "5",
		"6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
	
    public int fps = FPS_10;
    
	public static final int FPS_1 = 0;
	public static final int FPS_2 = 1;
	public static final int FPS_3 = 2;
	public static final int FPS_4 = 3;
	public static final int FPS_5 = 4;
	public static final int FPS_6 = 5;
	public static final int FPS_7 = 6;
	public static final int FPS_8 = 7;
	public static final int FPS_9 = 8;
	public static final int FPS_10 = 9;
	public static final int FPS_11 = 10;
	public static final int FPS_12 = 11;
	public static final int FPS_13 = 12;
	public static final int FPS_14 = 13;
	public static final int FPS_15 = 14;
	
	public static final CharSequence[] spectrumAverages = { "0", "1", "2", "3", "4", "5", "6", "7", "8"};
	
	private int spectrumAverage = 0;

	public static final CharSequence[] ssbFilters = { "5.0k", "4.4k", "3.8k",
			"3.3k", "2.9k", "2.7k", "2.4k", "2.1k", "1.8k", "1.0k" };
	public static final CharSequence[] cwFilters = { "1.0k", "800", "750",
			"600", "500", "400", "250", "100", "50", "25" };
	public static final CharSequence[] amFilters = { "16.0k", "12.0k", "10.0k",
			"8.0k", "6.6k", "5.2k", "4.0k", "3.1k", "2.9k", "2.0k" };
	public static final CharSequence[] fmFilters = { "80.0k", "12.0k", "10.0k",
		"8.0k", "6.6k", "5.2k", "4.0k", "3.1k", "2.9k", "2.0k" };

	private int filter = FILTER_5;
	private CharSequence[] filters;
	private int filterLow=150;
	private int filterHigh=2875;
	private CustomAdapter filterAdapter;

	public static final int FILTER_0 = 0;
	public static final int FILTER_1 = 1;
	public static final int FILTER_2 = 2;
	public static final int FILTER_3 = 3;
	public static final int FILTER_4 = 4;
	public static final int FILTER_5 = 5;
	public static final int FILTER_6 = 6;
	public static final int FILTER_7 = 7;
	public static final int FILTER_8 = 8;
	public static final int FILTER_9 = 9;

	private int cwPitch = 600;

	private String server = "qtradio.napan.ca";
	private String qAnswer = "";
	private int BASE_PORT = 8000;
	private int port = 8000;
	private CustomAdapter serverAdapter;
	private CharSequence servers[];
	 
	private String txUser = "";
	private String txPass = "";
	
	
	private Waterfall mGLSurfaceView = null;
	// The Renderer
	Renderer renderer = null;

}
