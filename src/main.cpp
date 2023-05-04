#include <SDL.h>


int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* Window = SDL_CreateWindow("PhyWater", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	SDL_Renderer* Render = SDL_CreateRenderer(Window, -1, 0);

	SDL_Event Event;
	for (;;) {
		while(SDL_PollEvent(&Event)) {
			if (Event.type == SDL_QUIT) {
				return 0;
			}
		}
		SDL_SetRenderDrawColor(Render, 127, 127, 127, 0);
		SDL_RenderClear(Render);
		SDL_RenderPresent(Render);
	}
	SDL_Quit();
}
