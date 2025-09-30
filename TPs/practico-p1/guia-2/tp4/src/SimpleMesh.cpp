#include "SimpleMesh.h"

SimpleMesh::SimpleMesh(const std::vector<float> &positions,
                       bool useIndices,
                       const std::vector<uint32_t> &indices)
    : indexed_(useIndices)
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 positions.size() * sizeof(float),
                 positions.data(),
                 GL_STATIC_DRAW);

    if (indexed_ && !indices.empty())
    {
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(uint32_t),
                     indices.data(),
                     GL_STATIC_DRAW);
        indexCount_ = static_cast<GLsizei>(indices.size());
    }
    else
    {
        vertexCount_ = static_cast<GLsizei>(positions.size() / 2);
    }

    // layout(location=0) -> vec2 posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), (void *)0);

    glBindVertexArray(0);
}

SimpleMesh::~SimpleMesh() { destroy(); }

void SimpleMesh::destroy()
{
    if (ebo_)
        glDeleteBuffers(1, &ebo_), ebo_ = 0;
    if (vbo_)
        glDeleteBuffers(1, &vbo_), vbo_ = 0;
    if (vao_)
        glDeleteVertexArrays(1, &vao_), vao_ = 0;
}

SimpleMesh::SimpleMesh(SimpleMesh &&o) noexcept
{
    vao_ = o.vao_;
    vbo_ = o.vbo_;
    ebo_ = o.ebo_;
    vertexCount_ = o.vertexCount_;
    indexCount_ = o.indexCount_;
    indexed_ = o.indexed_;
    o.vao_ = o.vbo_ = o.ebo_ = 0;
    o.vertexCount_ = o.indexCount_ = 0;
    o.indexed_ = false;
}

SimpleMesh &SimpleMesh::operator=(SimpleMesh &&o) noexcept
{
    if (this != &o)
    {
        destroy();
        vao_ = o.vao_;
        vbo_ = o.vbo_;
        ebo_ = o.ebo_;
        vertexCount_ = o.vertexCount_;
        indexCount_ = o.indexCount_;
        indexed_ = o.indexed_;
        o.vao_ = o.vbo_ = o.ebo_ = 0;
        o.vertexCount_ = o.indexCount_ = 0;
        o.indexed_ = false;
    }
    return *this;
}

void SimpleMesh::bind() const { glBindVertexArray(vao_); }

void SimpleMesh::draw() const
{
    if (indexed_)
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, (void *)0);
    else
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}
