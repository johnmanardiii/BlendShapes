#version 330 core
layout(location = 0) in vec3 neutral;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 o;
layout(location = 4) in vec3 a;
layout(location = 5) in vec3 e;
layout(location = 6) in vec3 f;
layout(location = 7) in vec3 l;
layout(location = 8) in vec3 m;
layout(location = 9) in vec3 i;
layout(location = 10) in vec3 t_face;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform int currentChar;
uniform int nextChar;
uniform float t;

out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;

vec3 getLetterPos(int c)
{
	vec3 pos = neutral;
	switch(c) {
	case 0:	//a
		pos = a;
		break;
	case 1: // b
		pos = m;
		break;
	case 2: // c
		pos = a * .2 + e * .8;
		break;
	case 3: // d
		pos = l;
		break;
	case 4: // e
		pos = e;
		break;
	case 5: //f
		pos = f;
		break;
	case 6: //g
		pos = a * .3 + l * .7;
		break;
	case 7: //h
		pos = l * .6 + o * .4;
		break;
	case 8: //i
		pos = i;
		break;
	case 9: //j
		pos = l * .7 + a * .3;
		break;
	case 10: //k
		pos = t_face * .8 + i * .2;
		break;
	case 11: //l
		pos = l;
		break;
	case 12: //m
		pos = m;
		break;
	case 13: //n
		pos = l;
		break;
	case 14: //o
		pos = o;
		break;
	case 15: //p
		pos = m * .8 + e * .2;
		break;
	case 16: //q
		pos = l * .5 + e * .5;
		break;
	case 17: //r
		pos = t_face * .6 + i * .4;
		break;
	case 18: //s
		pos = t_face * .6 + e * .4;
		break;
	case 19: //t
		pos = t_face * .9 + i * .1;
		break;
	case 20: //u
		pos = o * .8 + l * .2;
		break;
	case 21: //v
		pos = f * .8 + e * .2;
		break;
	case 22: //w
		pos = o * .9 + l * .1;
		break;
	case 23: //x
		pos = e * .9 + l * .1;
		break;
	case 24: //y
		pos = e * .2 + o * .8;
		break;
	case 25: //z
		pos = e * .9 + l * .1;
		break;
	}
		

	return pos;
}

vec3 getCurrentFace()
{
	return getLetterPos(currentChar) * (1 - t) + getLetterPos(nextChar) * t;
}

void main()
{
	vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;

	vec3 charPos = getCurrentFace();
	vec4 tpos =  M * vec4(charPos, 1.0);
	vertex_pos = tpos.xyz;
	gl_Position = P * V * tpos;
	vertex_tex = vertTex;
}
