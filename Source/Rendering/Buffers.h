#ifndef BUFFERS_H
#define BUFFERS_H

/*
* All subsequent rendering operations will now render to the attachments of the currently bound framebuffer.
* Since it is not the default framebuffer, the rendering commands will have no impact on the visual output of your window. 
* For this reason it is called "off-screen rendering" when rendering to a different framebuffer.
* If you want all rendering operations to have a visual impact again on the main window we need to make the default framebuffer 
* active by binding to 0
* 
* Before the completeness check is executed we need to attach one or more attachments to the framebuffer.
* An attachment is a memory location that can act as a buffer for the framebuffer, think of it as an image.
* When creating an attachment we have two options to take: "textures" or "renderbuffer" objects.
* 
* When attaching a texture to a framebuffer, all rendering commands will write to the texture as if it was a normal color/depth 
* or stencil buffer. The advantage of using textures is that the render output is stored inside the texture image that we can 
* then easily use in our shaders.
* 
* Renderbuffer objects were introduced to OpenGL after textures as a possible type of framebuffer attachment, Just like a texture image, 
* a renderbuffer object is an actual buffer e.g. an array of bytes, integers, pixels or whatever. However, a renderbuffer object can not 
* be directly read from. This gives it the added advantage that OpenGL can do a few memory optimizations that can give it a performance 
* edge over textures for off-screen rendering to a framebuffer.
*/

unsigned int CreateFrameBuffer(bool enable = false);

unsigned int CreateTextureAttachment(int width, int height);
unsigned int CreateDepthTextureAttachment(int width, int height);
unsigned int CreateRenderBufferAttachment(int width, int height);
unsigned int CreateDepthTextureArray(int width, int height, int layers);

void BindFramebuffer(unsigned int fbo);
void UnbindFramebuffer();
void BindDefaultFramebuffer();

void ValidateBuffer(unsigned int fbo);

#endif 