#include "Animation.h"

Animation::Animation()
{

}
Animation::~Animation()
{

}
void Animation::PushBack(const SDL_Rect& rect)
{
	frames[last_frame++] = rect;
}
SDL_Rect& Animation::GetCurrentFrame(float dt)
{
	current_frame += speed * dt;
	if (current_frame >= last_frame)
	{
		if (loop) {
			current_frame = 0.0f;
		}
		else {
			current_frame = last_frame - 1;
		}
		repetitions++;
	}

	return frames[(int)current_frame];
}
int Animation::GetCurrentFrame()
{
	return (int)current_frame;
}
bool Animation::Finished() const
{
	return repetitions > 0;
}
void Animation::Restart()
{
	current_frame = 0;
	repetitions = 0;
}
bool Animation::isInFrame(int x)
{
	return(current_frame >= x && current_frame < x + 1);
}