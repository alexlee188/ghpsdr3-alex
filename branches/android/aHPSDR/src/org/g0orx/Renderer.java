/** 
 * The OpenGL renderer
 */

package org.g0orx;

import java.io.InputStream;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.nio.IntBuffer;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.os.SystemClock;
import android.util.Log;
import android.widget.Toast;

class Renderer implements GLSurfaceView.Renderer {
	/******************************
	 * PROPERTIES
	 ******************************/

	// Modelview/Projection matrices
	private float[] mMVPMatrix = new float[16];
	private float[] mProjMatrix = new float[16];
	private float[] mVMatrix = new float[16]; 		// modelview

	private final int MAX_CL_WIDTH = 1024;
	private final int MAX_CL_HEIGHT = 512;
	
	private Context mContext;
	private static String TAG = "Renderer";
	private int vShader = R.raw.basic_vs;
	private int fShader = R.raw.basic_fs;
	private Shader shader;
	private int _program;
	private float _cy;
	private float _LO_offset;
	private float _width = (float)640 / MAX_CL_WIDTH;
	private float _waterfallLow;
	private float _waterfallHigh;
	private int spectrumTexture_location;
	private int cy_location;
	private int offset_location;
	private int width_location;
	private int waterfallLow_location;
	private int waterfallHigh_location;
	private int uMVPMatrix_location;
	private int aPosition_location;
	private int spectrumTex;

    private ShortBuffer mIndices;
    
    private final short[] mIndicesData =
    { 
            0, 1, 2, 0, 2, 3 
    };
    

	
	/***************************
	 * CONSTRUCTOR(S)
	 **************************/
	public Renderer(Context context) {

		this.mContext = context;
		shader = new Shader(this.vShader, this.fShader, this.mContext, true, 1);
		
		mIndices = ByteBuffer.allocateDirect(mIndicesData.length * 2).order(ByteOrder.nativeOrder()).asShortBuffer();
		mIndices.put(mIndicesData).position(0);
	}

	/*****************************
	 * SET RENDER PARAMETERS
	 *****************************/
	
	public void set_cy(int cy){
		_cy = cy;
		GLES20.glUniform1f(cy_location, _cy);
	}
	
	public void set_width(int width){
		_width = width;
		GLES20.glUniform1f(width_location, _width);
	}
	
	public void set_LO_offset(float offset){
		_LO_offset = offset;
		GLES20.glUniform1f(offset_location, _LO_offset);
	}
	
	public void set_waterfallLow(int low){
		_waterfallLow = low;
		_waterfallLow /= 256.0;
		GLES20.glUniform1f(waterfallLow_location, _waterfallLow);
	}
	
	public void set_waterfallHigh(int high){
		_waterfallHigh = high;
		_waterfallHigh /= 256.0;
		GLES20.glUniform1f(waterfallHigh_location, _waterfallHigh);
	}
	
	/*****************************
	 * GL FUNCTIONS
	 ****************************/
	
