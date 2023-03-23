
#version 430

uniform vec3 pointSize;

layout(points) in;
layout(triangle_strip, max_vertices = 36) out;

void main()
{
    vec4 pos = gl_in[0].gl_Position;

    //front
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, pointSize.z, 0.0)); //tl
    EmitVertex();


    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, pointSize.z, 0.0)); //br
    EmitVertex();


    //back
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, -pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, -pointSize.z, 0.0)); //br
    EmitVertex();


    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, -pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //tl
    EmitVertex();


    //right
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, pointSize.z, 0.0)); //br
    EmitVertex();


    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, -pointSize.z, 0.0)); //tl
    EmitVertex();


    //left
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, pointSize.z, 0.0)); //br
    EmitVertex();


    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, -pointSize.z, 0.0)); //tl
    EmitVertex();


    //top
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, pointSize.z, 0.0)); //br
    EmitVertex();


    gl_Position = (pos + vec4(-pointSize.x, pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, pointSize.y, -pointSize.z, 0.0)); //tl
    EmitVertex();


    //bottom
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, pointSize.z, 0.0)); //br
    EmitVertex();


    gl_Position = (pos + vec4(-pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //bl
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, pointSize.z, 0.0)); //tr
    EmitVertex();
    gl_Position = (pos + vec4(pointSize.x, -pointSize.y, -pointSize.z, 0.0)); //tl
    EmitVertex();

}