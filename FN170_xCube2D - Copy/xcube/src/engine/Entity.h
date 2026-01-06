#pragma once
#include <SDL.h>
#include "GameMath.h"

class GraphicsEngine;

//base class for all world entities
class Entity
{
protected:
    //transform and movement state
    Point2 position;
    Point2 velocity;

    //rendering resources
    SDL_Texture* texture = nullptr;
    SDL_Rect src{ 0, 0, 0, 0 };
    SDL_Rect dest{ 0, 0, 0, 0 };

    //rotation and lifecycle state
    float angle = 0.0f;
    bool alive = true;

public:
    Entity() = default;
    virtual ~Entity() = default;

    //per frame update and rendering interface
    virtual void update(float dt) = 0;
    virtual void render(GraphicsEngine* gfx) = 0;

    //position accessors
    Point2 getPosition() const { return position; }
    void setPosition(const Point2& p)
    {
        position = p;
        dest.x = (int)p.x;
        dest.y = (int)p.y;
    }

    //render bounds and lifecycle query
    SDL_Rect getRect() const { return dest; }
    bool isAlive() const { return alive; }

    //assign entity texture
    void setTexture(SDL_Texture* t) { texture = t; }

    //set render size and initialise source rect if unset
    void setSize(int w, int h)
    {
        dest.w = w;
        dest.h = h;

        //initialise source rect to match size if not already set
        if (src.w == 0 && src.h == 0)
        {
            src.w = w;
            src.h = h;
        }
    }

    //explicitly define source rectangle
    void setSrcRect(int x, int y, int w, int h)
    {
        src = { x, y, w, h };
    }
};
