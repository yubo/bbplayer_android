package org.yubo.player;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;




import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;





import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Message;
import android.view.MotionEvent;
import android.view.View;
import android.util.Log;
import android.os.SystemClock;
import android.view.KeyEvent;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Toast;

import java.lang.Math;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

import android.content.Intent;
import android.widget.SeekBar;
import android.widget.TextView;


import android.graphics.PixelFormat;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import android.opengl.GLU;
import android.opengl.GLUtils;

/*
 * NativePlayer
 */
public class NativePlayer extends Activity 
				implements SeekBar.OnSeekBarChangeListener{
	private static final String TAG = "NativePlayer";
    private long exitTime = 0;
    private NPlayerRenderer mRenderer;
    private Intent intent;
    public SeekBar mTimeBar;
    public TextView mTimeText;
    public ImageView mImageView;
    private int iseek;
    public int mDuration;
    public boolean inputfile = false;
    private SensorManager mSensorManager;
    private float sX;
    private float sY;
    

    private static native void nativeNotify(int msgid);
    private static native void nativeSeek(int duration);
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG,"onCreate");
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.main);
        
        GLSurfaceView glSurfaceView =
                (GLSurfaceView) findViewById(R.id.glsurfaceview);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        glSurfaceView.getHolder().setFormat(PixelFormat.RGBA_8888); 
        mTimeBar = (SeekBar)findViewById(R.id.timeseek);
        mTimeBar.setOnSeekBarChangeListener(this);
        mTimeText = (TextView)findViewById(R.id.timetext);
        mImageView = (ImageView)findViewById(R.id.imageView);
        
        mRenderer = new NPlayerRenderer();
        glSurfaceView.setRenderer(mRenderer);
        
        intent = getIntent();
        if ("android.intent.action.VIEW".equals(intent.getAction())) {
            mTimeBar.setVisibility(View.GONE);
            mTimeText.setVisibility(View.GONE);
            mImageView.setVisibility(View.GONE);
        	mRenderer.filename = intent.getDataString();
        	inputfile = true;
        }else{
            mTimeBar.setVisibility(View.VISIBLE);
            mTimeText.setVisibility(View.VISIBLE);
            mImageView.setVisibility(View.VISIBLE);
            mTimeBar.setMax(7199000);
            inputfile = false;
//            mTimeBar.setVisibility(View.GONE);
//            mTimeText.setVisibility(View.GONE);
//            mImageView.setVisibility(View.GONE);
//        	inputfile = true;
        }
        mRenderer.np = this;
        
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

        Log.d(TAG,"onCreate end");
    }
    
    @Override
    protected void onStart(){
    	super.onStart();
    	Log.d(TAG,"onStart");
    }

    @Override
    protected void onStop(){
    	mSensorManager.unregisterListener(mRenderer);
    	super.onStop();
    	Log.d(TAG,"onStop");
    }
    
    public boolean onTouchEvent(final MotionEvent event) {
		float X = event.getX();
		float Y = event.getY();
    	Log.d(TAG,"onTouchEvent x:["+event.getX()+"] y:["+event.getY()+"]");
    	
		switch (event.getAction()) {
		case MotionEvent.ACTION_DOWN:
			touchDown(X, Y);
			break;
		case MotionEvent.ACTION_MOVE:
			touchMove(X, Y);
			break;
		case MotionEvent.ACTION_UP:
			break;
		}
        return true;
    }

	private void touchDown(float x2, float y2) {
		// TODO Auto-generated method stub

		sX = x2;
		sY = y2;
		
        float y = y2 - mRenderer.height/2;    
        if(y<0){ // up
    		if (View.VISIBLE == mTimeBar.getVisibility()) {
    			mTimeBar.setVisibility(View.GONE);
    			mTimeText.setVisibility(View.GONE);
    		}else{
    			//mTimeBar.setMax(mRenderer.duration);
   				mTimeBar.setVisibility(View.VISIBLE);
   				mTimeText.setVisibility(View.VISIBLE);
    		}
        }else if (false){ //down
        	 nativeNotify(2);
        	 if(!inputfile)
        		 Toast.makeText(getApplicationContext(), 
                 		"请在文件浏览器中选择影片,点击播放\n yubo@yubo.org 2012-03", 
                 		Toast.LENGTH_SHORT).show();
        		 
        }
		
		
		
	}
	
	
	private void touchMove(float x2, float y2) {
		// TODO Auto-generated method stub
		float dx = x2 - sX;
		float dy = y2 - sY;

		mRenderer.setAngleY(mRenderer.getAngleY() + 45f * dx / (3.1415926f*150f));
		mRenderer.setAngleX(mRenderer.getAngleX() + 45f * dy / (3.1415926f*150f));
		sX = x2;
		sY = y2;
	}
    
    
    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG,"onPause");
    }

    @Override
    protected void onResume() {
        super.onResume();
        
        mSensorManager.registerListener(mRenderer,
                mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_NORMAL);
        mSensorManager.registerListener(mRenderer,
                mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
                SensorManager.SENSOR_DELAY_NORMAL);
        mSensorManager.registerListener(mRenderer, 
                mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION),
                SensorManager.SENSOR_DELAY_NORMAL);
        
        Log.d(TAG,"onResume");
    }

    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if(keyCode == KeyEvent.KEYCODE_BACK
        && event.getAction() == KeyEvent.ACTION_DOWN)
        {
            if((System.currentTimeMillis()-exitTime) > 2000){
                Toast.makeText(getApplicationContext(), 
                		"再按一次退出程序\n yubo@yubo.org \n 2012-03", 
                		Toast.LENGTH_SHORT).show();
                exitTime = System.currentTimeMillis();
            } else {
                finish();
                System.exit(0);
            }
            return true;
        }
        
        return super.onKeyDown(keyCode, event);
    }

    
    public void onProgressChanged(SeekBar seekBar, 
    		int progress, boolean fromTouch) {
    	mTimeText.setText(getStringTime(progress));
    }
    
    
	public static String getStringTime(int position) {
		SimpleDateFormat fmPlayTime;
		if (position <= 0) {
			return "00:00";
		}

		long lCurrentPosition = position / 1000;
		long lHours = lCurrentPosition / 3600;

		if (lHours > 0)
			fmPlayTime = new SimpleDateFormat("HH:mm:ss");
		else
			fmPlayTime = new SimpleDateFormat("mm:ss");

		fmPlayTime.setTimeZone(TimeZone.getTimeZone("GMT"));
		return fmPlayTime.format(position);
	}    

	
    public void onStartTrackingTouch(SeekBar seekBar) {
        
    }

    
    public void onStopTrackingTouch(SeekBar seekBar) {
    	if(inputfile)
    		nativeSeek(seekBar.getProgress());
    }

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("native-player");
    }
}


