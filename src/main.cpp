#include "SDL_render.h"
#include "SDL_timer.h"
#include <SDL.h>
#include <cmath>


#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480
#define VELOCITY_SIGMA		0.001


int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {

		status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
									 x + offsety, y + offsetx);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
									 x + offsetx, y + offsety);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
									 x + offsetx, y - offsety);
		status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
									 x + offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

class Vector {
public:
	double X;
	double Y;
public:
	Vector() {}
	Vector(double X, double Y) :
		X(X),
		Y(Y)
	{}
	~Vector() {}

	Vector operator+(const Vector& Rhs) const
	{
		return Vector(X + Rhs.X, Y + Rhs.Y);
	}
	Vector operator*(const double& Rhs) const
	{
		return Vector(X * Rhs, Y * Rhs);
	}
};

class Ball {
public:
	Vector Position;
	Vector Velocity;
	float Radius;
	float Friction;
public:
	Ball() {}

	void Update(float DeltaTime)
	{
		ApplyGravity(DeltaTime);
		Clipping();
		printf("\rPos:(%lf, %lf) Vel:(%lf, %lf)", Position.X, Position.Y, Velocity.X, Velocity.Y);
	}

	void Draw(SDL_Renderer* Render)
	{
		SDL_SetRenderDrawColor(Render, 200, 255, 200, 0);
		SDL_RenderFillCircle(Render, Position.X, Position.Y, Radius);
	}

private:
	void ApplyGravity(float DeltaTime)
	{
		static const Vector Gravity(.0, 9.8 * 0.0001);
		Vector OldVelocity = Velocity;
		OldVelocity = OldVelocity + Gravity * DeltaTime;
		Velocity = OldVelocity;
		Position = Position + Velocity;
	}

	void Clipping()
	{
		if (Position.X < Radius) {
			Position.X = Radius;
			Velocity.X *= -Friction;
		}
		if (Position.X > double(WINDOW_WIDTH) - Radius) {
			Position.X = double(WINDOW_WIDTH) - Radius;
			Velocity.X *= -Friction;
		}
		if (Position.Y < Radius) {
			Position.Y = Radius;
			Velocity.Y *= -Friction;
		}
		if (Position.Y > double(WINDOW_HEIGHT) - Radius) {
			Position.Y = double(WINDOW_HEIGHT) - Radius;
			Velocity.Y *= -Friction;
		}
		if (abs(Velocity.X) < VELOCITY_SIGMA) {
			Velocity.X = 0.;
		}
		if (abs(Velocity.Y) < VELOCITY_SIGMA) {
			Velocity.Y = 0.;
		}
	}
};

class Application {
public:
	Application() :
		RequestedFinish(false),
		PerSec(SDL_GetPerformanceFrequency()),
		LastTime(SDL_GetPerformanceCounter()),
		DeltaTime(0.f),
		Window(nullptr),
		Render(nullptr)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
	}
	~Application() 
	{
		SDL_Quit();
	}

	void ShowWindow()
	{
		Window = SDL_CreateWindow("PhyWater", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
		Render = SDL_CreateRenderer(Window, -1, 0);
	}

	void Execute()
	{
		Uint64 Now;
		while (!RequestedFinish) {
			PollEvent();
			Now = SDL_GetPerformanceCounter();

			DeltaTime = (Now - LastTime) / PerSec * 1000.f;

			Update(DeltaTime);

			Draw();

			LastTime = Now;
		}
		if (Window) {
			SDL_DestroyRenderer(Render);
			SDL_DestroyWindow(Window);
		}
	}

	void Update(float DeltaTime)
	{
		OneBall.Update(DeltaTime);
	}

	void PollEvent()
	{
		SDL_Event Event;
		if (SDL_PollEvent(&Event)) {
			if (Event.type == SDL_QUIT) {
				RequestedFinish = true;
			}
		}
	}

	void Draw()
	{
		SDL_SetRenderDrawColor(Render, 100, 100, 100, 0);
		SDL_RenderClear(Render);

		OneBall.Draw(Render);

		SDL_RenderPresent(Render);
	}

public:
	Ball OneBall;

private:
	bool RequestedFinish;
	const float PerSec;
	Uint64 LastTime;
	float DeltaTime;
	SDL_Window* Window;
	SDL_Renderer* Render;
};

int main(int argc, char** argv)
{
	Application App;
	App.ShowWindow();

	App.OneBall.Position = Vector(WINDOW_WIDTH / 2.f, 0.f);
	App.OneBall.Velocity = Vector(0.0, 0.0);
	App.OneBall.Radius = 10.f;
	App.OneBall.Friction = 0.9f;

	App.Execute();
}
