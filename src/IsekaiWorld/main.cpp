#include "stdafx.h"
#include "tengine.h"
#include "cube.h"
#include "DebugNew.h"
#if defined(_MSC_VER)
#	pragma comment( lib, "MicroEngine.lib" )
#endif

void resetfpshistory();
void limitfps(int& millis, int curmillis);
void updatefpshistory(int millis);
void inputgrab(bool on, bool delay = false);
void ignoremousemotion();
void checkinput();
void quit(); // normal exit
#define SCR_MINW 320
#define SCR_MINH 200
#define SCR_MAXW 10000
#define SCR_MAXH 10000
int focused = 0;

void WindowFocusFunc(bool focused)
{
	extern bool shouldgrab;
	if (focused)
	{
		shouldgrab = true;
	}
	else
	{
		shouldgrab = false;
		focused = -1;
	}
}

void MouseEnterFunc(bool enter)
{
	extern bool shouldgrab;

	if (enter)
	{
		shouldgrab = false;
		focused = 1;
	}
	else
	{
		shouldgrab = false;
		focused = -1;
	}
}

void WindowMinimizeFunc(bool minimize)
{
	minimized = minimize == true;
}

void MouseBtnFunc(int button, int action) noexcept
{
	if (button == 0) processkey(-1, action == 1/*GLFW_PRESS*/);
	if (button == 1) processkey(-3, action == 1/*GLFW_PRESS*/);
	if (button == 2) processkey(-2, action == 1/*GLFW_PRESS*/);
	/*case SDL_BUTTON_X1: processkey(-6, event.button.state == SDL_PRESSED); break;
	case SDL_BUTTON_X2: processkey(-7, event.button.state == SDL_PRESSED); break;
	case SDL_BUTTON_X2 + 1: processkey(-10, event.button.state == SDL_PRESSED); break;
	case SDL_BUTTON_X2 + 2: processkey(-11, event.button.state == SDL_PRESSED); break;
	case SDL_BUTTON_X2 + 3: processkey(-12, event.button.state == SDL_PRESSED); break;*/
}

void KeyFunc(int key, int mod, bool isPressed)
{
	extern int keyrepeatmask;
	//if (keyrepeatmask || !IsKeyboardKeyPressed(key)/*event.key.repeat */ )
		processkey(key, isPressed, mod/*event.key.keysym.mod | SDL_GetModState()*/);
}

void WindowResizeFunc(int width, int height) noexcept
{
	extern int scr_w;
	extern int scr_h;
	//extern int fullscreendesktop;
	//if (!fullscreendesktop)
	{
		scr_w = clamp(GetRenderWidth(), SCR_MINW, SCR_MAXW);
		scr_h = clamp(GetRenderHeight(), SCR_MINH, SCR_MAXH);
	}
	gl_resize();
}

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
	atexit((void(__cdecl*)(void))_CrtDumpMemoryLeaks);
#endif
		
	engine::EngineCreateInfo createInfo;
	createInfo.Window.Width = 1366;
	createInfo.Window.Height = 768;
	createInfo.Window.WindowFocusEvent = WindowFocusFunc;
	createInfo.Window.MouseCursorEnterEvent = MouseEnterFunc;
	createInfo.Window.WindowMinimizeEvent = WindowMinimizeFunc;
	createInfo.Window.WindowResizeEvent = WindowResizeFunc;
	createInfo.Window.MouseButtonEvent = MouseBtnFunc;
	createInfo.Window.KeyEvent = KeyFunc;

	if (engine::CreateEngine(createInfo))
	{
		SetMouseLock(true);

		initing = INIT_RESET;

		execfile("init.cfg", false);

		initing = NOT_INITING;

		numcpus = clamp(std::thread::hardware_concurrency(), 1, 16);

		SDL_SetMainReady();
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
			fatal("Unable to initialize SDL: %s", SDL_GetError());

		game::initclient();

		OpenGLCheckExtensions();
		OpenGLInit();
		notexture = textureload("packages/textures/notexture.png");
		if (!notexture) fatal("could not find core textures");

		if (!execfile("data/stdlib.cfg", false)) fatal("cannot find data files (you are running from the wrong folder, try .bat file in the main folder)");   // this is the first file we load.
		if (!execfile("data/font.cfg", false)) fatal("cannot find font definitions");
		if (!setfont("default")) fatal("no default font specified");

		inbetweenframes = true;
		renderbackground("initializing...");

		camera1 = player = game::iterdynents(0);
		emptymap(0, true, NULL, false);

		initsound();

		initing = INIT_LOAD;
		execfile("data/keymap.cfg");
		execfile("data/stdedit.cfg");
		execfile("data/sounds.cfg");
		execfile("data/menus.cfg");
		execfile("data/heightmap.cfg");
		execfile("data/blendbrush.cfg");
		defformatstring(gamecfgname, "data/game_%s.cfg", game::gameident());
		execfile(gamecfgname);

		identflags |= IDF_PERSIST;

		if (!execfile(game::savedconfig(), false))
		{
			execfile(game::defaultconfig());
			writecfg(game::restoreconfig());
		}
		execfile(game::autoexec(), false);

		identflags &= ~IDF_PERSIST;

		initing = INIT_GAME;
		//..
		initing = NOT_INITING;
		
		loadshaders();
		initparticles();
		initdecals();

		identflags |= IDF_PERSIST;

		initmumble();
		resetfpshistory();

		inputgrab(grabinput = true);
		ignoremousemotion();

		while (engine::IsRunningEngine())
		{
			const float deltaTime = engine::GetDeltaTime();
			engine::BeginFrameEngine();

			{
				static int frames = 0;
				int millis = getclockmillis();
				limitfps(millis, totalmillis);
				elapsedtime = millis - totalmillis;
				static int timeerr = 0;
				int scaledtime = game::scaletime(elapsedtime) + timeerr;
				curtime = scaledtime / 100;
				timeerr = scaledtime % 100;
				if (!multiplayer(false) && curtime > 200) curtime = 200;
				if (game::ispaused()) curtime = 0;
				lastmillis += curtime;
				totalmillis = millis;
				updatetime();

				checkinput();
				{
					// event
					
					// mouse move
					{
						bool mousemoved = false;
						extern bool shouldgrab;
						if (grabinput)
						{
							auto mousedelta = GetMouseDelta();
							int dx = mousedelta.x, dy = mousedelta.y;
							//checkmousemotion(dx, dy);
							if (!g3d_movecursor(dx, dy))
								mousemove(dx, dy);
							mousemoved = true;
						}
						else if (shouldgrab) inputgrab(grabinput = true, false);
						//if (mousemoved) resetmousemotion();
					}
				}



				menuprocess();
				tryedit();

				if (lastmillis) game::updateworld();

				checksleep(lastmillis);

				if (frames) updatefpshistory(elapsedtime);
				frames++;

				// miscellaneous general game effects
				recomputecamera();
				updateparticles();
				updatesounds();

				if (minimized) continue;

				inbetweenframes = false;
				if (mainmenu) gl_drawmainmenu();
				else gl_drawframe();

				swapbuffers();
				renderedframe = inbetweenframes = true;
			}

			engine::EndFrameEngine();
		}
		quit();
	}

	engine::DestroyEngine();
}