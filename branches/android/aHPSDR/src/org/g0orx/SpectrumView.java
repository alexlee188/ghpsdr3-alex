package org.g0orx;

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
import android.view.View;
import android.view.View.OnTouchListener;
import android.util.Log;

public class SpectrumView extends View implements OnTouchListener {

	public SpectrumView(Context context, int width,int height,Connection connection) {
		super(context);
Log.i("SpectrumView","width="+width+" height="+height);
		this.connection = connection;
		paint = new Paint();
		WIDTH=width;
		HEIGHT=height/2;
		points = new float[WIDTH * 4];

		waterfall = Bitmap.createBitmap(WIDTH, HEIGHT*2,
				Bitmap.Config.ARGB_8888);
		//pixels = new int[WIDTH * HEIGHT];

		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < (HEIGHT*2); y++) {
				waterfall.setPixel(x, y, Color.BLACK);
			}
		}
		
		cy = HEIGHT - 1;
		average=waterfallLow;
		this.setOnTouchListener(this);

	}

	public void setConnection(Connection connection) {
		this.connection=connection;	
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

			// draw the filter
			paint.setColor(Color.GRAY);
			paint.setTextSize(10.0F);
			canvas.drawRect(filterLeft+offset, 0, filterRight+offset, HEIGHT, paint);

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
			float hzPerPixel=(float)connection.getSampleRate()/(float)WIDTH;
			long f=connection.getFrequency()-(connection.getSampleRate()/2);
			String fs;
			for(int i=0;i<WIDTH;i++) {
				f=connection.getFrequency()-(connection.getSampleRate()/2)+(long)(hzPerPixel*i);
				if(f>0) {
					if((f%10000)<(long)hzPerPixel) {
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
			canvas.drawLine((WIDTH/2)+offset, 0, (WIDTH/2)+offset, HEIGHT, paint);

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

			// draw the waterfall
			{
				Bitmap subBitmap = Bitmap.createBitmap(waterfall, 0, cy, WIDTH, HEIGHT);
				canvas.drawBitmap(subBitmap, 1, HEIGHT, paint);

			}
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
			
			// draw the job buttons
			paint.setColor(Color.DKGRAY);
			canvas.drawRect(0, getHeight()-50, 50, getHeight(), paint);
            canvas.drawRect(getWidth()-50,getHeight()-50,getWidth(),getHeight(),paint);
            canvas.drawRect(125, getHeight()-50, 200, getHeight(), paint); //kb3omm add 1000's button
            canvas.drawRect(getWidth()-200,getHeight()-50,getWidth()-125,getHeight(),paint); //kb3omm add 1000's button
            paint.setColor(Color.WHITE);
            paint.setTextSize(40.0F);
            canvas.drawText("<", 12, getHeight()-12, paint);
            canvas.drawText(">", getWidth()-36, getHeight()-12, paint);
            canvas.drawText("<<", 142, getHeight()-12, paint); //kb3omm add 1000's button
            canvas.drawText(">>", getWidth()-182, getHeight()-12, paint); //kb3omm add 1000's button
			
		} else {
			paint.setColor(0xffffffff);
			canvas.drawRect(0, 0, canvas.getWidth(), canvas.getHeight(), paint);
			paint.setColor(Color.RED);
			//canvas.drawText("Server is busy - please wait", 20, canvas
			//		.getHeight() / 2, paint);
			canvas.drawText(connection.getStatus(), 20, canvas
					.getHeight() / 2, paint);
		}
	}

	public void plotSpectrum(int[] samples, int filterLow, int filterHigh,
			int sampleRate, int offset) {

		this.offset=offset;
		
		// scroll the waterfall down
		if(waterfall.isRecycled()) {
			waterfall = Bitmap.createBitmap(WIDTH, HEIGHT*2,
					Bitmap.Config.ARGB_8888);
			for (int x = 0; x < WIDTH; x++) {
				for (int y = 0; y < (HEIGHT*2); y++) {
					waterfall.setPixel(x, y, Color.BLACK);
				}
			}
			cy = HEIGHT - 1;
		}
		//waterfall.getPixels(pixels, 0, WIDTH, 0, 0, WIDTH, HEIGHT - 1);
		//waterfall.setPixels(pixels, 0, WIDTH, 0, 1, WIDTH, HEIGHT - 1);
		if (--cy < 0) cy = HEIGHT - 1;  // "scroll" down one row with fast waterfall algorithm

		int p = 0;
		float sample;
		float previous = 0.0F;

		average=0;
		
		for (int i = 0; i < WIDTH; i++) {
			sample = (float) Math
					.floor(((float) spectrumHigh - (float) samples[i])
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

			int pixel_value = calculatePixel(samples[i]);
			waterfall.setPixel(i, cy, pixel_value);
			waterfall.setPixel(i, cy + HEIGHT, pixel_value);
			previous = sample;
			average+=samples[i];
		}

		this.filterLow = filterLow;
		this.filterHigh = filterHigh;
		filterLeft = (filterLow - (-sampleRate / 2)) * WIDTH / sampleRate;
		filterRight = (filterHigh - (-sampleRate / 2)) * WIDTH / sampleRate;

		waterfallLow=(average/WIDTH)-5;
		waterfallHigh=waterfallLow+55;
		
		this.postInvalidate();
	}

	private int calculatePixel(float sample) {
	/*
		// simple gray scale
		int v = ((int) sample - waterfallLow) * 255
				/ (waterfallHigh - waterfallLow);

		if (v < 0)
			v = 0;
		if (v > 255)
			v = 255;

		int pixel = (255 << 24) + (v << 16) + (v << 8) + v;
		return pixel;		
	*/
		
		int colorLowR = 0;
		int colorLowG = 0;
		int colorLowB = 0;
		int colorHighR = 255;
		int colorHighG = 255;
		int colorHighB = 255;
		
	    int R,G,B;
	    if(sample<waterfallLow) {
	        R=colorLowR;
	        G=colorLowG;
	        B=colorLowB;
	    } else if(sample>waterfallHigh) {
	        R=colorHighR;
	        G=colorHighG;
	        B=colorHighB;
	    } else {
	        float range=waterfallHigh-waterfallLow;
	        float offset=sample-waterfallLow;
	        float percent=offset/range;
	        if(percent<(2.0f/9.0f)) {
	            float local_percent = percent / (2.0f/9.0f);
	            R = (int)((1.0f-local_percent)*colorLowR);
	            G = (int)((1.0f-local_percent)*colorLowG);
	            B = (int)(colorLowB + local_percent*(255-colorLowB));
	        } else if(percent<(3.0f/9.0f)) {
	            float local_percent = (percent - 2.0f/9.0f) / (1.0f/9.0f);
	            R = 0;
	            G = (int)(local_percent*255);
	            B = 255;
	        } else if(percent<(4.0f/9.0f)) {
	             float local_percent = (percent - 3.0f/9.0f) / (1.0f/9.0f);
	             R = 0;
	             G = 255;
	             B = (int)((1.0f-local_percent)*255);
	        } else if(percent<(5.0f/9.0f)) {
	             float local_percent = (percent - 4.0f/9.0f) / (1.0f/9.0f);
	             R = (int)(local_percent*255);
	             G = 255;
	             B = 0;
	        } else if(percent<(7.0f/9.0f)) {
	             float local_percent = (percent - 5.0f/9.0f) / (2.0f/9.0f);
	             R = 255;
	             G = (int)((1.0f-local_percent)*255);
	             B = 0;
	        } else if(percent<(8.0f/9.0f)) {
	             float local_percent = (percent - 7.0f/9.0f) / (1.0f/9.0f);
	             R = 255;
	             G = 0;
	             B = (int)(local_percent*255);
	        } else {
	             float local_percent = (percent - 8.0f/9.0f) / (1.0f/9.0f);
	             R = (int)((0.75f + 0.25f*(1.0f-local_percent))*255.0f);
	             G = (int)(local_percent*255.0f*0.5f);
	             B = 255;
	        }
	    }

	    int pixel = (255 << 24)+(R << 16)+(G << 8) + B;
	    return pixel;
	}

	public void setVfoLock() {
		vfoLocked = !vfoLocked;
	}

	public void scroll(int step) {
		if (!vfoLocked) {
			connection
					.setFrequency((long) (connection.getFrequency() + (step * (connection
							.getSampleRate() / WIDTH))));
		}
	}

	public boolean onTouch(View view, MotionEvent event) {
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
					if(startX<=50 && startY>=(getHeight()-50)) {
						// frequency down 100
						jog=true;
						jogAmount=-100;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if(startX>=(getWidth()-50) && startY>=(getHeight()-50)) {
						// frequency up 100 Hz
						jog=true;
						jogAmount=100;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if((startX<=200) && (startX>=125) && (startY>=(getHeight()-50))) {
						// frequency down 1000 Hz kb3omm added 1k decrement
						jog=true;
						jogAmount=-1000;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					} else if((startX<=(getWidth()-125) && (startX>=(getWidth()-200)) && startY>=(getHeight()-50))) {
						// frequency up 1000 Hz kb3omm added 1k increment
						jog=true;
						jogAmount=1000;
						connection.setFrequency((long) (connection.getFrequency() + jogAmount));
						timer=new Timer();
						timer.schedule(new JogTask(), 1000);
					}
				}
				break;
			case MotionEvent.ACTION_MOVE:
				// Log.i("onTouch","ACTION_MOVE");
				if (connection.isConnected()) {
					if(!jog) {
					// connection.setStatus("onTouch.ACTION_MOVE: "+(int)event.getX());
					    int increment = (int) (startX - event.getX());
					    if(!scroll) {
						    connection.setFrequency((long) (((connection.getFrequency() + (increment * (connection
									.getSampleRate() / WIDTH))))/100)*100); // kb3omm *100/100 min swipe increment 100hz
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
					    int scrollAmount = (int) ((event.getX() - (WIDTH / 2)) * (connection
							.getSampleRate() / WIDTH));

					    if (!moved & !scroll) {
						    // move this frequency to center of filter
						    if (filterHigh < 0) {
							    connection.setFrequency(connection.getFrequency()
											+ (scrollAmount + ((filterHigh - filterLow) / 2)));
						    } else {
							    connection.setFrequency(connection.getFrequency()
											+ (scrollAmount - ((filterHigh - filterLow) / 2)));
						    }
					    }
					} else {
						jog=false;
						timer.cancel();
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
	
	private Paint paint;

	private Connection connection;

	private int WIDTH = 480;
	private int HEIGHT = 160;

	private float[] points;

	Bitmap waterfall;
	//int[] pixels;
	private int cy;
	int offset;
	
	private int vShader;
	private int fShader;

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
	
	private int average;
	
	private Timer timer;
	private long jogAmount;

}
