/*
 * MonitorUpdateThread.java
 * Created by John Melton G0ORX
 */

/*
 * This code has been and reviewed modified.
 * John Tucker, N8MDP
 */

/*
 * Revsion History
 * 1/20/13: No Changes
 * 
 * 
 */

package jmonitor;

public class MonitorUpdateThread extends Thread {

    MonitorUpdateThread(Client client,MonitorUpdateListener listener) {
        this.client=client;
        this.listener=listener;
    }

    public void run() {
        System.err.println("MonitorUpdateThread.run");
        while(true) {
            client.getSpectrum(listener);
            try {
                sleep(1000/fps);
            } catch (InterruptedException e) {
                System.err.println("MonitorUpdateThread: InterruptedException: "+e.getMessage());
            }
        }
        
    }

    public void setFps(int fps) {
        this.fps=fps;
    }

    private Client client;
    private MonitorUpdateListener listener;
    private int fps=10;

}