/*
 * NPlayerRenderer
 * 
 */
class NPlayerRenderer implements GLSurfaceView.Renderer, SensorEventListener{
	private static final String TAG = "NativePlayer:NPlayerRenderer";
	public boolean started = false;
    public int duration;
    public long time = 0;
    public short framerate = 0;
    public long fpsTime = 0;
    public long frameTime = 0;
    public float avgFPS = 0;
    public int width=0;
    public int height=0;
    public int vWidth=1;
    public int vHeight=1;
    public String filename="";
    public NativePlayer np;
    private int mSensor;
	private FloatBuffer vertexBuffer;
	private ShortBuffer indexBuffer;
	private FloatBuffer colorBuffer;
	private FloatBuffer textureBuffer;
	private int[] textures;
	private float angleX = 0;
	private float angleY = 0;
	private float angleZ = 0;
	private int angleChanged = 0;
	
	
    private static native void nativeInit(int argc, String argv);
    private static native int  nativeStart(int w, int h);
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
    private static native void nativeDone();
    private static native void nativeSetObj(NPlayerRenderer obj);
    private static native int  nativeGetClock();
    private static native int  nativeGetVw();
    private static native int  nativeGetVh();
    


    final static int NATIVE_PLAYER_MSG = 0;

	public float getAngleZ() {
		return angleZ;
	}
	


	
	public void setAngleZ(float angleZ) {
		if(angleZ != this.angleZ){
			this.angleZ = angleZ;
			angleChanged = 1;
		}
	}

	
	public float getAngleX() {
		return angleX;
	}