	/*
	 * Draw function - called for every frame
	 */
	public void onDrawFrame(GL10 glUnused) {
		// Ignore the passed-in GL10 interface, and use the GLES20
		// class's static methods instead.
		// Load the vertex position
		float[] mVerticesData =
		    { 
		            -0.5f, 0.5f, 0.0f, // Position 0
		            0.0f, 0.0f, // TexCoord 0
		            -0.5f, -0.5f, 0.0f, // Position 1
		            0.0f, _width, // TexCoord 1
		            0.5f, -0.5f, 0.0f, // Position 2
		            1.0f, _width, // TexCoord 2
		            0.5f, 0.5f, 0.0f, // Position 3
		            1.0f, 0.0f // TexCoord 3
		    };
		
	    FloatBuffer mVertices;
		
		mVertices = ByteBuffer.allocateDirect(mVerticesData.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
		mVertices.put(mVerticesData).position(0);
		
        mVertices.position(0);
        GLES20.glVertexAttribPointer ( aPosition_location, 3, GLES20.GL_FLOAT, 
                                       false, 
                                       5 * 4, mVertices );
        // Load the texture coordinate
        mVertices.position(3);
        GLES20.glVertexAttribPointer ( spectrumTexture_location, 2, GLES20.GL_FLOAT,
                                       false, 
                                       5 * 4, 
                                       mVertices );
        GLES20.glEnableVertexAttribArray ( aPosition_location);
        GLES20.glEnableVertexAttribArray ( spectrumTexture_location );
        
        // Bind the texture
        GLES20.glActiveTexture ( GLES20.GL_TEXTURE0 );
        GLES20.glBindTexture ( GLES20.GL_TEXTURE_2D, spectrumTex );

        // Set the sampler texture unit to 0
        GLES20.glUniform1i (spectrumTexture_location, 0 );

        GLES20.glDrawElements ( GLES20.GL_TRIANGLES, 6, GLES20.GL_UNSIGNED_SHORT, mIndices );
	}

	/*
	 * Called when viewport is changed
	 * @see android.opengl.GLSurfaceView$Renderer#onSurfaceChanged(javax.microedition.khronos.opengles.GL10, int, int)
	 */
	public void onSurfaceChanged(GL10 glUnused, int width, int height) {
		GLES20.glViewport(0, 0, width, height);
		_width = (float)width / MAX_CL_WIDTH;
		float ratio = (float) width / height;
		Matrix.frustumM(mProjMatrix, 0, -ratio, ratio, -1, 1, 0.5f, 10);

		// Creating MVP matrix
		Matrix.multiplyMM(mMVPMatrix, 0, mProjMatrix, 0, mVMatrix, 0);
		// send to the shader
		GLES20.glUniformMatrix4fv(uMVPMatrix_location, 1, false, mMVPMatrix, 0);
	}

	/**
	 * Initialization function
	 */
	public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
		
		GLES20.glClearColor(.0f, .0f, .0f, 1.0f);
		GLES20.glClear( GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);

		//GLES20.glEnable   ( GLES20.GL_DEPTH_TEST );
		GLES20.glClearDepthf(1.0f);
		//GLES20.glDepthFunc( GLES20.GL_LEQUAL );
		//GLES20.glDepthMask( true );

		// cull backface
		//GLES20.glEnable( GLES20.GL_CULL_FACE );
		//GLES20.glCullFace(GLES20.GL_BACK); 

		// set the view matrix
		Matrix.setLookAtM(mVMatrix, 0, 0, 0, -5.0f, 0.0f, 0f, 0f, 0f, 1.0f, 0.0f);
		
		GLES20.glUseProgram(0);
		_program = shader.get_program();	
		// Start using the shader
		GLES20.glUseProgram(_program);
		checkGlError("glUseProgram");
		spectrumTexture_location = GLES20.glGetUniformLocation(_program, "spectrumTexture");
		GLES20.glUniform1i(spectrumTexture_location, 0);
		cy_location = GLES20.glGetUniformLocation(_program, "cy");
		offset_location = GLES20.glGetUniformLocation(_program, "offset");
		width_location = GLES20.glGetUniformLocation(_program, "width");
		waterfallLow_location = GLES20.glGetUniformLocation(_program, "waterfallLow");
		waterfallHigh_location = GLES20.glGetUniformLocation(_program, "waterfallHigh");
		aPosition_location = GLES20.glGetAttribLocation(_program, "aPosition");
		uMVPMatrix_location = GLES20.glGetUniformLocation(_program, "uMVPMatrix");
	
		spectrumTex = createTexture2D();
		
	}

	private int createTexture2D(){
		int[] textureId = new int[1];
	       // 2x2 Image, 4 bytes per pixel (R, G, B, A)
        byte[] pixels = 
            {  
                127,   0,   0, 127, // Red
                0, 127,   0, 127, // Green
                0,   0, 127, 127, // Blue
                127, 127, 0,  127  // Yellow
            };
        ByteBuffer pixelBuffer = ByteBuffer.allocateDirect(4*4);
        pixelBuffer.put(pixels).position(0);

        // Use tightly packed data
        GLES20.glPixelStorei ( GLES20.GL_UNPACK_ALIGNMENT, 1 );

        //  Generate a texture object
        GLES20.glGenTextures ( 1, textureId, 0 );

        // Bind the texture object
        GLES20.glBindTexture ( GLES20.GL_TEXTURE_2D, textureId[0] );

        //  Load the texture
        GLES20.glTexImage2D ( GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, 2, 2, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixelBuffer );

        // Set the filtering mode
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST );
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST );

		return textureId[0];
	}
	
	
	// debugging opengl
	private void checkGlError(String op) {
		int error;
		while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
			Log.e(TAG, op + ": glError " + error);
			throw new RuntimeException(op + ": glError " + error);
		}
	}

} 

// END CLASS