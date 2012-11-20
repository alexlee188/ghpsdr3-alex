package org.v1al;

import java.util.Timer;
import java.util.TimerTask;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.DashPathEffect;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.View.OnTouchListener;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

public class SpectrumView extends View implements OnTouchListener {

	public SpectrumView(Context context, int width, int height) {
		super(context);
		Log.i("SpectrumView","width="+width+" height="+height);
		this.connection = null;
		paint = new Paint();
		WIDTH = width;
		HEIGHT = height;
		points = new float[WIDTH * 4];
		average= -100;
		cy = MAX_CL_HEIGHT - 1;
		this.setOnTouchListener(this);
		detector = new ScaleGestureDetector(context, new ScaleListener());
	}

	public void setConnection(Connection connection) {
		this.connection=connection;	
	}
	
	public void setAverage(int a){
		average = a;
	}
	
	void setSensors(float sensor1,float sensor2,float sensor3) {
		
		if(sensor2>(-1.9F+4.0F)) {
			connection.setFrequency((long) (connection.getFrequency() - 1000));
		} else if(sensor2>(-1.9F+3.0F)) {
			connection.setFrequency((long) (connection.getFrequency() - 100));
		} else if(sensor2>(-1.9F+2.0F)) {
			connection.setFrequency((long) (connection.getFrequency() - 10));
		} else if(sensor2<(-1.9F-4.0F)) {
			connection.setFrequency((long) (connection.getFrequency() + 1000));
		} else if(sensor2<(-1.9F-3.0F)) {
			connection.setFrequency((long) (connection.getFrequency() + 100));
		} else if(sensor2<(-1.9F-2.0F)) {
			connection.setFrequency((long) (connection.getFrequency() + 10));
		}
	}
	
