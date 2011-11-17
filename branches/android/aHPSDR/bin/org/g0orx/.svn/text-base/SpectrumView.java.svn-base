package org.g0orx;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.DashPathEffect;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class SpectrumView extends View implements OnTouchListener {

	public SpectrumView(Context context, int width,int height,Connection connection) {
		super(context);
System.err.println("SpectrumView: width="+width+" height="+height);
		this.connection = connection;
		paint = new Paint();
		WIDTH=width;
		HEIGHT=height/2;
		points = new float[WIDTH * 4];

		waterfall = Bitmap.createBitmap(WIDTH, HEIGHT,
				Bitmap.Config.ARGB_8888);
		pixels = new int[WIDTH * HEIGHT];

		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				waterfall.setPixel(x, y, Color.BLACK);
			}
		}
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
			canvas.drawRect(filterLeft, 0, filterRight, HEIGHT, paint);

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
			canvas.drawLine(WIDTH/2, 0, WIDTH/2, HEIGHT, paint);

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
			canvas.drawBitmap(waterfall, 1, HEIGHT, paint);

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
			int sampleRate) {

		// scroll the waterfall down
		waterfall.getPixels(pixels, 0, WIDTH, 0, 0, WIDTH, HEIGHT - 1);
		waterfall.setPixels(pixels, 0, WIDTH, 0, 1, WIDTH, HEIGHT - 1);

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

			waterfall.setPixel(i, 0, calculatePixel(samples[i]));

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
		// simple gray scale
		int v = ((int) sample - waterfallLow) * 255
				/ (waterfallHigh - waterfallLow);

		if (v < 0)
			v = 0;
		if (v > 255)
			v = 255;

		int pixel = (255 << 24) + (v << 16) + (v << 8) + v;
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
					moved=false;
					scroll=false;
				}
				break;
			case MotionEvent.ACTION_MOVE:
				// Log.i("onTouch","ACTION_MOVE");
				if (connection.isConnected()) {
					// connection.setStatus("onTouch.ACTION_MOVE: "+(int)event.getX());
					int increment = (int) (startX - event.getX());
					if(!scroll) {
						connection.setFrequency((long) (connection.getFrequency() + (increment * (connection
									.getSampleRate() / WIDTH))));
					startX = event.getX();
					moved=true;
			        } 
				}
				break;
			case MotionEvent.ACTION_OUTSIDE:
				// Log.i("onTouch","ACTION_OUTSIDE");
				break;
			case MotionEvent.ACTION_UP:
				// Log.i("onTouch","ACTION_UP");
				if (connection.isConnected()) {
					int scrollAmount = (int) ((event.getX() - (WIDTH / 2)) * (connection
							.getSampleRate() / WIDTH));

					if (!moved & !scroll) {
						// move this frequency to center of filter
						if (filterHigh < 0) {
							connection
									.setFrequency(connection.getFrequency()
											+ (scrollAmount + ((filterHigh - filterLow) / 2)));
						} else {
							connection
									.setFrequency(connection.getFrequency()
											+ (scrollAmount - ((filterHigh - filterLow) / 2)));
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
	
	private Paint paint;

	private Connection connection;

	private int WIDTH = 480;
	private int HEIGHT = 160;

	private float[] points;

	Bitmap waterfall;
	int[] pixels;

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
	private boolean moved;
	private boolean scroll;
	
	private int average;

}