	public void setAngleX(float angleX) {
		if(angleX>360 || angleX < -360){
		this.angleX = angleX%360;
		}
		else{
			this.angleX = angleX;
		}
	}

	public float getAngleY() {
		return angleY;
	}

	public void setAngleY(float angleY) {
		if(angleY>360 || angleY < -360){
		this.angleY = angleY%360;
		}
		else{
			this.angleY = angleY;
		}
	}
	
	
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}
	public void onSensorChanged(SensorEvent event) {
		// TODO Auto-generated method stub
		if(event.values[1] < -45){
			if(mSensor != 2){		
				Log.d(TAG,"onSensorChanged to front");
				mSensor = 2;
				setAngleZ(90);
			}
		}else if(event.values[1] > 45){
			if(mSensor != -2){		
				Log.d(TAG,"onSensorChanged to back");
				mSensor = -2;
				setAngleZ(270);
			}		
		}else if(event.values[2] < -45){
			if(mSensor != 1){		
				Log.d(TAG,"onSensorChanged to right");
				mSensor = 1;
				setAngleZ(180);
			}			
		}else if(event.values[2] > 45){
			if(mSensor != -1){		
				Log.d(TAG,"onSensorChanged to left");
				mSensor = -1;
				setAngleZ(0);
			}		
		}
	}
	
	
    public void NativePlayerMsg(int msg){
    	Log.d(TAG,"NativePlayerMsg:"+msg);
    	switch( msg ){
    	case NATIVE_PLAYER_MSG:
    		break;
    	default:
    		break;    		
    	}
    
    } 
    
    public void SyncPlayerClock(){
    	int clock = nativeGetClock();
		Log.d(TAG,"SyncPlayerClock:"+clock);
	    np.mTimeBar.setProgress(clock);
    } 
    
    
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.d(TAG,"NPlayerRenderer onSurfaceCreated ");
        
		gl.glShadeModel(GL11.GL_SMOOTH);
		gl.glClearColor(0f, 0.0f, 0.0f, 1.0f);
		gl.glClearDepthf(1.0f);
		gl.glEnable(GL11.GL_DEPTH_TEST);
		gl.glDepthFunc(GL11.GL_LEQUAL);
		gl.glHint(GL11.GL_PERSPECTIVE_CORRECTION_HINT, GL11.GL_FASTEST);
        
		ByteBuffer vbb = ByteBuffer.allocateDirect(obj.vertices.length * 4);
		vbb.order(ByteOrder.nativeOrder());
		vertexBuffer = vbb.asFloatBuffer();
		vertexBuffer.put(obj.vertices);
		vertexBuffer.position(0);

		ByteBuffer ibb = ByteBuffer.allocateDirect(obj.indices.length * 2);
		ibb.order(ByteOrder.nativeOrder());
		indexBuffer = ibb.asShortBuffer();
		indexBuffer.put(obj.indices);
		indexBuffer.position(0);

		ByteBuffer cbb = ByteBuffer.allocateDirect(obj.colors.length * 4);
		cbb.order(ByteOrder.nativeOrder());
		colorBuffer = cbb.asFloatBuffer();
		colorBuffer.put(obj.colors);
		colorBuffer.position(0);

		textures = new int[1];
		gl.glGenTextures(1, textures, 0);

		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER,
				GL10.GL_LINEAR);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER,
				GL10.GL_LINEAR);

