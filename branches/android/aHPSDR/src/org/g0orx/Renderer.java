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

	private int MAX_CL_WIDTH = 4096;
	private int MAX_CL_HEIGHT = 512;
	
	private Context mContext;
	private static String TAG = "Renderer";
	private FloatBuffer rectangleVB;
	private int vShader = R.raw.basic_vs;
	private int fShader = R.raw.basic_fs;
	private Shader shader;
	private int _program;
	private int _cy;
	private float _LO_offset;
	private int _width;
	private int _height;
	private float _waterfallLow;
	private float _waterfallHigh;
	private int spectrumTexture_location;
	private int cy_location;
	private int offset_location;
	private int width_location;
	private int waterfallLow_location;
	private int waterfallHigh_location;
	private int spectrumTex;
	private ByteBuffer byteBuffer;

	/***************************
	 * CONSTRUCTOR(S)
	 **************************/
	public Renderer(Context context) {

		this.mContext = context;
		shader = new Shader(this.vShader, this.fShader, this.mContext, true, 1);
		
		byteBuffer = ByteBuffer.allocateDirect(MAX_CL_WIDTH * MAX_CL_HEIGHT *4);
		byteBuffer.order(ByteOrder.BIG_ENDIAN);
		
	}

	/*****************************
	 * SET RENDER PARAMETERS
	 *****************************/
	
	public void set_cy(int cy){
		_cy = cy;
		GLES20.glUniform1i(cy_location, _cy);
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
	

	private void initShapes(){
		float triangleCoords[] = {
				-0.5f, -0.25f, 0,
				0.5f, -0.25f, 0,
				0.0f, 0.559016994f, 0
		};
		ByteBuffer vbb = ByteBuffer.allocateDirect(triangleCoords.length * 4);
		vbb.order(ByteOrder.nativeOrder());
		rectangleVB = vbb.asFloatBuffer();
		rectangleVB.put(triangleCoords);
		rectangleVB.position(0);
		
	}
	
	/*
	 * Draw function - called for every frame
	 */
	public void onDrawFrame(GL10 glUnused) {
		// Ignore the passed-in GL10 interface, and use the GLES20
		// class's static methods instead.
	
	}

	/*
	 * Called when viewport is changed
	 * @see android.opengl.GLSurfaceView$Renderer#onSurfaceChanged(javax.microedition.khronos.opengles.GL10, int, int)
	 */
	public void onSurfaceChanged(GL10 glUnused, int width, int height) {
		GLES20.glViewport(0, 0, width, height);
		_width = width;
		_height = height;
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
		//maPositionHandle = GLES20.glGetAttribLocation(_program, "vPosition");
		//initShapes();
	
		int[] textureId = new int[1];
		GLES20.glGenTextures(1, textureId, 0);
		spectrumTex = textureId[0];
		GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, spectrumTex);
		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, _width, _height, 0,
				GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, byteBuffer);
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