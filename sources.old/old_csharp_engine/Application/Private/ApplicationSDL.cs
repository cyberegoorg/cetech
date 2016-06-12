using System;
using CETech.CEMath;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech
{
    public static partial class Application
    {
        private static SDL.SDL_Event _sdlEvent;

        private static void PlaformUpdateEvents()
        {
            while (SDL.SDL_PollEvent(out _sdlEvent) > 0)
            {
                switch (_sdlEvent.type)
                {
                    case SDL.SDL_EventType.SDL_QUIT:
                        Quit();
                        break;

                    // TODO: No coment =D
                    case SDL.SDL_EventType.SDL_FIRSTEVENT:
                        break;
                    case SDL.SDL_EventType.SDL_WINDOWEVENT:
                        break;
                    case SDL.SDL_EventType.SDL_SYSWMEVENT:
                        break;
                    case SDL.SDL_EventType.SDL_KEYDOWN:
                        break;
                    case SDL.SDL_EventType.SDL_KEYUP:
                        break;
                    case SDL.SDL_EventType.SDL_TEXTEDITING:
                        break;
                    case SDL.SDL_EventType.SDL_TEXTINPUT:
                        break;
                    case SDL.SDL_EventType.SDL_MOUSEMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_MOUSEBUTTONDOWN:
                        break;
                    case SDL.SDL_EventType.SDL_MOUSEBUTTONUP:
                        break;
                    case SDL.SDL_EventType.SDL_MOUSEWHEEL:
                        break;
                    case SDL.SDL_EventType.SDL_JOYAXISMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_JOYBALLMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_JOYHATMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_JOYBUTTONDOWN:
                        break;
                    case SDL.SDL_EventType.SDL_JOYBUTTONUP:
                        break;
                    case SDL.SDL_EventType.SDL_JOYDEVICEADDED:
                        break;
                    case SDL.SDL_EventType.SDL_JOYDEVICEREMOVED:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERAXISMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERBUTTONDOWN:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERBUTTONUP:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERDEVICEADDED:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERDEVICEREMOVED:
                        break;
                    case SDL.SDL_EventType.SDL_CONTROLLERDEVICEREMAPPED:
                        break;
                    case SDL.SDL_EventType.SDL_FINGERDOWN:
                        break;
                    case SDL.SDL_EventType.SDL_FINGERUP:
                        break;
                    case SDL.SDL_EventType.SDL_FINGERMOTION:
                        break;
                    case SDL.SDL_EventType.SDL_DOLLARGESTURE:
                        break;
                    case SDL.SDL_EventType.SDL_DOLLARRECORD:
                        break;
                    case SDL.SDL_EventType.SDL_MULTIGESTURE:
                        break;
                    case SDL.SDL_EventType.SDL_CLIPBOARDUPDATE:
                        break;
                    case SDL.SDL_EventType.SDL_DROPFILE:
                        break;
                    case SDL.SDL_EventType.SDL_AUDIODEVICEADDED:
                        break;
                    case SDL.SDL_EventType.SDL_AUDIODEVICEREMOVED:
                        break;
                    case SDL.SDL_EventType.SDL_RENDER_TARGETS_RESET:
                        break;
                    case SDL.SDL_EventType.SDL_RENDER_DEVICE_RESET:
                        break;
                    case SDL.SDL_EventType.SDL_USEREVENT:
                        break;
                    case SDL.SDL_EventType.SDL_LASTEVENT:
                        break;
                    default:
                        throw new ArgumentOutOfRangeException();
                }
            }
        }

        private static void SetCursorPositionImpl(Vec2f pos)
        {
            _mainWindow.SetCursorPosition(pos);
        }
    }
}