	protected void onDraw(Canvas canvas) {
		if (connection.isConnected()) {
			float zoom_factor;
			
			if (connection.getIsSlave()){
				scaleFactor = (float)connection.getScaleFactor();
				mGLSurfaceView.setScaleFactor(scaleFactor);
			}
			zoom_factor = 1f + (scaleFactor - 1f)/25f;

			// draw the filter
			paint.setColor(Color.GRAY);
			paint.setTextSize(10.0F);
			filterLeft = WIDTH/2 + (int)((float)(filterLow)*(float)WIDTH*zoom_factor/(float)connection.getSampleRate());
			filterRight = WIDTH/2 + (int)((float)(filterHigh)*(float)WIDTH*zoom_factor/(float)connection.getSampleRate());
			canvas.drawRect(filterLeft+(float)offset*zoom_factor, 0,
					filterRight+(float)offset*zoom_factor, HEIGHT, paint);

			// plot the spectrum levels
			paint.setColor(Color.GRAY);
			int V = spectrumHigh - spectrumLow;
			int numSteps = V / 20;
			for (int i = 1; i < numSteps; i++) {
				int num = spectrumHigh - i * 20;
				int y = (int) Math.floor((spectrumHigh - num) * HEIGHT / V);

				paint.setColor(Color.YELLOW);
				canvas.drawLine(0, y, WIDTH, y, paint);

				paint.setColor(Color.WHITE);
				canvas.drawText(Integer.toString(num)+"dBm", 3, y-2, paint);
			}

			// plot the vertical frequency markers
			float hzPerPixel=(float)connection.getSampleRate()/(float)WIDTH/zoom_factor;
			//long f=connection.getFrequency()-(connection.getSampleRate()/2);
			String fs;
			long lineStep = 10000;
			if (connection.getSampleRate() > 1000000) lineStep = 100000;
			else if (connection.getSampleRate() > 400000) lineStep = 50000;
			else if (connection.getSampleRate() > 200000) lineStep = 20000;
			for(int i=0;i<WIDTH;i++) {
				long f=connection.getFrequency()
						-(long)((float)connection.getSampleRate()/zoom_factor/2f)
						+(long)(hzPerPixel*i)
						- connection.getLO_offset();
				if(f>0) {
					if((f%lineStep)<(long)(hzPerPixel* 1.5f)) {
						paint.setColor(Color.YELLOW);
						DashPathEffect dashPath = new DashPathEffect(new float[]{1,4}, 1);
						paint.setPathEffect(dashPath);
					    paint.setStrokeWidth(1);
						canvas.drawLine(i,35,i,HEIGHT-25,paint);
						paint.setColor(Color.WHITE);
						paint.setPathEffect(null);
						fs=String.format("%d.%02d", f/1000000, (f%1000000)/10000);
						canvas.drawText(fs, i-5, HEIGHT-5, paint);
					}
				}
			}

			// plot the cursor
			paint.setColor(Color.RED);
			canvas.drawLine((WIDTH/2)+(int)((float)offset*zoom_factor), 0,
					(WIDTH/2)+(int)((float)offset*zoom_factor), HEIGHT, paint);

			// display the frequency and mode
			paint.setColor(Color.GREEN);
			paint.setTextSize(30.0F);
			fs=String.format("%d.%03d.%03d", connection.getFrequency()/1000000,
			                                       (connection.getFrequency()%1000000)/1000,
			                                       (connection.getFrequency()%1000));
			canvas.drawText(fs + " "
					+ connection.getStringMode(), 50, 30, paint);
			
			//DEBUG
			//canvas.drawText(Integer.toString(waterfallLow), WIDTH/2, 30, paint);

			if (vfoLocked) {
				paint.setColor(Color.RED);
				canvas.drawText("LOCKED", 300, 10, paint);
			}

			// plot the spectrum
			paint.setColor(Color.WHITE);
			canvas.drawLines(points, paint);

			// draw the S-Meter
			int dbm=connection.getMeter();
			int smeter=dbm+127;
			paint.setColor(Color.GREEN);
			paint.setTextSize(16.0F);
			canvas.drawText(Integer.toString(dbm)+" dBm", WIDTH-200, 25, paint);
			paint.setColor(Color.RED);
			canvas.drawRect(WIDTH-125,10,(WIDTH-125)+smeter,25, paint);
			paint.setColor(Color.GREEN);
			int y;
			for(int i=0;i<=54;i+=6) {
				if((i%18)==0) {
					y=10;
				} else {
					y=20;
				}
				canvas.drawLine(WIDTH-125+i, 25, WIDTH-125+i, y, paint);
			}
			for(int i=10;i<=60;i+=10) {
				if((i%20)==0) {
					y=10;
				} else {
					y=20;
				}
				canvas.drawLine(WIDTH-125+54+i, 25, WIDTH-125+54+i, y, paint);
			}
			paint.setTextSize(8.0F);
			canvas.drawText("3", WIDTH-125+18-2, 35, paint);
			canvas.drawText("6", WIDTH-125+36-2, 35, paint);
			canvas.drawText("9", WIDTH-125+54-2, 35, paint);
			canvas.drawText("+20", WIDTH-125+74-4, 35, paint);
			canvas.drawText("+40", WIDTH-125+94-4, 35, paint);
			canvas.drawText("+60", WIDTH-125+114-4, 35, paint);

			String status = connection.getStatus();
			if (status != null) {
				paint.setColor(Color.RED);
				canvas.drawText(status, 0, 10, paint);
			}
			
			// draw the jog and PTT buttons
			paint.setColor(Color.DKGRAY);
			canvas.drawRect(0, HEIGHT-66, 50, HEIGHT-16, paint);
            canvas.drawRect(WIDTH-50,HEIGHT-66,WIDTH,HEIGHT-16,paint);
            canvas.drawRect(125, HEIGHT-66, 200, HEIGHT-16, paint); //kb3omm add 1000's button
            canvas.drawRect(WIDTH-200,HEIGHT-66,WIDTH-125,HEIGHT-16,paint); //kb3omm add 1000's button
            if (connection.getAllowTx()){
            	paint.setColor(connection.getMOX()? Color.RED : Color.DKGRAY);
            	canvas.drawRect(WIDTH-50, 100, WIDTH, HEIGHT-100, paint);
            }     
            paint.setColor(Color.WHITE);
            paint.setTextSize(40.0F);
            canvas.drawText("<", 12, HEIGHT-28, paint);
            canvas.drawText(">", WIDTH-36, HEIGHT-28, paint);
            canvas.drawText("<<", 142, HEIGHT-28, paint); //kb3omm add 1000's button
            canvas.drawText(">>", WIDTH-182, HEIGHT-28, paint); //kb3omm add 1000's button
			if (connection.getAllowTx()){
				canvas.drawText("T", WIDTH-36, 150, paint);
				canvas.drawText("x", WIDTH-36, 200, paint);
			}
		} else {
			paint.setColor(0xffffffff);
			canvas.drawRect(0, 0, WIDTH, HEIGHT, paint);
			paint.setColor(Color.RED);
			canvas.drawText(connection.getStatus(), 20, HEIGHT/2, paint);
		}
	}

	public void plotSpectrum(byte[] samples, int filterLow, int filterHigh,
			int sampleRate, int offset) {

		this.offset=offset;
		if (--cy < 0) cy = MAX_CL_HEIGHT - 1;  // "scroll" down one row with fast waterfall algorithm

		int p = 0;
		float sample;
		float previous = 0.0F;

		int sum = 0;
		for (int i = 0; i < WIDTH; i++) {
			sample = (float) Math
					.floor(((float) spectrumHigh - (float) (-(samples[i] & 0xFF)))
							* (float) HEIGHT
							/ (float) (spectrumHigh - spectrumLow));
			if (i == 0) {
				points[p++] = (float) i;
				points[p++] = sample;
			} else {
				points[p++] = (float) i;
				points[p++] = previous;
			}

			points[p++] = (float) i;
			points[p++] = sample;
			
			previous = sample;
			sum -= (samples[i] & 0xFF);
		}

		this.filterLow = filterLow;
		this.filterHigh = filterHigh;
		mGLSurfaceView.setFilterLow(filterLow);
		mGLSurfaceView.setFilterHigh(filterHigh);

		average = (int) ((float)average * 0.98f + (float)sum / WIDTH * 0.02f);
		waterfallLow= average -15;
		waterfallHigh=average + 45;
		
		
		if (renderer != null && mGLSurfaceView != null){
			final byte[] bitmap = new byte[WIDTH];	// GL_LUMINANCE
			for (int i = 0; i < WIDTH; i++){
				bitmap[i] = samples[i];
			}
            mGLSurfaceView.queueEvent(new Runnable() {
                // This method will be called on the rendering
                // thread:
                public void run() {
        			renderer.set_cy(cy);
        			renderer.set_waterfallHigh(waterfallHigh);
        			renderer.set_waterfallLow(waterfallLow);	
        			renderer.plotWaterfall(bitmap);
                }
            });
		}
		
		this.postInvalidate();
		mGLSurfaceView.requestRender();
	}

