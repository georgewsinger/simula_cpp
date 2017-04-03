/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2014 Forrest Reiling
**
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/
#ifndef WIREFRAMENODE_H
#define WIREFRAMENODE_H

#include <scenegraph/output/drawable.h>
#include <scenegraph/output/viewpoint.h>
#include <gl/openglshader.h>

namespace motorcar {
class WireframeNode : public Drawable
{
public:
   // WireframeNode(std::vector<std::pair<glm::vec3, glm::vec3> > &segments, glm::vec3 lineColor, SceneGraphNode *parent, const glm::mat4 &transform = glm::mat4());

    WireframeNode(float *segments, int numSegments, glm::vec3 lineColor, SceneGraphNode *parent, const glm::mat4 &transform = glm::mat4());

    ~WireframeNode();

    virtual void draw(Scene *scene, Display *display) override;

    glm::vec3 lineColor() const;
    void setLineColor(const glm::vec3 &lineColor);

    int numSegments() const;
    float *segments() const;

private:
    float *m_segments;
    int m_numSegments;
    glm::vec3 m_lineColor;

    OpenGLShader *m_lineShader;

    GLuint m_lineVertexCoordinates;

    GLint h_aPosition_line, h_uMVPMatrix_line, h_uColor_line;
};
}


#endif // WIREFRAMENODE_H
