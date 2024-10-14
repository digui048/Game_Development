#pragma once
#include "SDL2/SDL_rect.h"
#include "Textures.h"
#include <vector>

enum class AnimMode { AUTOMATIC, MANUAL };

struct Animation
{
    int delay;
    std::vector<SDL_Rect> frames;
};

class Sprite : public Render
{
public:
    Sprite(const Textures* texture);
    ~Sprite();

    void SetNumberAnimations(int num);
    void SetAnimationDelay(int id, int delay);
    void AddKeyFrame(int id, const SDL_Rect& rect);
    void SetAnimation(int id);
    int GetAnimation();

    void SetManualMode();
    void SetAutomaticMode();

    void Update();
    void NextFrame();
    void PrevFrame();

    /*void Draw(int x, int y);
    void DrawTint(int x, int y, const SDL_Color& col);*/

    void Release();

private:
    int current_anim;
    int current_frame;
    int current_delay;

    const Textures* img;
    std::vector<Animation> animations;

    AnimMode mode;
};
