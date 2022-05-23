
//Misc functions

#pragma once

#include<string>

extern "C"{
#include<X11/Xlib.h>
#include<sys/time.h>
}

//Gets the string of an X11 event
//https://stackoverflow.com/a/60763874/6342516
const std::string& getXEventString(const XEvent& e){
    const static std::string XEventTypeStrings[37] = {
        "", "", "KeyPress", "KeyRelease", "ButtonPress"
        "ButtonRelease", "MotionNotify", "EnterNotify",
        "LeaveNotify", "FocusIn", "FocusOut", "KeymapNotify",
        "Expose", "GraphicsExpose", "NoExpose", "VisibilityNotify",
        "CreateNotify", "DestroyNotify", "UnmapNotify",
        "MapNotify", "MapRequest", "ReparentNotify",
        "ConfigureNotify", "ConfigureRequest", "GravityNotify",
        "ResizeRequest", "CirculateNotify", "CirculateRequest",
        "PropertyNotify", "SelectionClear", "SelectionRequest",
        "SelectionNotify", "ColormapNotify", "ClientMessage",
        "MappingNotify", "GenericEvent", "LASTEvent"
    };
    return XEventTypeStrings[e.type];
}

//Get the miliseconds since the unix epoch
unsigned long long int getTime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}