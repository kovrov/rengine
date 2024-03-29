/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <stack>

RENGINE_BEGIN_NAMESPACE

class OpenGLRenderer : public Renderer
{
public:

    struct TexturePool : public std::vector<GLuint>
    {
        ~TexturePool()
        {
            glDeleteTextures(size(), data());
        }

        GLuint acquire() {
            GLuint id;
            if (empty()) {
                glGenTextures(1, &id);
                return id;
            }
            id = back();
            pop_back();
            return id;
        }

        void release(GLuint id) {
            push_back(id);
        }

        void compact() {
            glFlush();
            for (auto id : *this) {
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
        }
    };

    struct Element {
        Node *node;
        unsigned vboOffset;         // offset into vbo for flattened, rect and layer nodes
        float z;                    // only valid when 'projection' is set
        unsigned texture;           // only valid during rendering when 'layered' is set.
        unsigned sourceTexture;     // only valid during rendering when 'layered' is set and we have a shadow node
        unsigned groupSize : 29;    // The size of this group, used with 'projection' and 'layered'. Packed to ft into 32-bit
                                    // The groupSize is the number of nodes inside the group, excluding the parent.
        unsigned projection : 1;    // 3d subtree
        unsigned layered : 1;       // subtree is flattened into a layer (texture)
        unsigned completed : 1;     // used during the actual rendering to know we're done with it

        bool operator<(const Element &e) const { return e.completed || z < e.z; }
    };
    struct Program : OpenGLShaderProgram {
        int matrix;
    };
    enum ProgramUpdate {
        UpdateSolidProgram          = 0x01,
        UpdateTextureProgram        = 0x02,
        UpdateAlphaTextureProgram   = 0x04,
        UpdateColorFilterProgram    = 0x08,
        UpdateBlurProgram           = 0x10,
        UpdateShadowProgram         = 0x20,
        UpdateAllPrograms           = 0xffffffff
    };

    OpenGLRenderer();
    ~OpenGLRenderer();

    Texture *createTextureFromImageData(const vec2 &size, Texture::Format format, void *data);
    void initialize();
    bool render() override;
    void frameSwapped() override { m_texturePool.compact(); }
    bool readPixels(int x, int y, int w, int h, unsigned *pixels);

    void prepass(Node *n);
    void build(Node *n);
    void drawColorQuad(unsigned bufferOffset, const vec4 &color);
    void drawTextureQuad(unsigned bufferOffset, GLuint texId, float opacity = 1.0);
    void drawColorFilterQuad(unsigned bufferOffset, GLuint texId, const mat4 &cm);
    void drawBlurQuad(unsigned bufferOffset, GLuint texId, int radius, const vec2 &renderSize, const vec2 &textureSize, const vec2 &step);
    void drawShadowQuad(unsigned bufferOffset, GLuint texId, int radius, const vec2 &renderSize, const vec2 &textureSize, const vec2 &step, const vec4 &color);
    void activateShader(const Program *shader);
    void projectQuad(const vec2 &a, const vec2 &b, vec2 *v);
    void render(Element *first, Element *last);
    void renderToLayer(Element *e);
    rect2d boundingRectFor(unsigned vertexOffset) const { return rect2d(m_vertices[vertexOffset], m_vertices[vertexOffset + 3]); }

    void ensureMatrixUpdated(ProgramUpdate bit, Program *p);

    struct TextureProgram : public Program {
        int matrix;
    } prog_layer;
    struct AlphaTextureProgram : public Program {
        int alpha;
    } prog_alphaTexture;
    struct SolidProgram : public Program {
        int color;
    } prog_solid;
    struct ColorFilterProgram : public Program {
        int colorMatrix;
    } prog_colorFilter;
    struct BlurProgram : public Program {
        int dims;
        int radius;
        int sigma;
        int dir;
    } prog_blur;
    struct ShadowProgram : public BlurProgram {
        int color;
    } prog_shadow;

    unsigned m_numLayeredNodes;
    unsigned m_numTextureNodes;
    unsigned m_numRectangleNodes;
    unsigned m_numTransformNodes;
    unsigned m_numTransformNodesWith3d;
    unsigned m_additionalQuads;

    unsigned m_vertexIndex;
    unsigned m_elementIndex;
    vec2 *m_vertices;
    Element *m_elements;
    mat4 m_proj;
    mat4 m_m2d;    // for the 2d world
    mat4 m_m3d;    // below a 3d projection subtree
    float m_farPlane;
    rect2d m_layerBoundingBox;
    vec2 m_surfaceSize;

    TexturePool m_texturePool;

    const Program *m_activeShader;
    GLuint m_texCoordBuffer;
    GLuint m_vertexBuffer;
    GLuint m_fbo;

    unsigned m_matrixState;

    bool m_render3d : 1;
    bool m_layered : 1;

};

inline void OpenGLRenderer::projectQuad(const vec2 &a, const vec2 &b, vec2 *v)
{
    // The steps involved in each line is as follows.:
    // pt_3d = matrix3D * pt                 // apply the 3D transform
    // pt_proj = pt_3d.project2D()           // project it to 2D based on current farPlane
    // pt_screen = parent_matrix * pt_proj   // Put the output of our local 3D into the scene world coordinate system
    v[0] = m_m2d * ((m_m3d * vec3(a))       .project2D(m_farPlane));    // top left
    v[1] = m_m2d * ((m_m3d * vec3(a.x, b.y)).project2D(m_farPlane));    // bottom left
    v[2] = m_m2d * ((m_m3d * vec3(b.x, a.y)).project2D(m_farPlane));    // top right
    v[3] = m_m2d * ((m_m3d * vec3(b))       .project2D(m_farPlane));    // bottom right
}

inline void OpenGLRenderer::ensureMatrixUpdated(ProgramUpdate bit, Program *p)
{
    if (m_matrixState & bit) {
        m_matrixState &= ~bit;
        glUniformMatrix4fv(p->matrix, 1, true, m_proj.m);
    }
}



RENGINE_END_NAMESPACE
