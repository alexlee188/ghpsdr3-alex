/** 
 * The OpenGL renderer
 */

package org.v1al;

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
	private float[] mScaleMatrix = new float[16];   // scaling
	private float[] mVMatrix = new float[16]; 		// modelview

	private final int MAX_CL_WIDTH = 2048;
	private final int MAX_CL_HEIGHT = 512;
	
	private Context mContext;
	private static String TAG = "Renderer";
	private int vShader = R.raw.basic_vs;
	private int fShader = R.raw.basic_fs;
	private Shader shader;
	private int _program;
	private int spectrumTex;
	private float _cy;
	private int cy;
	private float _LO_offset;
	private float _width = (float)720 / MAX_CL_WIDTH;
	private float width;
	private float height;
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
	private int textureCoord_location;
	
	private ShortBuffer mIndices;
	private final short[] mIndicesData =
	{ 
        0, 1, 2, 0, 2, 3 
	};
	
    private FloatBuffer mVertices;
        
	/***************************
	 * CONSTRUCTOR(S)
	 **************************/
	public Renderer(Context context) {

		this.mContext = context;
		
		shader = new Shader();
		mIndices = ByteBuffer.allocateDirect(mIndicesData.length * 2).order(ByteOrder.nativeOrder()).asShortBuffer();
		mIndices.put(mIndicesData).position(0);
		set_LO_offset(0f);
	}

	/*****************************
	 * SET RENDER PARAMETERS
	 *****************************/
	
	public void set_cy(int cy){
		this.cy = cy;
		_cy = (float)cy / MAX_CL_HEIGHT;
		GLES20.glUniform1f(cy_location, _cy);
	}
	
	public void set_width(int width){
		_width = (float)width/MAX_CL_WIDTH;
		if (_width > 1.0) _width = 1.0f;
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

		mVertices.position(0);
        GLES20.glVertexAttribPointer ( aPosition_location, 3, GLES20.GL_FLOAT, 
                                       false, 
                                       5 * 4, mVertices );
        GLES20.glEnableVertexAttribArray ( aPosition_location);
        // Load the texture coordinate
        mVertices.position(3);
        GLES20.glVertexAttribPointer ( textureCoord_location, 2, GLES20.GL_FLOAT,
                                       false, 
                                       5 * 4, 
                                       mVertices );
        GLES20.glEnableVertexAttribArray ( textureCoord_location );
        
        // Bind the texture
        GLES20.glActiveTexture ( GLES20.GL_TEXTURE0 );
        GLES20.glBindTexture ( GLES20.GL_TEXTURE_2D, spectrumTex );

        // Set the sampler texture unit to 0
        GLES20.glUniform1i (spectrumTexture_location, 0 );
        GLES20.glDrawElements ( GLES20.GL_TRIANGLES, 6, GLES20.GL_UNSIGNED_SHORT, mIndices );
        
        mVertices.clear();
	/*
        GLES20.glDisableVertexAttribArray(aPosition_location);
        GLES20.glDisableVertexAttribArray(textureCoord_location);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
	}
	*/
	}
	/*
	 * Called when viewport is changed
	 * @see android.opengl.GLSurfaceView$Renderer#onSurfaceChanged(javax.microedition.khronos.opengles.GL10, int, int)
	 */
	public void onSurfaceChanged(GL10 glUnused, int width, int height) {
		GLES20.glViewport(0, 0, width, height);
		this.width = width;
		this.height = height;
		
		set_width(width);
		
		// Ortho2D Projection
		mProjMatrix = new float []{ 2.0f/this.width, 0.0f, 0.0f, -0.0f,
						0.0f, -2.0f/this.height, 0.0f, -0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						-1.0f, 1.0f, 0.0f, 1.0f
						};
		// scaling
		Matrix.setIdentityM(mScaleMatrix, 0);
		Matrix.scaleM(mScaleMatrix, 0, -1.0f, 2.0f, 1.0f);
		Matrix.translateM(mScaleMatrix, 0, -this.width, 0.0f, 0.0f);
		// Creating MVP matrix
		Matrix.multiplyMM(mMVPMatrix, 0, mProjMatrix, 0, mScaleMatrix, 0);
		// send to the shader
		GLES20.glUniformMatrix4fv(uMVPMatrix_location, 1, false, mMVPMatrix, 0);
		
		
		// Load the vertex position
		
		float[] mVerticesData =
		    { 
		            0.0f, 0.0f, 0.0f, // Position 0
		            _width, 0.0f, // TexCoord 0
		            0.0f, this.height/2.0f, 0.0f, // Position 1
		            _width, 1.0f, // TexCoord 1
		            this.width, this.height/2.0f, 0.0f, // Position 2
		            0.0f, 1.0f, // TexCoord 2
		            this.width, 0.0f, 0.0f, // Position 3
		            0.0f, 0.0f // TexCoord 3
		    };

		mVertices = ByteBuffer.allocateDirect(mVerticesData.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
		mVertices.put(mVerticesData);	
        mVertices.flip();
		
	}

	/**
	 * Initialization function
	 */
	public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
		
		GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		GLES20.glClear( GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
		
		GLES20.glUseProgram(0);
		shader = new Shader(vShader, fShader, mContext);
		_program = shader.get_program();			
		// Start using the shader
		GLES20.glUseProgram(_program);
		checkGlError("glUseProgram");
	
		spectrumTexture_location = GLES20.glGetUniformLocation(_program, "spectrumTexture");
		GLES20.glUniform1i(spectrumTexture_location, 0);
		textureCoord_location = GLES20.glGetAttribLocation(_program, "aTextureCoord");
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
        ByteBuffer pixelBuffer = ByteBuffer.allocateDirect(MAX_CL_WIDTH * MAX_CL_HEIGHT);
        pixelBuffer.position(0);
        for (int i = 0; i < MAX_CL_HEIGHT; i++){
        	for (int j = 0; j < MAX_CL_WIDTH; j++){
        		pixelBuffer.put((byte)0xff);
        	}
        }
        pixelBuffer.position(0);
        
        //  Generate a texture object
        GLES20.glGenTextures ( 1, textureId, 0 );
        //checkGlError("GenTextures");

        // Bind the texture object
        GLES20.glBindTexture ( GLES20.GL_TEXTURE_2D, textureId[0] );

        //  Load the texture
        GLES20.glTexImage2D ( GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, MAX_CL_WIDTH, 
        		MAX_CL_HEIGHT, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, pixelBuffer);
        // Set the filtering mode
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR );
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR );
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE );
        GLES20.glTexParameteri ( GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE );
		return textureId[0];
	}
	
	
	public void plotWaterfall(final byte[] bitmap) {
		int width = bitmap.length;
		if (width > MAX_CL_WIDTH) width = MAX_CL_WIDTH;		
		ByteBuffer buffer = ByteBuffer.wrap(bitmap);
		try{
	        // Bind the texture
	        GLES20.glActiveTexture ( GLES20.GL_TEXTURE0 );
	        GLES20.glBindTexture ( GLES20.GL_TEXTURE_2D, spectrumTex );
			GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, 0, cy, width, 1, 
				GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, buffer);
			//checkGlError("glTexSubImage2D");
		} catch (Exception e){
			
		}
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
