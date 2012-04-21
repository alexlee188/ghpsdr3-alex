package org.g0orx;

import android.util.Log;

public class Update extends Thread {

	public Update(Connection connection) {
	}

	public void run() {
		Log.i("Update","run");
		while (running) {
			try {
				sleep(1000 / fps);
			} catch (Exception e) {

			}
		}
		Log.i("Update","exit run");
	}

	public void close() {
		Log.i("Update","close");
		running = false;
	}

	public void setFps(int fps) {
		this.fps=fps;
	}

	private boolean running = true;
	private int fps = 10;

}
