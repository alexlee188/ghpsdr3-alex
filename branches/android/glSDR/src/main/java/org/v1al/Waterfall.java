package org.v1al;


import android.content.Context;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.View.OnTouchListener;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

public class Waterfall extends GLSurfaceView implements OnTouchListener {

	public Waterfall(Context context, int width, int height) {
		super(context);
		// TODO Auto-generated constructor stub
		WIDTH = width;
		this.setOnTouchListener(this);
		detector = new ScaleGestureDetector(context, new ScaleListener());
	}

	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		detector.onTouchEvent(event);
		if (!vfoLocked) {
			switch (event.getAction()) {
			case MotionEvent.ACTION_CANCEL:
				// Log.i("onTouch","ACTION_CANCEL");
				break;
			case MotionEvent.ACTION_DOWN:
				// Log.i("onTouch","ACTION_DOWN");
				if (connection.isConnected()) {
					// connection.setStatus("onTouch.ACTION_DOWN: "+event.getX());
					startX = event.getX();
					moved=false;
				}
				break;
			case MotionEvent.ACTION_MOVE:
				// Log.i("onTouch","ACTION_MOVE");
				if (connection.isConnected()) {
					// connection.setStatus("onTouch.ACTION_MOVE: "+(int)event.getX());
					    int increment = (int) (startX - event.getX());
					    float zoom_factor = 1f + (scaleFactor - 1f)/25f;
					    float move_ratio = (float)connection.getSampleRate()/48000f/zoom_factor;
			            int move_step = 100;
			            if (move_ratio > 10.0f) move_step = 500;
			            else if (move_ratio > 5.0f) move_step = 200;
			            else if (move_ratio > 2.5f) move_step = 100;
			            else if (move_ratio > 1.0f) move_step = 50;
			            else if (move_ratio > 0.5f) move_step = 10;
			            else if (move_ratio > 0.25f) move_step = 5;
			            else move_step = 1;
						connection.setFrequency((long) (((connection.getFrequency() + (increment * (connection
									.getSampleRate() / WIDTH))/zoom_factor))/move_step)*move_step);
					    startX = event.getX();
	   				    moved=true;
			            } 
				break;
			case MotionEvent.ACTION_OUTSIDE:
				// Log.i("onTouch","ACTION_OUTSIDE");
				break;
			case MotionEvent.ACTION_UP:
				// Log.i("onTouch","ACTION_UP");
				if (connection.isConnected()) {
						float zoom_factor = 1f + (scaleFactor - 1f)/25f;
					    int scrollAmount = (int) ((event.getX() - (WIDTH / 2)) * (int)((float)connection
							.getSampleRate() / (float)WIDTH / zoom_factor));

					    if (!moved) {
						    // move this frequency to center of filter
						    if (filterHigh < 0) {
							    connection.setFrequency(connection.getFrequency()
											+ (scrollAmount 
											+ (int)((float)(filterHigh - filterLow)/zoom_factor/ 2f)));
						    } else {
							    connection.setFrequency(connection.getFrequency()
											+ (scrollAmount 
											- (int)((float)(filterHigh - filterLow)/zoom_factor / 2f)));
						    }
					    }
				}
				break;
			}
		}
		
		try {
		    Thread.sleep(50);
		} catch (Exception e) {
			System.err.println("onTouch: "+e.toString());
		}
		return true;
	}
	
	public void setConnection(Connection connection){
		this.connection = connection;
	}
	
	public void setSpectrumView(SpectrumView spectrumView){
		this.spectrumView = spectrumView;
	}
	
	public void setVfoLock() {
		vfoLocked = !vfoLocked;
	}
	
	public void setScaleFactor(float scaleFactor){
		this.scaleFactor = scaleFactor;
	}
	
	public void setFilterLow (int filterLow){
		this.filterLow = filterLow;
	}
	
	public void setFilterHigh(int filterHigh){
		this.filterHigh = filterHigh;
	}
	
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
		@Override
		public boolean onScale(ScaleGestureDetector detector) {
			float multiplier = detector.getScaleFactor();
			if (scaleFactor < 0.2f * MAX_ZOOM){
				if (multiplier > 1.0) multiplier *= 1.3f;
				else multiplier /= 1.3f;
			}
			scaleFactor *= multiplier;
			scaleFactor = Math.max(MIN_ZOOM, Math.min(scaleFactor, MAX_ZOOM));
			connection.setScaleFactor(scaleFactor);
			spectrumView.setScaleFactor(scaleFactor);
			return true;
		}
	}
	
	private Connection connection;
	private SpectrumView spectrumView;
	private float startX;
	private boolean vfoLocked = false;
	private boolean moved;
	private int WIDTH;
	private float scaleFactor = 1.0f;
	private ScaleGestureDetector detector;
	private static float MIN_ZOOM = 1f;
	private static float MAX_ZOOM = 100f;
	private int filterLow;
	private int filterHigh;
}
