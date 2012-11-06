package org.yubo.player;

public class obj {
	public static float vertices[] = { -1.0f, 1.0f, 0.0f, // 0, Top Left
			-1.0f, -1.0f, 0.0f, // 1, Bottom Left
			1.0f, -1.0f, 0.0f, // 2, Bottom Right
			1.0f, 1.0f, 0.0f, // 3, Top Right
	};

	public static short[] indices = { 0, 1, 2, 0, 2, 3 };

	public static float[] colors = { 1f, 0f, 0f, 1f, // vertex 0 red
			0f, 1f, 0f, 1f, // vertex 1 green
			0f, 0f, 1f, 1f, // vertex 2 blue
			1f, 0f, 1f, 1f, // vertex 3 magenta
	};

	public static float textureCoordinates[] = { 
		0.0f, 0.0f, 
		0.0f, 1.0f, 
		1.0f, 1.0f,
		1.0f, 0.0f};
}
