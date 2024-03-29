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

#include "rengine.h"
#include "examples.h"

using namespace rengine;
using namespace std;

class MyWindow : public StandardSurfaceInterface
{
public:

    Node *createSubtree(float dx, float dy) {
        return
            &(*TransformNode::create(mat4::translate2D(dx, dy))
              << &(*ColorFilterNode::create()
                   << RectangleNode::create(rect2d::fromXywh(  0, 0, 90, 90), vec4(0.8, 0.1, 0.1, 1))
                   << RectangleNode::create(rect2d::fromXywh(100, 0, 90, 90), vec4(0.8, 0.8, 0.1, 1))
                   << RectangleNode::create(rect2d::fromXywh(200, 0, 90, 90), vec4(0.1, 0.8, 0.1, 1))
                   << RectangleNode::create(rect2d::fromXywh(300, 0, 90, 90), vec4(0.1, 0.8, 0.8, 1))
                   << RectangleNode::create(rect2d::fromXywh(400, 0, 90, 90), vec4(0.1, 0.1, 0.8, 1))
                   << RectangleNode::create(rect2d::fromXywh(500, 0, 90, 90), vec4(0.8, 0.1, 0.8, 1))
                   << TextureNode::create(rect2d::fromXywh(600, 0, 90, 90), m_layer)
                  )
             );
    }

    Node *update(Node *old) {
        if (old)
            return old;

        m_layer = rengine_loadImage(renderer(), "walker.png");

        Node *root = Node::create();

        float pos = 10;

        Node *unfiltered = createSubtree(10, pos);
        root->append(unfiltered);

        { // desaturate
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            node->setColorMatrix(ColorMatrix::saturation(0.2));
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setDirection(Animation::Alternate);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1;
            anim->keyFrames.addValues<double, ColorFilterNode_saturation>() << 0 << 4;
            animationManager()->startAnimation(anim);
        }

        { // grayscale
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            node->setColorMatrix(ColorMatrix::grayscale());
            root->append(tree);
        }

        { // sepia
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setDirection(Animation::Alternate);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1;
            anim->keyFrames.addValues<double, ColorFilterNode_sepia>() << 0 << 1;
            animationManager()->startAnimation(anim);
        }

        { // brightness
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            node->setColorMatrix(ColorMatrix::brightness(0.3));
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setDirection(Animation::Alternate);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1.0;
            anim->keyFrames.addValues<double, ColorFilterNode_brightness>() << 0 << 4;
            animationManager()->startAnimation(anim);
        }

        { // hue shift
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            node->setColorMatrix(ColorMatrix::hue(1.0));
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1.0;
            anim->keyFrames.addValues<double, ColorFilterNode_hue>() << 0 << 3.14152 * 2;
            animationManager()->startAnimation(anim);
        }

        { // contrast
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setDirection(Animation::Alternate);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1.0;
            anim->keyFrames.addValues<double, ColorFilterNode_contrast>() << 0 << 2;
            animationManager()->startAnimation(anim);
        }

        { // invert
            pos += 100;
            Node *tree = createSubtree(10, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->child());
            root->append(tree);

            AnimationClosure<ColorFilterNode> *anim = new AnimationClosure<ColorFilterNode>(node);
            anim->setDuration(3);
            anim->setDirection(Animation::Alternate);
            anim->setIterations(-1);
            anim->keyFrames.times() << 0 << 1.0;
            anim->keyFrames.addValues<double, ColorFilterNode_invert>() << 0 << 1;
            animationManager()->startAnimation(anim);
        }

        return root;
    }

    Texture *m_layer;
};

RENGINE_MAIN(MyWindow)