	public void setVfoLock() {
		vfoLocked = !vfoLocked;
	}
	
	public void setRenderer(Renderer renderer){
		this.renderer = renderer;
	}
	
	public void setGLSurfaceView(Waterfall mGLSurfaceView){
		this.mGLSurfaceView = mGLSurfaceView;
	}
	
	public void setScaleFactor(float scaleFactor){
		this.scaleFactor = scaleFactor;
	}

	public void scroll(int step) {
		if (!vfoLocked) {
			connection
					.setFrequency((long) (connection.getFrequency() + (step * (connection
							.getSampleRate() / WIDTH))));
		}
	}

	public boolean onTouch(View view, MotionEvent event) {
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
					startY = event.getY();
					moved=false;
					scroll=false;
					jog=false;
					if(startX<=50 && startY>=(HEIGHT-66) && startY <= HEIGHT) {
						// frequency down 100
						jog=true;
						jogAmount=-100;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if(startX>=(WIDTH-50) && startY>=(HEIGHT-66) && startY <= HEIGHT) {
						// frequency up 100 Hz
						jog=true;
						jogAmount=100;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if((startX<=200) && (startX>=125) && (startY>=(HEIGHT-66))
							&& (startY <= HEIGHT)) {
						// frequency down 1000 Hz kb3omm added 1k decrement
						jog=true;
						jogAmount=-1000;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if((startX<=(WIDTH-125)) && (startX>=(WIDTH-200)) 
							&& (startY>=(HEIGHT-66)) && (startY <= HEIGHT)) {
						// frequency up 1000 Hz kb3omm added 1k increment
						jog=true;
						jogAmount=1000;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if ((startX>=(WIDTH-50)) && (startY>=100) && startY <=(HEIGHT-100)
							&& connection.getAllowTx()){
						jog = true;
						if (!connection.getMOX()) connection.setMOX(true);
						else connection.setMOX(false);
					}
				}
				break;
			case MotionEvent.ACTION_MOVE:
				// Log.i("onTouch","ACTION_MOVE");
				if (connection.isConnected()) {
					if(!jog) {
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
					    if(!scroll) {
						    connection.setFrequency((long) (((connection.getFrequency() + (increment * (connection
									.getSampleRate() / WIDTH))/zoom_factor))/move_step)*move_step);
					    startX = event.getX();
	   				    moved=true;
			            } 
					}
				}
				break;
			case MotionEvent.ACTION_OUTSIDE:
				// Log.i("onTouch","ACTION_OUTSIDE");
				break;
			case MotionEvent.ACTION_UP:
				// Log.i("onTouch","ACTION_UP");
				if (connection.isConnected()) {
					if(!jog) {
						float zoom_factor = 1f + (scaleFactor - 1f)/25f;
					    int scrollAmount = (int) ((event.getX() - (WIDTH / 2)) * (int)((float)connection
							.getSampleRate() / (float)WIDTH / zoom_factor));

					    if (!moved & !scroll) {
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
					} else {
						jog=false;
						if (timer != null) timer.cancel();
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
	
	class JogTask extends TimerTask {
	    public void run() {
	    	connection.setFrequency((long) (connection.getFrequency() + jogAmount));
	    	timer.schedule(new JogTask(), 250);
	    }
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
			mGLSurfaceView.setScaleFactor(scaleFactor);
			return true;
		}
	}
	
	private Paint paint;

	private Connection connection;
	private Waterfall mGLSurfaceView;
	private Renderer renderer;

	private int WIDTH = 480;
	private int HEIGHT = 160;
	private final int MAX_CL_HEIGHT = 512;
	private float[] points;

	//Bitmap waterfall;
	//int[] pixels;
	private int cy;
	int offset;

	private int spectrumHigh = 0;
	private int spectrumLow = -140;

	private int waterfallHigh = -45;
	private int waterfallLow = -115;

	private int filterLow;
	private int filterHigh;

	private int filterLeft;
	private int filterRight;

	private boolean vfoLocked = false;

	private float startX;
	private float startY;
	private boolean moved;
	private boolean scroll;
	private boolean jog;
	
	private int average = -100;
	
	private Timer timer;
	private long jogAmount;
	
	private float scaleFactor = 1f;
	private ScaleGestureDetector detector;
	private static float MIN_ZOOM = 1f;
	private static float MAX_ZOOM = 100f;

}
