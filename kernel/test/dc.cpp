/*
 *  Copyright (C) 2016 Matthew Keeter  <matt.j.keeter@gmail.com>
 *
 *  This file is part of the Ao library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  Ao is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Ao.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <catch/catch.hpp>

#include "ao/kernel/format/mesh.hpp"
#include "ao/kernel/format/contours.hpp"

#include "ao/kernel/render/region.hpp"
#include "ao/kernel/eval/evaluator.hpp"
#include "ao/kernel/tree/tree.hpp"

// Overloaded toString for glm::vec3
#include "glm.hpp"

TEST_CASE("Small sphere mesh")
{
    std::unique_ptr<Token> t(
        Token::operation(Opcode::OP_SUB,
            Token::operation(Opcode::OP_ADD,
            Token::operation(Opcode::OP_ADD,
                    Token::operation(Opcode::OP_MUL, Token::X(), Token::X()),
                    Token::operation(Opcode::OP_MUL, Token::Y(), Token::Y())),
                    Token::operation(Opcode::OP_MUL, Token::Z(), Token::Z())),
            Token::constant(0.5)));

    Region r({-1, 1}, {-1, 1}, {-1, 1}, 1);

    auto m = Mesh::Render(t.get(), r);

    REQUIRE(m.tris.size() == 12);
}

TEST_CASE("Face normals")
{
    Token* axis[3] = {Token::X(), Token::Y(), Token::Z()};
    glm::vec3 norm[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    Region r({-1, 1}, {-1, 1}, {-1, 1}, 2);

    SECTION("Positive")
    {
        for (int i=0; i < 3; ++i)
        {
            auto m = Mesh::Render(axis[i], r);
            for (unsigned j=0; j < m.tris.size(); ++j)
            {
                REQUIRE(m.norm(j) == norm[i]);
            }
        }
    }

    SECTION("Negative")
    {
        for (int i=0; i < 3; ++i)
        {
            std::unique_ptr<Token> t(Token::operation(
                        Opcode::OP_NEG,
                        Token::operation(Opcode::OP_ADD, axis[i], 
                            Token::constant(0.75))));
            auto m = Mesh::Render(t.get(), r);
            for (unsigned j=0; j < m.tris.size(); ++j)
            {
                REQUIRE(m.norm(j) == -norm[i]);
            }
        }
    }

    // Clean up after yourself!
    for (auto a : axis)
    {
        delete a;
    }
}

TEST_CASE("Simple 2D contouring")
{
    std::unique_ptr<Token> t(
        Token::operation(Opcode::OP_SUB,
            Token::operation(Opcode::OP_ADD,
                    Token::operation(Opcode::OP_MUL, Token::X(), Token::X()),
                    Token::operation(Opcode::OP_MUL, Token::Y(), Token::Y())),
            Token::constant(0.5)));

    Region r({-1, 1}, {-1, 1}, {0, 0}, 1);

    auto m = Contours::Render(t.get(), r);
    REQUIRE(m.contours.size() == 1);
}

TEST_CASE("2D contour tracking")
{
    std::unique_ptr<Token> t(
        Token::operation(Opcode::OP_SUB,
            Token::operation(Opcode::OP_ADD,
                    Token::operation(Opcode::OP_MUL, Token::X(), Token::X()),
                    Token::operation(Opcode::OP_MUL, Token::Y(), Token::Y())),
            Token::constant(0.5)));

    Region r({-1, 1}, {-1, 1}, {0, 0}, 10);

    auto m = Contours::Render(t.get(), r);
    REQUIRE(m.contours.size() == 1);

    float min = 1;
    float max = 0;
    for (auto c : m.contours[0])
    {
        auto r = pow(c.x, 2) + pow(c.y, 2);
        min = fmin(min, r);
        max = fmax(max, r);
    }
    REQUIRE(max < 0.51);
    REQUIRE(min > 0.49);
}
