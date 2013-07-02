/*
 * MonitorUpdateListener.java
 * Created by John Melton G0ORX
 */

/*
 * This code has been and reviewed modified.
 * John Tucker, N8MDP
 */

/*
 * Revsion History
 * 1/20/13: Updated the "public void updateSamples()" function to now include:
 *          1. offset - adjustment factor to relocate the Filter block based
 *                      on the dspserver offset value.
 *          2. localOscOffset - This is the actual dspserver offset value in Hz
 * 
 * 
 */

package jmonitor;

/**
 *
 * @author john
 */
public interface MonitorUpdateListener {

    public void updateSamples(float[] samples, int filterLow, int filterHigh, int sampleRate, int offset, int localOscOffset);
    public void updateStatus();

}