//		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S,
//				GL10.GL_REPEAT);
//		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T,
//				GL10.GL_REPEAT);

		//GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, OpenglActivity.bmp, 0);
		ByteBuffer byteBuf = ByteBuffer
				.allocateDirect(obj.textureCoordinates.length * 4);
		byteBuf.order(ByteOrder.nativeOrder());
		textureBuffer = byteBuf.asFloatBuffer();
		textureBuffer.put(obj.textureCoordinates);
		textureBuffer.position(0);
        
        if ("".equals(filename)) {
        	//fix me        	
        	nativeInit(2,"bbplayer NULL");
        	//nativeInit(3,"bbplayer -fast /mnt/sdcard/1.avi");
        }else{
        	nativeInit(2,"bbplayer  "+filename);
        }
    }

    
    public void onSurfaceChanged(GL10 gl, int w, int h) {
        Log.d(TAG,"NPlayerRenderer onSurfaceChanged ");
        this.width = w;
        this.height = h;
        


		//gl.glViewport(0, 0, w, h);
		//gl.glMatrixMode(GL10.GL_PROJECTION);
		//gl.glLoadIdentity();
		//GLU.gluPerspective(gl, 45.0f, (float) w / (float) h, 0.001f,
		//		10.0f);
		gl.glMatrixMode(GL10.GL_MODELVIEW);
		gl.glLoadIdentity();  
		
		
		
        
        if(np.inputfile){
	        if(!started){
	        	duration = nativeStart(w, h);
	        	vWidth = nativeGetVw();
	        	vHeight = nativeGetVh();
	        	if(angleZ == 0 || angleZ == 180){
	        		angleChange(gl,width,height,vWidth,vHeight);
	        	}else if(angleZ == 90 || angleZ == 270){
	        		angleChange(gl,width,height,vHeight,vWidth);
	        	}
	            nativeSetObj(this);
	        	np.mTimeBar.setMax(duration);
	        	np.mTimeBar.setProgress(3600000);
	            started = true;
	        }else{
	            nativeResize(w, h);
	        }
        }else{
        	//fix me
        }
    }

    private void angleChange(GL10 gl, int w, int h, int vw, int vh){
    	int height,width;
    	float aspect_ratio;
    	aspect_ratio= 1.0f;
    	aspect_ratio *= (float)vw / (float)vh;
    	height = h;
        width = ((int)Math.rint(h * aspect_ratio)) & ~1;
        if (width > w) {
            width = w;
            height = ((int)Math.rint(width / aspect_ratio)) & ~1;
        }
        gl.glViewport((w - width) / 2,  (h - height) / 2, width, height);
        Log.d(TAG,"glViewport("+(w - width) / 2+","+  (h - height) / 2+","+ width+","+ height+")");
    }
    
    public void onDrawFrame(GL10 gl) {
    	if(np.inputfile){

        time = SystemClock.uptimeMillis();

        if (time >= (frameTime + 1000.0f)) {
            frameTime = time;
            avgFPS += framerate;
            framerate = 0;
            SyncPlayerClock();
            
        }

        if (time >= (fpsTime + 3000.0f)) {
            fpsTime = time;
            avgFPS /= 3.0f;
            Log.d(TAG, "FPS: " + Float.toString(avgFPS));
            avgFPS = 0;
        }
        framerate++;
        
        
        if(angleChanged == 1){
        	angleChanged = 0;
        	if(angleZ == 0 || angleZ == 180){
        		angleChange(gl,width,height,vWidth,vHeight);
        	}else if(angleZ == 90 || angleZ == 270){
        		angleChange(gl,width,height,vHeight,vWidth);
        	}
        }
        
		gl.glEnable(GL10.GL_TEXTURE_2D);		
		nativeRender();
		gl.glDisable(GL10.GL_TEXTURE_2D);
		
		
		gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
		gl.glLoadIdentity();
		//gl.glTranslatef(0, 0, -0.001f);
		gl.glRotatef(angleY, 0f, 1f, 0f);
		gl.glRotatef(angleX, 1f, 0f, 0f);
		gl.glRotatef(angleZ, 0f, 0f, 1f);

		gl.glFrontFace(GL10.GL_CCW);
		gl.glEnable(GL10.GL_CULL_FACE);
		gl.glCullFace(GL10.GL_BACK);

		gl.glVertexPointer(3, GL10.GL_FLOAT, 0, vertexBuffer);

		gl.glEnable(GL10.GL_TEXTURE_2D);
		gl.glEnableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
		gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);

		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		gl.glTexCoordPointer(2, GL10.GL_FLOAT, 0, textureBuffer);
		gl.glDrawElements(GL10.GL_TRIANGLES, obj.indices.length,
				GL10.GL_UNSIGNED_SHORT, indexBuffer);

		gl.glDisableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
		gl.glDisable(GL10.GL_TEXTURE_2D);
		gl.glDisable(GL10.GL_CULL_FACE);
		/*************************/
		
       
    	}else{
    		//fix me
    	}
	}

}

