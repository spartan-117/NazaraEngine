#include <Nazara/Utility/Linux/XDisplay.hpp>
#include <X11/Xlib-xcb.h>
namespace {
    Display* sharedDisplay = nullptr;
    xcb_connection_t* sharedConnection = nullptr;
    unsigned int displayRefCount = 0;
    unsigned int xcbRefCount = 0;

    int screenNum = -1;
}


Display* nzOpenDisplay(int* screenNb) {
    if (!sharedDisplay)
    {
        sharedDisplay = XOpenDisplay(NULL);
    }
    ++displayRefCount;    
    return sharedDisplay;
}

void nzCloseDisplay(Display* display) {
    //assert(display == sharedDisplay);
    --displayRefCount;
    XCloseDisplay(display);
}

xcb_connection_t* nzOpenXCBConnection(int* screenNb) {
    if (!sharedConnection)
        sharedConnection = XGetXCBConnection(nzOpenDisplay(screenNb));
    return sharedConnection;
}

void nzCloseXCBConnection(xcb_connection_t* c) {
    //assert(c == sharedConnection);
    nzCloseDisplay(sharedDisplay);
    xcb_disconnect(c);
}
