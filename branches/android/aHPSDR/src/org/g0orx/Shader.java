/**
 * Represents a shader object
 */

package org.g0orx;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.content.Context;
import android.opengl.GLES20;
import android.util.Log;

public class Shader {
	/************************
	 * PROPERTIES
	 **********************/

	// program/vertex/fragment handles
	private int _program, _vertexShader, _pixelShader;

	// The shaders
	private String _vertexS, _fragmentS;

	// does it have textures?
	private boolean hasTextures;
	private int numTextures;

	/************************
	 * CONSTRUCTOR(S)
	 *************************/
	public Shader() {

	}

	// Takes in Strings directly
	public Shader(String vertexS, String fragmentS, boolean hasTextures, int numTextures) {
		setup(vertexS, fragmentS, hasTextures, numTextures);
	}

	// Takes in ids for files to be read
	public Shader(int vID, int fID, Context context, boolean hasTextures, int numTextures) {
		StringBuffer vs = new StringBuffer();
		StringBuffer fs = new StringBuffer();

		// read the files
		try {
			// Read the file from the resource
			//Log.d("loadFile", "Trying to read vs");
			// Read VS first
			InputStream inputStream = context.getResources().openRawResource(vID);
			// setup Bufferedreader
			BufferedReader in = new BufferedReader(new InputStreamReader(inputStream));

			String read = in.readLine();
			while (read != null) {
				vs.append(read + "\n");
				read = in.readLine();
			}

			vs.deleteCharAt(vs.length() - 1);

			// Now read FS
			inputStream = context.getResources().openRawResource(fID);
			// setup Bufferedreader
			in = new BufferedReader(new InputStreamReader(inputStream));

			read = in.readLine();
			while (read != null) {
				fs.append(read + "\n");
				read = in.readLine();
			}

			fs.deleteCharAt(fs.length() - 1);
		} catch (Exception e) {
			Log.d("ERROR-readingShader", "Could not read shader: " + e.getLocalizedMessage());
		}


		// Setup everything
		setup(vs.toString(), fs.toString(), hasTextures, numTextures);
	}


	/**************************
	 * OTHER METHODS
	 *************************/

	/** 
	 * Sets up everything
	 * @param vs the vertex shader
	 * @param fs the fragment shader 
	 */
	private void setup(String vs, String fs, boolean hasTextures, int numTextures) {
		this._vertexS = vs;
		this._fragmentS = fs;

		// create the program
		int create = createProgram();

		// texture variables
		this.hasTextures = hasTextures;
		this.numTextures = numTextures;
	}

	/**
	 * Creates a shader program.
	 * @param vertexSource
	 * @param fragmentSource
	 * @return returns 1 if creation successful, 0 if not
	 */
	private int createProgram() {
		// Vertex shader
		_vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, _vertexS);
		if (_vertexShader == 0) {
			return 0;
		}

		// pixel shader
		_pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, _fragmentS);
		if (_pixelShader == 0) {
			return 0;
		}

		// Create the program
		_program = GLES20.glCreateProgram();
		if (_program != 0) {
			GLES20.glAttachShader(_program, _vertexShader);
			//checkGlError("glAttachShader VS " + this.toString());
			GLES20.glAttachShader(_program, _pixelShader);
			//checkGlError("glAttachShader PS");
			GLES20.glLinkProgram(_program);
			int[] linkStatus = new int[1];
			GLES20.glGetProgramiv(_program, GLES20.GL_LINK_STATUS, linkStatus, 0);
			if (linkStatus[0] != GLES20.GL_TRUE) {
				Log.e("Shader", "Could not link _program: ");
				Log.e("Shader", GLES20.glGetProgramInfoLog(_program));
				GLES20.glDeleteProgram(_program);
				_program = 0;
				return 0;
			}
		}
		else
			Log.d("CreateProgram", "Could not create program");

		return 1;
	}

	/**
	 * Loads a shader (either vertex or pixel) given the source
	 * @param shaderType VERTEX or PIXEL
	 * @param source The string data representing the shader code
	 * @return handle for shader
	 */
	private int loadShader(int shaderType, String source) {
		int shader = GLES20.glCreateShader(shaderType);
		if (shader != 0) {
			GLES20.glShaderSource(shader, source);
			GLES20.glCompileShader(shader);
			int[] compiled = new int[1];
			GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
			if (compiled[0] == 0) {
				Log.e("Shader", "Could not compile shader " + shaderType + ":");
				Log.e("Shader", GLES20.glGetShaderInfoLog(shader));
				GLES20.glDeleteShader(shader);
				shader = 0;
			}
		}
		return shader;
	}

	/**
	 * Error for OpenGL
	 * @param op
	 */
	private void checkGlError(String op) {
		int error;
		while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
			Log.e("Shader", op + ": glError " + error);
			throw new RuntimeException(op + ": glError " + error);
		}
	}

	/***************************
	 * GET/SET
	 *************************/
	public int get_program() {
		return _program;
	}

	public void set_program(int _program) {
		this._program = _program;
	}

	public int get_vertexShader() {
		return _vertexShader;
	}

	public void set_vertexShader(int shader) {
		_vertexShader = shader;
	}

	public int get_pixelShader() {
		return _pixelShader;
	}

	public void set_pixelShader(int shader) {
		_pixelShader = shader;
	}

	public String get_vertexS() {
		return _vertexS;
	}

	public void set_vertexS(String _vertexs) {
		_vertexS = _vertexs;
	}

	public String get_fragmentS() {
		return _fragmentS;
	}

	public void set_fragmentS(String _fragments) {
		_fragmentS = _fragments;
	}
}
